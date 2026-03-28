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

typedef int8_t ElemColor;

class Color
{
public:
	Color(uint8_t r_ = 0, uint8_t g_ = 0, uint8_t b_ = 0)
		: mR((ElemColor)r_)
		, mG((ElemColor)g_)
		, mB((ElemColor)b_)
	{}

	uint8_t r() { return mR; }
	uint8_t g() { return mG; }
	uint8_t b() { return mB; }

	Color operator+(const Color &other) const
	{ return Color(mR + other.mR, mG + other.mG, mB + other.mB); }
	Color operator-(const Color &other) const
	{ return Color(mR - other.mR, mG - other.mG, mB - other.mB); }
	Color operator*(MbValFull t) const
	{ return Color(mR * t, mG * t, mB * t); }

private:
	ElemColor mR;
	ElemColor mG;
	ElemColor mB;
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

template<typename T>
static T fractionalIter(
			T zx, T zy,
			size_t numIter)
{
	T mag = sqrt(zx * zx + zy * zy);
	return numIter + 1 - log2(log2(mag));
}

template<typename T>
static void mandelbrot(
			T cx, T cy, size_t numIterMax,
			T &zx, T &zy, size_t &numIter)
{
	T xx, yy, xy;

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
template<typename T>
static void colorMandelbrotScalar(ConfigMandelbrot *pCfg, char *pData, size_t idxLine, size_t idxPixel, size_t &numIter)
{
	// 1. From image pixel space -> Complex space

	T idxX = idxPixel - pCfg->w2;
	T idxY = idxLine - pCfg->h2;
	T cx = pCfg->scaleX * idxX + pCfg->posX;
	T cy = pCfg->scaleY * idxY + pCfg->posY;

	// 2. Do the mandelbrot calculation in complex space

	size_t numIterMax = pCfg->numIterMax;
	T zx, zy;

	mandelbrot(cx, cy, numIterMax, zx, zy, numIter);

	// 3. Color mapping from fractional iterator -> RGB color

	if (numIter >= numIterMax)
	{
		*pData++ = 0;
		*pData++ = 0;
		*pData++ = 0;

		return;
	}

	GradientStop *pGrad1, *pGrad2;
	T mu, t, tMin, tMax;
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
	dbgLog("-----------------------------------");
	dbgLog("idxX            %12.0f (%zu)", idxPixel);
	dbgLog("idxY            %12.0f (%zu)", idxLine);
	dbgLog("cx              %12.8f", cx);
	dbgLog("cy              %12.8f", cy);
	dbgLog("zx              %12.8f", zx);
	dbgLog("zy              %12.8f", zy);
	dbgLog("numIter         %12u", numIter);
	dbgLog("mu              %12.8f", mu);
	dbgLog("t               %12.8f", t);
	dbgLog("idxGrad1        %12u", idxGrad1);
	dbgLog("R/G/B            %3u/%3u/%3u", c.r(), c.g(), c.b());
#endif
	// Not RGB but BGR! => BMP specific
	*pData++ = c.b();
	*pData++ = c.g();
	*pData++ = c.r();
#if 0
	hexDump(pData - 3, 3, "COLOR SCALAR");
#endif
}

size_t colorMandelbrotScalar(ConfigMandelbrot *pCfg, char *pData, size_t idxLine, size_t idxPixel)
{
	size_t numIter;

	if (pCfg->useDouble)
		colorMandelbrotScalar<MbValFull>(pCfg, pData, idxLine, idxPixel, numIter);
	else
		colorMandelbrotScalar<MbVal>(pCfg, pData, idxLine, idxPixel, numIter);

	return numIter;
}

// (x[4], y[4]) -> (r, g, b)[4]
#if APP_HAS_AVX2
#if 0
static void m128iPrint(__m128i &val, const char *pName = NULL)
{
	int32_t valOut[cNumPixelPerBlock];

	_mm_storeu_si128((__m128i *)valOut, val);

	dbgLog("%s = [%d, %d, %d, %d]",
		pName ? pName : "m128i",
		valOut[0], valOut[1], valOut[2], valOut[3]);

	hexDump(&valOut, sizeof(valOut));
}
#if 1
static void m256dPrint(__m256d &val, const char *pName = NULL)
{
	MbValFull valOut[cNumPixelPerBlock];

	_mm256_storeu_pd(valOut, val);

	dbgLog("%s = [%.8f, %.8f, %.8f, %.8f]",
		pName ? pName : "m256d",
		valOut[0], valOut[1], valOut[2], valOut[3]);

	hexDump(&valOut, sizeof(valOut));
}
#endif
#endif
static __m256d fractionalIter(
			__m256d zx, __m256d zy,
			__m256d numIter)
{
	MbValFull mag_d[cNumPixelPerBlock];
	__m256d xx, yy, mag, cOne;

	cOne = _mm256_set1_pd(1.0);

	xx = _mm256_mul_pd(zx, zx);
	yy = _mm256_mul_pd(zy, zy);
	mag = _mm256_sqrt_pd(_mm256_add_pd(xx, yy));

	_mm256_storeu_pd(mag_d, mag);

	for (size_t i = 0; i < cNumPixelPerBlock; ++i)
		mag_d[i] = log2(log2(mag_d[i]));

	mag = _mm256_loadu_pd(mag_d);
	mag = _mm256_sub_pd(mag, cOne);

	return _mm256_sub_pd(numIter, mag);
}

static void mandelbrot(
			__m256d &cx, __m256d &cy, size_t numIterMax,
			__m256d &zx, __m256d &zy, __m256d &numIter)
{
	__m256d xx, yy, xy, mag, mask, newZx, newZy;
	__m256d cOne, cTwo, cFour;
	__m256d numIterNew;

	cOne = _mm256_set1_pd(1.0);
	cTwo = _mm256_set1_pd(2.0);
	cFour = _mm256_set1_pd(4.0);

	zx = _mm256_setzero_pd();
	zy = _mm256_setzero_pd();

	numIterNew = _mm256_setzero_pd();
	numIter = _mm256_setzero_pd();

	(void)cOne;
	(void)numIterNew;

	for (size_t i = 0; i < numIterMax; ++i)
	{
		// xx = zx * zx;
		xx = _mm256_mul_pd(zx, zx);
		// yy = zy * zy;
		yy = _mm256_mul_pd(zy, zy);

		// if (xx + yy > 4.0)
		mag = _mm256_add_pd(xx, yy);
		mask = _mm256_cmp_pd(mag, cFour, _CMP_LE_OS);

		if (_mm256_testz_pd(mask, mask))
			break;

		// xy = zx * zy;
		xy = _mm256_mul_pd(zx, zy);

		// zx = xx - yy + cx;
		newZx = _mm256_add_pd(_mm256_sub_pd(xx, yy), cx);
		zx = _mm256_blendv_pd(zx, newZx, mask);

		// zy = 2 * xy + cy;
		newZy = _mm256_add_pd(_mm256_mul_pd(cTwo, xy), cy); // _mm256_fmadd_pd
		zy = _mm256_blendv_pd(zy, newZy, mask);
#if 1 // 60ms
		// ++numIter
		numIterNew = _mm256_add_pd(cOne, numIter);
		numIter = _mm256_blendv_pd(numIter, numIterNew, mask);
#endif
	}
}

__m128i lerp(MbValFull t_d, __m256d a, __m256d b)
{
	__m256d t, tmp_d;
	__m128i tmp_i;

	t = _mm256_set1_pd(t_d);

	tmp_d = _mm256_sub_pd(b, a);
	tmp_d = _mm256_mul_pd(tmp_d, t);
	tmp_i = _mm256_cvttpd_epi32(tmp_d);

	tmp_i = _mm_add_epi32(_mm256_cvttpd_epi32(a), tmp_i);

	return tmp_i;
}

void colorMandelbrotSimdDouble(ConfigMandelbrot *pCfg, char *pData, size_t idxLine, size_t idxPixel, size_t &numIterSum)
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

	// 2. Do the mandelbrot calculation in complex space

	size_t numIterMax = pCfg->numIterMax;
	MbValFull numIter_d[cNumPixelPerBlock];
	__m256d zx, zy, numIter;

	mandelbrot(cx, cy, numIterMax, zx, zy, numIter);

	_mm256_storeu_pd(numIter_d, numIter);

	numIterSum = 0;
	for (size_t i = 0; i < cNumPixelPerBlock; ++i)
		numIterSum += (size_t)numIter_d[i];

	// 3. Color mapping from fractional iterator -> RGB color

	uint32_t idxGrad1_u[cNumPixelPerBlock];
	MbValFull t_d[cNumPixelPerBlock];
	GradientStop *pGrad1, *pGrad2;
	__m256d mu, t, tMin, tMax;
	__m128i idxGrad1, c;
	__m256d c1, c2;

	mu = fractionalIter(zx, zy, numIter);

	t = _mm256_mul_pd(_mm256_set1_pd(0.02), mu);
	t = _mm256_sub_pd(t, _mm256_floor_pd(t));

	tMin = _mm256_set1_pd(0.0);
	tMax = _mm256_set1_pd(1.0);

	t = _mm256_min_pd(t, tMax);
	t = _mm256_max_pd(t, tMin);

	tmp_i = _mm_set1_epi32(cNumGradients - 1);
	tmp_d = _mm256_cvtepi32_pd(tmp_i);
	tmp_d = _mm256_mul_pd(t, tmp_d);
	idxGrad1 = _mm256_cvttpd_epi32(tmp_d);

	tmp_i = _mm_set1_epi32(cNumGradients - 2);
	idxGrad1 = _mm_min_epi32(idxGrad1, tmp_i);

	_mm256_storeu_pd(t_d, t);
	_mm_storeu_si128((__m128i *)idxGrad1_u, idxGrad1);
#if 0
	m128iPrint(idxXin, "idxXin");
	m128iPrint(idxYin, "idxYin");
	m256dPrint(cx, "cx");
	m256dPrint(cy, "cy");
	m256dPrint(zx, "zx");
	m256dPrint(zy, "zy");
	m256dPrint(numIter, "numIter");
	dbgLog("numIterSum = %zu", numIterSum);
	m256dPrint(mu, "mu");
	m256dPrint(t, "t");
	m128iPrint(idxGrad1, "idxGrad1");
#endif
	for (size_t i = 0; i < cNumPixelPerBlock; ++i)
	{
		pGrad1 = &gradient[idxGrad1_u[i]];
		pGrad2 = pGrad1 + 1;

		c1 = _mm256_set_pd(0, pGrad1->c.b(), pGrad1->c.g(), pGrad1->c.r());
		c2 = _mm256_set_pd(0, pGrad2->c.b(), pGrad2->c.g(), pGrad2->c.r());

		c = lerp(t_d[i], c1, c2);

		if ((size_t)numIter_d[i] >= numIterMax)
			c = _mm_set1_epi32(0);
#if 0
		dbgLog("R/G/B            %3u/%3u/%3u",
				_mm_extract_epi8(c, 0),
				_mm_extract_epi8(c, 4),
				_mm_extract_epi8(c, 8));
#endif
		// Not RGB but BGR! => BMP specific
		*pData++ = _mm_extract_epi8(c, 8);
		*pData++ = _mm_extract_epi8(c, 4);
		*pData++ = _mm_extract_epi8(c, 0);
	}
#if 0
	hexDump(pData - 12, 12, "COLOR SIMD");
#endif
}

void colorMandelbrotSimdFloat(ConfigMandelbrot *pCfg, char *pData, size_t idxLine, size_t idxPixel, size_t &numIterSum)
{
	(void)pCfg;
	(void)pData;
	(void)idxLine;
	(void)idxPixel;

	numIterSum = 0;

	for (size_t i = 0; i < cNumPixelPerBlock; ++i)
	{
		*pData++ = 0x7c;
		*pData++ = 0x7c;
		*pData++ = 0x7c;
	}
}

size_t colorMandelbrotSimd(ConfigMandelbrot *pCfg, char *pData, size_t idxLine, size_t idxPixel)
{
	size_t numIter;

	if (pCfg->useDouble)
		colorMandelbrotSimdDouble(pCfg, pData, idxLine, idxPixel, numIter);
	else
		colorMandelbrotSimdFloat(pCfg, pData, idxLine, idxPixel, numIter);

	return numIter;
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
			if (i >= 32) continue;
			dbgLog("%2u - %2u - %2u: %0.3f, %3u %3u %3u",
				k, s, i, pGrad->t, pGrad->c.r, pGrad->c.g, pGrad->c.b);
#endif
		}
	}

	gradient[cNumGradients - 1] = keysGradient[cNumKeysGradient - 1];
}

