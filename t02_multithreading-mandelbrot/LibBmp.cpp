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

using namespace std;

bool bmpCreate(const char *pFilename, FileBmp *pBmp)
{
	if (!pFilename || !pBmp)
		return false;

	FILE *pFile;

	pFile = fopen(pFilename, "wb");
	if (!pFile)
		return false;

	pBmp->pFile = pFile;

	uint8_t buffer[40];
	size_t len;

	// Header BMP
	len = 14;
	(void)memset(buffer, 0, len);
	fwrite(buffer, sizeof(buffer[0]), len, pFile);

	// Header DIB
	len = sizeof(buffer);
	(void)memset(buffer, 0, len);
	fwrite(buffer, sizeof(buffer[0]), len, pFile);

	return true;
}

bool bmpAppend(FileBmp *pBmp, const char *pData, size_t len)
{
	if (!pBmp || !pData || !len)
		return false;

	if (!pBmp->width || !pBmp->height)
		return false;

	return true;
}

void bmpClose(FileBmp *pBmp)
{
	if (!pBmp)
		return;

	if (!pBmp->pFile)
		return;

	if (!pBmp->width || !pBmp->height)
	{
		fclose(pBmp->pFile);
		return;
	}

	fclose(pBmp->pFile);
}

