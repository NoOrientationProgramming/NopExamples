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

#ifndef MANDEL_BLOCK_FILLING_H
#define MANDEL_BLOCK_FILLING_H

#include "Processing.h"

typedef float MbVal;

struct ConfigMandelbrot
{
	// Image
	size_t imgWidth;
	size_t imgHeight;
	size_t szData;
	size_t szLine;
	size_t szPadding;

	// Mandelbrot
	size_t numIterMax;
	MbVal posX;
	MbVal posY;
	MbVal zoom;

	// Filling
	size_t numBurst;
};

struct BlockMandelHeader
{
	uint32_t success;
	uint32_t numIter;
};

enum FlagsFilling
{
	FlagFillingDone = 1,
	FlagFillingPositive = 2,
};

class MandelBlockFilling : public Processing
{

public:

	static MandelBlockFilling *create()
	{
		return new dNoThrow MandelBlockFilling;
	}

	ConfigMandelbrot *mpCfg;

	char *mpLine;
	size_t mIdxLine;

	static void gradientBuild();

protected:

	virtual ~MandelBlockFilling() {}

private:

	MandelBlockFilling();
	MandelBlockFilling(const MandelBlockFilling &) = delete;
	MandelBlockFilling &operator=(const MandelBlockFilling &) = delete;

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success process();
	void processInfo(char *pBuf, char *pBufEnd);

	Success lineFill();
	void colorMandelbrot(char *pData, size_t idxLine, size_t idxPixel);
	size_t mandelbrot(
			MbVal cx, MbVal cy,
			MbVal &zx, MbVal &zy,
			size_t numIterMax);
	MbVal fractionalIter(
			MbVal zx, MbVal zy,
			size_t numIter);
	size_t idxGradient(MbVal t);

	/* member variables */
	//uint32_t mStartMs;
	size_t mNumPixel;
	size_t mIdxPixel;
	BlockMandelHeader *mpHdr;
	char *mpDataStart;
	char *mpData;

	/* static functions */
	static void colorLerp(MbVal t,
			int r1, int g1, int b1,
			int r2, int g2, int b2,
			int &ro, int &go, int &bo);

	/* static variables */

	/* constants */

};

#endif

