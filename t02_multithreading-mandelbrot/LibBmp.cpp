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

	if (pBmp->pFile)
		return false;

	FILE *pFile;

	pFile = fopen(pFilename, "wb");
	if (!pFile)
		return false;

	// Object
	pBmp->pFile = pFile;
	pBmp->idxLine = 0;
	pBmp->dataOk = 0;

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
	dataOk = idxLine == height;

	return true;
}

void FileBmp::close()
{
	if (!pFile)
		return;

	if (!width || !height || !dataOk)
	{
		if (!dataOk) wrnLog("Data NOT OK. Line index: %u", idxLine);

		fclose(pFile);
		pFile = NULL;
		return;
	}

	size_t szData = width * cBytesPerPixel; // Data per line
	size_t szLine = ((szData + 3) & ~3);
	uint32_t szFile;

	szData = szLine * height; // Data all lines
	szFile = szData + cSzHeaderBmp + cSzHeaderDib;

	dbgLog("Updating headers");
	dbgLog("Size file   %u", szFile);
	dbgLog("Width       %u", width);
	dbgLog("Height      %u", height);
	dbgLog("Size data   %u", szData);

	fseek(pFile, 2, SEEK_SET);
	fwrite(&szFile, 4, 1, pFile);

	fseek(pFile, cSzHeaderBmp + 4, SEEK_SET);
	fwrite(&width, 4, 1, pFile);

	fseek(pFile, cSzHeaderBmp + 8, SEEK_SET);
	fwrite(&height, 4, 1, pFile);

	fseek(pFile, cSzHeaderBmp + 20, SEEK_SET);
	fwrite(&szData, 4, 1, pFile);

	fclose(pFile);
	pFile = NULL;
}

