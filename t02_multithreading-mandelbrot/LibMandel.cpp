/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 25.03.2026

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

#include "LibMandel.h"
#include "LibDspc.h"

using namespace std;

class Color
{
public:
	Color(int r_ = 0, int g_ = 0, int b_ = 0)
		: r(r_)
		, g(g_)
		, b(b_)
	{}

	Color operator+(const Color &other) const
	{ return Color(r + other.r, g + other.g, b + other.b); }
	Color operator-(const Color &other) const
	{ return Color(r - other.r, g - other.g, b - other.b); }
	Color operator*(MbValFull t) const
	{
		return Color(static_cast<int>(r * t),
			static_cast<int>(g * t),
			static_cast<int>(b * t));
	}
	Color operator/(MbValFull t) const
	{
		return Color(static_cast<int>(r / t),
			static_cast<int>(g / t),
			static_cast<int>(b / t));
	}

	int r;
	int g;
	int b;
};

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

static MbValFull fractionalIter(
			MbValFull zx, MbValFull zy,
			size_t numIter)
{
	MbValFull mag = sqrt(zx * zx + zy * zy);
	return numIter + 1 - log2(log2(mag));
}

static void mandelbrot(
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
	}
}

// (x, y) -> (r, g, b)
size_t colorMandelbrotScalar(ConfigMandelbrot *pCfg, char *pData, size_t idxLine, size_t idxPixel)
{
	// 1. From image pixel space -> Complex space

	MbValFull idxX = idxPixel - pCfg->w2;
	MbValFull idxY = idxLine - pCfg->h2;
	MbValFull cx = pCfg->scaleX * idxX + pCfg->posX;
	MbValFull cy = pCfg->scaleY * idxY + pCfg->posY;

	// 2. Do the mandelbrot calculation in complex space

	size_t numIter, numIterMax = pCfg->numIterMax;
	MbValFull zx, zy;

	mandelbrot(cx, cy, numIterMax, zx, zy, numIter);

	// 3. Color mapping from fractional iterator -> RGB color

	if (numIter >= numIterMax)
	{
		*pData++ = 0;
		*pData++ = 0;
		*pData++ = 0;

		return numIter;
	}

	GradientStop *pGrad1, *pGrad2;
	MbValFull mu, t, tMin, tMax;
	size_t idxGrad1;
	Color c;

	mu = fractionalIter(zx, zy, numIter);
	t = mu * 0.02;
	t = t - floor(t);

	tMin = 0.0;
	tMax = 1.0;

	t = PMAX(tMin, PMIN(tMax, t));

	idxGrad1 = (size_t)(t * (cNumGradients - 1));
	idxGrad1 = PMIN(idxGrad1, cNumGradients - 2);

	pGrad1 = &gradient[idxGrad1];
	pGrad2 = pGrad1 + 1;

	c = lerp(t, pGrad1->c, pGrad2->c);
#if 0
	if (idxLine < 2 && idxPixel < 4)
	{
		dbgLog("-----------------------------------");
		dbgLog("Idx. X          %12.0f (%zu)", idxPixel);
		dbgLog("Idx. Y          %12.0f (%zu)", idxLine);

		dbgLog("Complex X       %12.8f", cx);
		dbgLog("Complex Y       %12.8f", cy);

		dbgLog("Iterations      %12u", numIter);
		dbgLog("Frac. iter.     %12.3f", mu);
		dbgLog("Normalized      %12.3f", t);
		dbgLog("Idx. grad. 1    %12u", idxGrad1);
		dbgLog("Idx. grad. 2    %12u", idxGrad1 + 1);

		dbgLog("R/G/B            %3d/%3d/%3d", c.r, c.g, c.b);
	}
#endif
	// Not RGB but BGR! => BMP specific
	*pData++ = c.b;
	*pData++ = c.g;
	*pData++ = c.r;

	return numIter;
}

// (x[4], y[4]) -> (r, g, b)[4]
#if APP_HAS_AVX2
#if 1
static void m128iPrint(__m128i &val, const char *pName = NULL)
{
	uint32_t valOut[cNumPixelPerBlock];

	_mm_storeu_si128((__m128i *)valOut, val);

	dbgLog("-----------------------------------");
	dbgLog("%s = [%u, %u, %u, %u]",
		pName ? pName : "m128i",
		valOut[0], valOut[1], valOut[2], valOut[3]);

	hexDump(&valOut, sizeof(valOut));
	dbgLog("");
}

static void m256dPrint(__m256d &val, const char *pName = NULL)
{
	__m256d valOut = val;

	//double valOut[cNumPixelPerBlock];
	//_mm256_store_pd(valOut, val);

	dbgLog("-----------------------------------");
	dbgLog("%s = [%.8f, %.8f, %.8f, %.8f]",
		pName ? pName : "m256d",
		valOut[0], valOut[1], valOut[2], valOut[3]);

	hexDump(&valOut, sizeof(valOut));
}
#endif
size_t colorMandelbrotSimd(ConfigMandelbrot *pCfg, char *pData, size_t idxLine, size_t idxPixel)
{
	// 1. From image pixel space -> Complex space

	__m128i tmp_i, idxXin, idxYin;
	__m256d tmp_d, idxX, idxY, cx, cy;

	idxXin = _mm_set_epi32(idxPixel + 3, idxPixel + 2, idxPixel + 1, idxPixel + 0);
	idxYin = _mm_set1_epi32(idxLine);

	idxX = _mm256_sub_pd(_mm256_cvtepi32_pd(idxXin), _mm256_set1_pd(pCfg->w2));
	idxY = _mm256_sub_pd(_mm256_cvtepi32_pd(idxYin), _mm256_set1_pd(pCfg->h2));

	cx = _mm256_mul_pd(_mm256_set1_pd(pCfg->scaleX), idxX);
	cx = _mm256_add_pd(cx, _mm256_set1_pd(pCfg->posX));

	cy = _mm256_mul_pd(_mm256_set1_pd(pCfg->scaleY), idxY);
	cy = _mm256_add_pd(cy, _mm256_set1_pd(pCfg->posY));
#if 1
	m128iPrint(idxXin, "idxXin");
	m128iPrint(idxYin, "idxYin");

	m256dPrint(cx, "cx");
	m256dPrint(cy, "cy");
#endif
	// 2. Do the mandelbrot calculation in complex space

	size_t numIterMax = pCfg->numIterMax;
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
	for (size_t i = 0; i < cNumPixelPerBlock; ++i)
	{
		// Not RGB but BGR! => BMP specific
		*pData++ = 0x20;
		*pData++ = 0x20;
		*pData++ = 0x20;

		++idxPixel;
	}

	return 0;
}
#endif

void gradientBuild()
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

