/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 14.03.2026

  Copyright (C) 2026, Johannes Natter

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "LibBmp.h"
#include "Processing.h"

using namespace std;

const size_t cSzHeaderBmp = 14;
const size_t cSzHeaderDib = 40;

bool FileBmp::create(const char *pFilename, FileBmp *pBmp)
{
	if (!pFilename || !pBmp)
		return false;

	Guard lock(pBmp->mMtxFile);

	if (pBmp->pFile)
		return false;

	FILE *pFile;

	pFile = fopen(pFilename, "wb");
	if (!pFile)
		return false;

	// Object
	pBmp->pFile = pFile;
	pBmp->idxLine = 0;

	// Headers
	uint8_t buf[cSzHeaderDib];
	size_t len;

	// Header BMP
	len = cSzHeaderBmp;

	(void)memset(buf, 0, len);

	buf[0] = 'B'; // Signature
	buf[1] = 'M';
	buf[10] = 54; // Pixel data offset

	fwrite(buf, sizeof(buf[0]), len, pFile);

	// Header DIB
	len = sizeof(buf);

	(void)memset(buf, 0, len);

	buf[0] = (uint8_t)len; // Header size

	buf[12] = 1; // Planes
	buf[14] = 24; // Bits per pixel

	fwrite(buf, sizeof(buf[0]), len, pFile);

	return true;
}

bool FileBmp::lineAppend(const char *pData, size_t len)
{
	Guard lock(mMtxFile);
	return lineAppendUnlocked(pData, len);
}

void FileBmp::close()
{
	Guard lock(mMtxFile);

	if (!pFile)
		return;

	if (!width || !height)
	{
		fclose(pFile);
		pFile = NULL;
		return;
	}

	size_t szData = width * cBytesPerPixel; // Size of data per line
	size_t maskLine = 3;
	size_t szLine = ((szData + maskLine) & ~maskLine);
	uint32_t szFile;

	/*
	 * Try to be as tolerant as possible to any situation.
	 * Do not overreact on errors!
	 * In this case we fill up the remaining
	 * lines if they weren't written by the user.
	 */
	imageComplete(szLine);

	szData = szLine * idxLine; // Size of data for all (written) lines
	szFile = szData + cSzHeaderBmp + cSzHeaderDib;

	dbgLog("Updating headers");
	dbgLog("Size file        %u", szFile);
	dbgLog("Width            %u", width);
	dbgLog("Height written   %u", idxLine);
	dbgLog("Size data        %u", szData);

	fseek(pFile, 2, SEEK_SET);
	fwrite(&szFile, sizeof(szFile), 1, pFile);

	fseek(pFile, cSzHeaderBmp + 4, SEEK_SET);
	fwrite(&width, sizeof(width), 1, pFile);

	fseek(pFile, cSzHeaderBmp + 8, SEEK_SET);
	fwrite(&idxLine, sizeof(idxLine), 1, pFile);

	fseek(pFile, cSzHeaderBmp + 20, SEEK_SET);
	fwrite(&szData, sizeof(szData), 1, pFile);

	fclose(pFile);
	pFile = NULL;
}

void FileBmp::imageComplete(size_t szLine)
{
	if (idxLine >= height)
		return;

	wrnLog("Image not finished. Filling up");
	wrnLog("Line index       %u", idxLine);
	wrnLog("Size             %u", szLine);

	size_t numLinesRemaining;
	char *pData;
	bool ok;

	pData = new dNoThrow char[szLine];
	if (!pData)
	{
		wrnLog("could not allocate data buffer");
		return;
	}

	memset(pData, 12, szLine);

	/*
	 * Important:
	 * We are in an unusual situation.
	 * We could check idxLine and height,
	 * but we can control the remaining lines
	 * variable by ourselves.
	 */
	numLinesRemaining = height - idxLine;

	for (; numLinesRemaining; --numLinesRemaining)
	{
		ok = lineAppendUnlocked(pData, szLine);
		if (ok)
			continue;

		wrnLog("could not append line");
		break;
	}

	delete[] pData;
}

bool FileBmp::lineAppendUnlocked(const char *pData, size_t len)
{
	if (idxLine >= height)
		return false;

	if (!pFile || !pData || !len || (len & 3))
		return false;

	if (!width || !height)
		return false;
#if 0
	if (idxLine < 5)
		wrnLog("Writing line: %u (%u) @ %p -> %p", idxLine, len, pData, pFile);
#endif
	fwrite(pData, sizeof(*pData), len, pFile);
	++idxLine;

	return true;
}

