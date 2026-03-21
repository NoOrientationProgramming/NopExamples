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
#include "LibDspc.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \
		gen(StDone) \
		gen(StIdle) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 0
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

using namespace std;

struct GradientStop
{
	MbVal t;
	Color c;
};
#if 1
static GradientStop keysGradient[] =
{
	{0.00,  {  0,   0,   0}}, // black
	{0.05,  {  0,   0,  80}}, // deep blue
	{0.10,  {  0,   0, 150}}, // blue
	{0.15,  {  0,  50, 200}}, // blue-cyan
	{0.20,  {  0, 120, 220}}, // cyan
	{0.25,  { 40, 180, 255}}, // light cyan
	{0.30,  {120, 220, 255}}, // very light blue
	{0.35,  {200, 240, 255}}, // almost white
	{0.40,  {255, 255, 255}}, // white
	{0.45,  {255, 240, 180}}, // warm white
	{0.50,  {255, 220, 120}}, // light gold
	{0.55,  {255, 200,  60}}, // gold
	{0.60,  {255, 170,   0}}, // deep gold
	{0.70,  {200, 120,   0}}, // bronze
	{0.80,  {120,  60,   0}}, // dark bronze
	{0.90,  { 60,  30,   0}}, // dark brown
	{1.00,  {  0,   0,   0}}, // back to black
};
#else
static GradientStop keysGradient[] =
{
	{0.00,  {  0,   0,   0}}, // black
	{0.05,  { 80,  80,  80}}, // deep blue
	{0.10,  {150, 150, 150}}, // blue
	{0.15,  {200, 200, 200}}, // blue-cyan
	{0.20,  {220, 220, 220}}, // cyan
	{0.25,  {255, 255, 255}}, // light cyan
	{0.30,  {255, 255, 255}}, // very light blue
	{0.35,  {255, 255, 255}}, // almost white
	{0.40,  {255, 255, 255}}, // white
	{0.45,  {200, 200, 200}}, // mint
	{0.50,  {140, 140, 140}}, // light green
	{0.55,  { 80,  80,  80}}, // green
	{0.60,  { 60,  60,  60}}, // strong green
	{0.70,  { 40,  40,  40}}, // forest green
	{0.80,  { 25,  25,  25}}, // dark green
	{0.90,  { 15,  15,  15}}, // very dark green
	{1.00,  {  0,   0,   0}}, // back to black
};
#endif
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
	, mNumIter(0)
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

		mpHdr = (BlockMandelHeader *)mpLine;
		mpData = mpDataStart = mpLine + sizeof(BlockMandelHeader);

		memset(mpHdr, 0, sizeof(*mpHdr));

		mNumPixel = mpCfg->szData / cBytesPerPixel;
		mIdxPixel = 0;

		mState = StMain;

		break;
	case StMain:

		success = lineFill();
		if (success == Pending)
			break;

		if (success == Positive)
			mpHdr->success |= FlagFillingPositive;

		memcpy(mpHdr->numIter, &mNumIter, sizeof(mpHdr->numIter));

		mState = StDone;

		break;
	case StDone:

		mpHdr->success |= FlagFillingDone;
#if 0
		if (mIdxLine < 5)
			procDbgLog("Line %u @ %p finished", mIdxLine, mpLine);
#endif
		if (!mIdxLine)
		{
			mState = StIdle;
			break;
		}

		return Positive;

		break;
	case StIdle:

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
	size_t numIterMax = mpCfg->numIterMax;
	MbVal offsX = mpCfg->posX;
	MbVal offsY = mpCfg->posY;
	MbVal zoom = mpCfg->zoom;

	MbVal w2 = (MbVal)(mpCfg->imgWidth >> 1);
	MbVal h2 = (MbVal)(mpCfg->imgHeight >> 1);
	MbVal idxX = idxPixel - w2;
	MbVal idxY = idxLine - h2;
	MbVal scaleX = 1.0 / zoom;
	MbVal scaleY = scaleX * mpCfg->imgHeight / mpCfg->imgWidth;
	MbVal cx = scaleX * idxX / w2 + offsX;
	MbVal cy = scaleY * idxY / h2 + offsY;
	MbVal zx, zy, mu, t, tMin, tMax;
	Color c;

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
		tMin = 0.0;
		tMax = 1.0;

		t = PMAX(tMin, PMIN(tMax, t));

		idxGrad1 = idxGradient(t);
		pGrad1 = &gradient[idxGrad1];
		pGrad2 = pGrad1 + 1;

		c = lerp(t, pGrad1->c, pGrad2->c);
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

		procDbgLog("R/G/B           %d/%d/%d", c.r, c.g, c.b);
	}
#endif
	// Not RGB but BGR! => BMP specific
	*pData++ = c.b;
	*pData++ = c.g;
	*pData++ = c.r;
}

size_t MandelBlockFilling::mandelbrot(
			MbVal cx, MbVal cy,
			MbVal &zx, MbVal &zy,
			size_t numIterMax)
{
	size_t i = 0;
	MbVal xx, yy, xy;

	zx = 0.0;
	zy = 0.0;

	while (i < numIterMax)
	{
		xx = zx * zx;
		yy = zy * zy;

		if (xx + yy > 4.0)
			break;

		xy = zx * zy;

		zx = xx - yy + cx;
		zy = 2 * xy + cy;

		++mNumIter;
		++i;
	}

	return i;
}

MbVal MandelBlockFilling::fractionalIter(
			MbVal zx, MbVal zy,
			size_t numIter)
{
	MbVal mag = sqrt(zx * zx + zy * zy);
	return numIter + 1 - log2(log2(mag));
}

size_t MandelBlockFilling::idxGradient(MbVal t)
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
#if 0
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
	dInfo("%03u: ", mIdxLine);
	pBuf += progressStr(pBuf, pBufEnd, (int)mIdxPixel, (int)mNumPixel);
	dInfo("\n");
}

/* static functions */

void MandelBlockFilling::gradientBuild()
{
	GradientStop *pKey1, *pKey2, *pGrad;
	size_t i, s, k = 0;
	MbVal t, tMin, tMax;

	for (; k < cNumKeysGradient - 1; ++k)
	{
		for (s = 0; s < cScaleGradient; ++s)
		{
			pKey1 = &keysGradient[k];
			pKey2 = pKey1 + 1;

			i = k * cScaleGradient + s;
			pGrad = &gradient[i];

			tMin = 0.0;
			tMax = 1.0;

			t = ((MbVal)s) / cScaleGradient;
			t = PMAX(tMin, PMIN(tMax, t));

			pGrad->t = lerp(t, pKey1->t, pKey2->t);
			pGrad->c = lerp(t, pKey1->c, pKey2->c);
#if 0
			if (i >= 32)
				continue;

			procDbgLog("%2u - %2u - %2u: %0.3f, %3u %3u %3u",
				k, s, i, pGrad->t, pGrad->c.r, pGrad->c.g, pGrad->c.b);
#endif
		}
	}
}

