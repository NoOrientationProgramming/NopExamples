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
#if APP_HAS_AVX2
#include <immintrin.h>
#endif

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
	MbValFull t;
	Color c;
};

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
	{0.65,  {227, 145,   0}},
	{0.70,  {200, 120,   0}}, // bronze
	{0.75,  {160,  90,   0}},
	{0.80,  {120,  60,   0}}, // dark bronze
	{0.85,  { 90,  25,   0}},
	{0.90,  { 60,  30,   0}}, // dark brown
	{0.95,  { 30,  15,   0}},
	{1.00,  {  0,   0,   0}}, // back to black
};

const size_t cScaleGradient = 20;

const size_t cNumKeysGradient = sizeof(keysGradient) / sizeof(keysGradient[0]);
const size_t cNumGradients = (cNumKeysGradient - 1) * cScaleGradient + 1;

static GradientStop gradient[cNumGradients] = {};

MandelBlockFilling::MandelBlockFilling()
	: Processing("MandelBlockFilling")
	//, mStartMs(0)
	, mpCfg(NULL)
	, mpLine(NULL)
	, mIdxLine(0)
	, mNumBlock(0)
	, mIdxBlock(0)
	, mNumPixel(0)
	, mIdxPixel(0)
	, mNumIter(0)
	, mpDataStart(NULL)
	, mpData(NULL)
{
	mState = StStart;
}

/* member functions */

const size_t shiftElem = 2;
const size_t numPixelPerBlock = 1 << shiftElem;
const size_t maskElem = numPixelPerBlock - 1;

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

		mNumBlock = ((mNumPixel + maskElem) >> shiftElem);
		mIdxBlock = 0;

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
	size_t numRemaining, numBurst; // Unit: Blocks
	size_t numPixelProcessed;

	numRemaining = mNumBlock - mIdxBlock;
	numBurst = PMIN(numRemaining, mpCfg->numBurst);

	char *pDataEnd = mpDataStart + mpCfg->szData + mpCfg->szPadding;
#if 0
	if (!mIdxLine && !mIdxPixel)
	{
		procDbgLog("Pixels per line  %u", mNumPixel);
		procDbgLog("Blocks per line  %u", mNumBlock);
	}
#endif
	for (; numBurst; --numBurst)
	{
		numRemaining = mNumPixel - mIdxPixel;
		numPixelProcessed = PMIN(numRemaining, numPixelPerBlock);
#if 0
		if (!mIdxLine)
			procDbgLog("%3u: %2u, %4u", mIdxBlock, numPixelProcessed, mIdxPixel);
#endif
		colorMandelbrotChunks(mpData, mIdxLine, mIdxPixel, numPixelProcessed);

		mpData += cBytesPerPixel * numPixelProcessed;

		++mIdxBlock;
		mIdxPixel += numPixelProcessed;
	}

	if (mIdxPixel < mNumPixel)
		return Pending;

	for (; mpData < pDataEnd; ++mpData)
	{
#if 0
		if (!mIdxLine)
			procDbgLog("0x00 -> %p", mpData);
#endif
		*mpData = 0;
	}

	return Positive;
}

void MandelBlockFilling::colorMandelbrotChunks(char *pData, size_t idxLine, size_t idxPixel, size_t numPixel)
{
#if APP_HAS_AVX2
	if (numPixel == numPixelPerBlock && !mpCfg->disableSimd)
	{
		colorMandelbrotSimd(pData, idxLine, idxPixel);
		return;
	}
#endif
#if 0
	if (!mIdxLine)
		procDbgLog("SCAL %3u: %4u", mIdxBlock, mIdxPixel);
#endif
	for (size_t i = 0; i < numPixel; ++i)
	{
		colorMandelbrotScalar(pData, idxLine, idxPixel);

		pData += cBytesPerPixel;
		++idxPixel;
	}

	//exit(1);
}

