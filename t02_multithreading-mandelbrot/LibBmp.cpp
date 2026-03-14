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

	return true;
}

bool bmpAppend(const char *pData, size_t len)
{
	(void)pData;
	(void)len;

	return true;
}

void bmpClose(FileBmp *pBmp)
{
	if (!pBmp)
		return;

	if (!pBmp->pFile)
		return;

	fclose(pBmp->pFile);
}

