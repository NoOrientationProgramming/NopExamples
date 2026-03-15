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

#include <math.h>

#include "MandelBlockFilling.h"
#include "LibBmp.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \
		gen(StDone) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 1
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

using namespace std;

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

static GradientStop gradient[cNumGradients] = {};

MandelBlockFilling::MandelBlockFilling()
	: Processing("MandelBlockFilling")
	//, mStartMs(0)
	, mpCfg(NULL)
	, mpLine(NULL)
	, mIdxLine(0)
	, mNumPixel(0)
	, mIdxPixel(0)
	, mpDataStart(NULL)
	, mpData(NULL)
{
	mState = StStart;
}

/* member functions */

Success MandelBlockFilling::process()
{
	//uint32_t curTimeMs = millis();
	//uint32_t diffMs = curTimeMs - mStartMs;
	Success success;
#if 0
	dStateTrace;
#endif
	switch (mState)
	{
	case StStart:

		if (!mpCfg || !mpLine)
			return procErrLog(-1, "invalid arguments");

		mpData = mpDataStart = mpLine + sizeof(BlockMandelHeader);

		mNumPixel = mpCfg->szData / cBytesPerPixel;
		mIdxPixel = 0;

		mState = StMain;

		break;
	case StMain:

		success = lineFill();
		if (success == Pending)
			break;

		if (success == Positive)
			mpLine[0] |= FlagFillingPositive;

		mState = StDone;

		break;
	case StDone:

		mpLine[0] |= FlagFillingDone;
#if 0
		if (mIdxLine < 5)
			procDbgLog("Line %u @ %p finished", mIdxLine, mpLine);
#endif
		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

Success MandelBlockFilling::lineFill()
{
	size_t numRemaining, numBurst;

	numRemaining = mNumPixel - mIdxPixel;
	numBurst = PMIN(numRemaining, mpCfg->numBurst);

	char *pDataEnd = mpDataStart + mpCfg->szData;
#if 0
	if (!mIdxLine && !mIdxPixel)
		procDbgLog("Pixels per line  %u", mNumPixel);
#endif
	for (; numBurst; --numBurst)
	{
		colorMandelbrot(mpData, mIdxLine, mIdxPixel);

		mpData += cBytesPerPixel;
		++mIdxPixel;
	}

	if (mIdxPixel < mNumPixel)
		return Pending;

	for (; mpData < pDataEnd; ++mpData)
		*mpData = 0;

	return Positive;
}

void MandelBlockFilling::colorMandelbrot(char *pData, size_t idxLine, size_t idxPixel)
{
	size_t numIterMax = 2000;
	double offsX = -0.743643887037151;
	double offsY = 0.131825904205330;
	double zoom = 170000;

	double w2 = mpCfg->imgWidth >> 1;
	double h2 = mpCfg->imgHeight >> 1;
	double idxX = idxPixel - w2;
	double idxY = idxLine - h2;
	double scaleX = 1.0 / zoom;
	double scaleY = scaleX * mpCfg->imgHeight / mpCfg->imgWidth;
	double cx = scaleX * idxX / w2 + offsX;
	double cy = scaleY * idxY / h2 + offsY;
	double zx, zy, mu, t;
	int r = 0, g = 0, b = 0;

	size_t numIter = mandelbrot(cx, cy, zx, zy, numIterMax);
	GradientStop *pGrad1, *pGrad2;
	size_t idxGrad1;

	if (numIter < numIterMax)
	{
		mu = fractionalIter(zx, zy, numIter);
#if 0
		t = mu / numIterMax;
		//t = pow(t, 0.7);
		t = sqrt(t);
		//t = 1.0 - t;
#else
		t = mu * 0.02;
		t = t - floor(t);
#endif
		t = PMAX(0.0, PMIN(1.0, t));

		idxGrad1 = idxGradient(t);
		pGrad1 = &gradient[idxGrad1];
		pGrad2 = pGrad1 + 1;

		colorLerp(t,
			pGrad1->r, pGrad1->g, pGrad1->b,
			pGrad2->r, pGrad2->g, pGrad2->b,
			r, g, b);

		//palette(mu, r, g, b);
	}
#if 0
	if (idxLine < 2 && !idxPixel)
	{
		procDbgLog("Idx. X          %.0f", idxX);
		procDbgLog("Idx. Y          %.0f", idxY);

		procDbgLog("Complex X       %.3f", cx);
		procDbgLog("Complex Y       %.3f", cy);

		procDbgLog("Iterations      %u", numIter);
		procDbgLog("Frac. iter.     %.3f", mu);
		procDbgLog("Normalized      %.3f", t);
		procDbgLog("Idx. grad. 1    %u", idxGrad1);
		procDbgLog("Idx. grad. 2    %u", idxGrad1 + 1);

		procDbgLog("R/G/B           %d/%d/%d", r, g, b);
	}
#endif
	// Not RGB but BGR! => BMP specific
	*pData++ = b;
	*pData++ = g;
	*pData++ = r;
}

size_t MandelBlockFilling::mandelbrot(
			double cx, double cy,
			double &zx, double &zy,
			size_t numIterMax)
{
	size_t i = 0;
	double xt;

	zx = 0.0;
	zy = 0.0;

	for (; zx*zx + zy*zy <= 4.0 && i < numIterMax; ++i)
	{
		xt = zx * zx - zy * zy + cx;
		zy = 2 * zx * zy + cy;
		zx = xt;
	}

	return i;
}

double MandelBlockFilling::fractionalIter(
			double zx, double zy,
			size_t numIter)
{
	double mag = sqrt(zx * zx + zy * zy);
	return numIter + 1 - log2(log2(mag));
}

size_t MandelBlockFilling::idxGradient(double t)
{
	GradientStop *pGrad1, *pGrad2;
	size_t i = 0;

	for (; i < cNumGradients - 1; ++i)
	{
		pGrad1 = &gradient[i];
		pGrad2 = pGrad1 + 1;

		if (t > pGrad1->t && t < pGrad2->t)
			return i;
	}

	return 0;
}

void MandelBlockFilling::processInfo(char *pBuf, char *pBufEnd)
{
#if 1
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
	dInfo("%03u: ", mIdxLine);
	pBuf += progressStr(pBuf, pBufEnd, mIdxPixel, mNumPixel);
	dInfo("\n");
}

/* static functions */

void MandelBlockFilling::gradientBuild()
{
	GradientStop *pKey1, *pKey2, *pGrad;
	size_t i, s, k = 0;
	double t;

	for (; k < cNumKeysGradient - 1; ++k)
	{
		for (s = 0; s < cScaleGradient; ++s)
		{
			pKey1 = &keysGradient[k];
			pKey2 = pKey1 + 1;

			i = k * cScaleGradient + s;
			pGrad = &gradient[i];

			t = ((double)s) / cScaleGradient;
			t = PMAX(0.0, PMIN(1.0, t));

			pGrad->t = pKey1->t + t * (pKey2->t - pKey1->t);

			colorLerp(t,
				pKey1->r, pKey1->g, pKey1->b,
				pKey2->r, pKey2->g, pKey2->b,
				pGrad->r, pGrad->g, pGrad->b);
#if 0
			if (i >= 32)
				continue;

			procDbgLog("%2u - %2u - %2u: %0.3f, %3u %3u %3u",
				k, s, i, pGrad->t, pGrad->r, pGrad->g, pGrad->b);
#endif
		}
	}
}

void MandelBlockFilling::colorLerp(double t,
			int r1, int g1, int b1,
			int r2, int g2, int b2,
			int &ro, int &go, int &bo)
{
	ro = r1 + (r2 - r1) * t;
	go = g1 + (g2 - g1) * t;
	bo = b1 + (b2 - b1) * t;
}