// (x[4], y[4]) -> (r, g, b)[4]
#if APP_HAS_AVX2
void MandelBlockFilling::colorMandelbrotSimd(char *pData, size_t idxLine, size_t idxPixel)
{
	if (!mIdxLine)
		procDbgLog("SIMD %3u: %4u", mIdxBlock, mIdxPixel);

	MbValFull w2 = ((MbValFull)mpCfg->imgWidth) / 2;
	MbValFull h2 = ((MbValFull)mpCfg->imgHeight) / 2;
	MbValFull scaleX = (1.0 / mpCfg->zoom) / w2;
	MbValFull scaleY = scaleX * (mpCfg->imgHeight / mpCfg->imgWidth) / h2;

	__m128i tmp_i, idxXin, idxYin;
	__m256d tmp_d, idxX, idxY, cx, cy;

	// 1. From image pixel space -> Complex space

	idxXin = _mm_set_epi32(idxPixel + 3, idxPixel + 2, idxPixel + 1, idxPixel + 0);
	idxYin = _mm_set1_epi32(idxLine);

	idxX = _mm256_sub_pd(_mm256_cvtepi32_pd(idxXin), _mm256_set1_pd(w2));
	idxY = _mm256_sub_pd(_mm256_cvtepi32_pd(idxYin), _mm256_set1_pd(h2));

	cx = _mm256_mul_pd(_mm256_set1_pd(scaleX), idxX);
	cx = _mm256_add_pd(cx, _mm256_set1_pd(mpCfg->posX));

	cy = _mm256_mul_pd(_mm256_set1_pd(scaleY), idxX);
	cy = _mm256_add_pd(cy, _mm256_set1_pd(mpCfg->posY));
#if 1
	hexDump(&idxX, sizeof(idxX));
	hexDump(&idxY, sizeof(idxY));

	for (size_t u = 0; u < numPixelPerBlock; ++u)
	{
		procDbgLog("[%u] idxX   %10.3f", u, idxX[u]);
		procDbgLog("[%u] idxY   %10.3f", u, idxY[u]);
	}

	for (size_t u = 0; u < numPixelPerBlock; ++u)
	{
		procDbgLog("[%u] cx     %10.3f", u, cx[u]);
		procDbgLog("[%u] cy     %10.3f", u, cy[u]);
	}
#endif
	// 2. Do the mandelbrot calculation in complex space

	size_t numIterMax = mpCfg->numIterMax;
	__m256d zx, zy;

	(void)numIterMax;
	(void)cx;
	(void)cy;
	(void)zx;
	(void)zy;

	// 3. Color mapping from fractional iterator -> RGB color

	__m128i idxGrad1, idxGrad2;
	__m256d mu, t, tMin, tMax;

	mu = _mm256_set1_pd(20);

	t = _mm256_mul_pd(_mm256_set1_pd(0.02), mu);
	t = _mm256_sub_pd(t, _mm256_floor_pd(t));

	tMin = _mm256_set1_pd(0.0);
	tMax = _mm256_set1_pd(1.0);

	t = _mm256_min_pd(t, tMax);
	t = _mm256_max_pd(t, tMin);

	tmp_i = _mm_set1_epi32(cNumGradients - 1);
	tmp_d = _mm256_cvtepi32_pd(tmp_i);
	tmp_d = _mm256_mul_pd(t, tmp_d);
	idxGrad1 = _mm256_cvtpd_epi32(tmp_d);

	tmp_i = _mm_set1_epi32(cNumGradients - 2);
	idxGrad1 = _mm_min_epi32(idxGrad1, tmp_i);

	tmp_i = _mm_set1_epi32(1);
	idxGrad2 = _mm_add_epi32(idxGrad1, tmp_i);

	hexDump(&idxGrad1, sizeof(idxGrad1));
	hexDump(&idxGrad2, sizeof(idxGrad2));
#if 0
	pGrad1 = &gradient[idxGrad1];
	pGrad2 = pGrad1 + 1;

	c = lerp(t, pGrad1->c, pGrad2->c);
#endif
	for (size_t i = 0; i < numPixelPerBlock; ++i)
	{
		// Not RGB but BGR! => BMP specific
		*pData++ = 0x20;
		*pData++ = 0x20;
		*pData++ = 0x20;

		++idxPixel;
	}
}
#endif

// (x, y) -> (r, g, b)
void MandelBlockFilling::colorMandelbrotScalar(char *pData, size_t idxLine, size_t idxPixel)
{
	size_t numIterMax = mpCfg->numIterMax;
	MbValFull w2 = ((MbValFull)mpCfg->imgWidth) / 2;
	MbValFull h2 = ((MbValFull)mpCfg->imgHeight) / 2;
	MbValFull scaleX = 1.0 / mpCfg->zoom;
	MbValFull scaleY = scaleX * mpCfg->imgHeight / mpCfg->imgWidth;

	MbValFull idxX = idxPixel - w2;
	MbValFull idxY = idxLine - h2;
	MbValFull cx = scaleX * idxX / w2 + mpCfg->posX;
	MbValFull cy = scaleY * idxY / h2 + mpCfg->posY;
	MbValFull zx, zy, mu, t, tMin, tMax;
	Color c;

	GradientStop *pGrad1, *pGrad2;
	size_t numIter, idxGrad1;

	mandelbrot(cx, cy, numIterMax, zx, zy, numIter);

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

		idxGrad1 = (size_t)(t * (cNumGradients - 1));
		idxGrad1 = PMIN(idxGrad1, cNumGradients - 2);

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

void MandelBlockFilling::mandelbrot(
			MbValFull cx, MbValFull cy, size_t numIterMax,
			MbValFull &zx, MbValFull &zy, size_t &numIter)
{
	MbValFull xx, yy, xy;

	zx = 0.0;
	zy = 0.0;

	numIter = 0;

	while (numIter < numIterMax)
	{
		xx = zx * zx;
		yy = zy * zy;

		if (xx + yy > 4.0)
			break;

		xy = zx * zy;

		zx = xx - yy + cx;
		zy = 2 * xy + cy;

		++numIter;
		++mNumIter;
	}
}

MbValFull MandelBlockFilling::fractionalIter(
			MbValFull zx, MbValFull zy,
			size_t numIter)
{
	MbValFull mag = sqrt(zx * zx + zy * zy);
	return numIter + 1 - log2(log2(mag));
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
	MbValFull t, tMin, tMax;

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

			t = ((MbValFull)s) / cScaleGradient;
			t = PMAX(tMin, PMIN(tMax, t));

			pGrad->t = lerp(t, pKey1->t, pKey2->t);
			pGrad->c = lerp(t, pKey1->c, pKey2->c);
#if 0
			if (i >= 32)
				continue;

			dbgLog("%2u - %2u - %2u: %0.3f, %3u %3u %3u",
				k, s, i, pGrad->t, pGrad->c.r, pGrad->c.g, pGrad->c.b);
#endif
		}
	}

	gradient[cNumGradients - 1] = keysGradient[cNumKeysGradient - 1];
}

