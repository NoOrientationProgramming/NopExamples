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
	double posX;
	double posY;
	double zoom;

	// Filling
	size_t numBurst;
};

struct BlockMandelHeader
{
	uint32_t success;
};

enum FlagsFilling
{
	FlagFillingDone = 1,
	FlagFillingPositive = 2,
};

// TODO: Move to cpp file
struct GradientStop
{
	double t;
	int r;
	int g;
	int b;
};

static GradientStop keysGradient[] =
{
	{0.00,    0,   0,   0}, // black
	{0.05,    0,   0,  80}, // deep blue
	{0.10,    0,   0, 150}, // blue
	{0.15,    0,  50, 200}, // blue-cyan
	{0.20,    0, 120, 220}, // cyan
	{0.25,   40, 180, 255}, // light cyan
	{0.30,  120, 220, 255}, // very light blue
	{0.35,  200, 240, 255}, // almost white
	{0.40,  255, 255, 255}, // white
	{0.45,  255, 240, 180}, // warm white
	{0.50,  255, 220, 120}, // light gold
	{0.55,  255, 200,  60}, // gold
	{0.60,  255, 170,   0}, // deep gold
	{0.70,  200, 120,   0}, // bronze
	{0.80,  120,  60,   0}, // dark bronze
	{0.90,   60,  30,   0}, // dark brown
	{1.00,    0,   0,   0}, // back to black
};

const size_t cNumGradients = 256;

const size_t cNumKeysGradient = sizeof(keysGradient) / sizeof(keysGradient[0]);
const size_t cScaleGradient = cNumGradients / (cNumKeysGradient - 1);
// TODO: END

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

	// TODO: Move to private section
	static GradientStop gradient[cNumGradients];
	// TODO: END

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
			double cx, double cy,
			double &zx, double &zy,
			size_t numIterMax);
	double fractionalIter(
			double zx, double zy,
			size_t numIter);
	size_t idxGradient(double t);

	/* member variables */
	//uint32_t mStartMs;
	size_t mNumPixel;
	size_t mIdxPixel;
	char *mpDataStart;
	char *mpData;

	/* static functions */
	static void colorLerp(double t,
			int r1, int g1, int b1,
			int r2, int g2, int b2,
			int &ro, int &go, int &bo);

	/* static variables */

	/* constants */

};

#endif

