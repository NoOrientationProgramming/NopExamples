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

#include "MandelbrotCreating.h"
#include "LibTime.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 1
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

using namespace std;

MandelbrotCreating::MandelbrotCreating()
	: Processing("MandelbrotCreating")
	, nameFile()
	, mStartMs(0)
	, mpPool(NULL)
	, mpBuffer(NULL)
	, mpLine(NULL)
	, mBmp()
	, mSzData(0)
	, mSzLine(0)
	, mSzPadding(0)
	, mSzBuffer(0)
	, mIdxLine(0)
	, mIdxProgress(0)
{
	cfg.imgWidth = 1920;
	cfg.imgHeight = 1200;

	cfg.numIterMax = 2000;
	cfg.posX = -0.743643887037151;
	cfg.posY = 0.131825904205330;
	cfg.zoom = 170000;

	mState = StStart;
}

/* member functions */

Success MandelbrotCreating::process()
{
	uint32_t curTimeMs = millis();
	uint32_t diffMs = curTimeMs - mStartMs;
	Success success;
	bool ok;
#if 0
	dStateTrace;
#endif
	switch (mState)
	{
	case StStart:

		mStartMs = curTimeMs;

		ok = servicesStart();
		if (!ok)
			return procErrLog(-1, "could not start services");

		mBmp.width = cfg.imgWidth;
		mBmp.height = cfg.imgHeight;

		mSzData = mBmp.width * cBytesPerPixel;
		mSzLine = ((mSzData + 3) & ~3);
		mSzPadding = mSzLine - mSzData;

		mSzLine += sizeof(uint32_t); // Add header

		procDbgLog("Line header   %u", sizeof(uint32_t));
		procDbgLog("Data size     %u", mSzData);
		procDbgLog("Line padding  %u", mSzPadding);

		procDbgLog("Line size     %u", mSzLine);

		mSzBuffer = mSzLine * mBmp.height;
		procDbgLog("Buffer size   %u", mSzBuffer);

		mpBuffer = new dNoThrow char[mSzBuffer];
		if (!mpBuffer)
			return procErrLog(-1, "could not allocate data buffer");

		procDbgLog("Buffer start  %p", mpBuffer);
		procDbgLog("Buffer end    %p", mpBuffer + mSzBuffer);

		nameFile += ".bmp";
		ok = FileBmp::create(nameFile.c_str(), &mBmp);
		if (!ok)
			return procErrLog(-1, "could not create BMP file");

		//userInfLog("Processing");

		mpLine = mpBuffer;
		mIdxLine = 0;

		//progressPrint();

		gradientBuild();

		mState = StMain;

		break;
	case StMain:

		success = linesProcess();
		if (success == Pending)
			break;

		if (success != Positive)
			return procErrLog(-1, "could not process lines");

		userInfLog("\n");
		userInfLog("Duration: %ums\n", diffMs);

		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

Success MandelbrotCreating::shutdown()
{
	mBmp.close();

	if (mpBuffer)
		delete[] mpBuffer;

	return Positive;
}

Success MandelbrotCreating::linesProcess()
{
	size_t numRemaining, numBurst = 53;
	size_t lenData = mSzLine - sizeof(uint32_t);
	char *pData;
	bool ok;

	numRemaining = mBmp.height - mIdxLine;
	numBurst = PMIN(numRemaining, numBurst);

	for (; numBurst; --numBurst)
	{
		pData = mpLine + sizeof(uint32_t);

		lineFill(mIdxLine, pData, lenData);
		ok = mBmp.lineAppend(pData, lenData);
		if (!ok)
			return procErrLog(-1, "could not append line");

		mpLine += mSzLine;

		progressPrint();
		++mIdxLine;
	}

	if (mIdxLine < mBmp.height)
		return Pending;

	progressPrint();

	return Positive;
}

void MandelbrotCreating::lineFill(size_t idxLine, char *pData, size_t len)
{
	char *pDataEnd = pData + len;
	size_t numPixels = len / cBytesPerPixel;
	size_t idxPixel = 0;

	if (!idxLine)
		procDbgLog("Pixels per line %u", numPixels);

	for (; idxPixel < numPixels; ++idxPixel)
	{
		colorMandelbrot(pData, idxLine, idxPixel);
		pData += cBytesPerPixel;
	}

	for (; pData < pDataEnd; ++pData)
		*pData = 0;
}

void MandelbrotCreating::colorTest(char *pData, size_t idxLine, size_t idxPixel)
{
	*pData++ = idxPixel % 256;
	*pData++ = idxLine % 256;
	*pData++ = 0;
}

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

void MandelbrotCreating::gradientBuild()
{
	GradientStop *pKey1, *pKey2, *pGrad;
	size_t i, s, k = 0;
	double t;

	for (; k < cNumKeysGradient - 1; ++k)
	{
		for (s = 0; s < cScaleGradient; ++s)
		{
			pKey1 = &keysGradient[k];
			pKey2 = &keysGradient[k + 1];

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

void MandelbrotCreating::colorMandelbrot(char *pData, size_t idxLine, size_t idxPixel)
{
	size_t numIterMax = 2000;
	double offsX = -0.743643887037151;
	double offsY = 0.131825904205330;
	double zoom = 170000;

	double w2 = mBmp.width >> 1;
	double h2 = mBmp.height >> 1;
	double idxX = idxPixel - w2;
	double idxY = idxLine - h2;
	double scaleX = 1.0 / zoom;
	double scaleY = scaleX * mBmp.height / mBmp.width;
	double cx = scaleX * idxX / w2 + offsX;
	double cy = scaleY * idxY / h2 + offsY;
	double zx, zy, mu, t;
	int r = 0, g = 0, b = 0;

	size_t numIter = mandelbrot(cx, cy, zx, zy, numIterMax);
	size_t idxGrad1, idxGrad2;

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
		idxGrad2 = idxGrad1 + 1;

		colorLerp(t,
			gradient[idxGrad1].r, gradient[idxGrad1].g, gradient[idxGrad1].b,
			gradient[idxGrad2].r, gradient[idxGrad2].g, gradient[idxGrad2].b,
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
		procDbgLog("Idx. grad. 2    %u", idxGrad2);

		procDbgLog("R/G/B           %d/%d/%d", r, g, b);
	}
#endif
	// Not RGB but BGR! => BMP specific
	*pData++ = b;
	*pData++ = g;
	*pData++ = r;
}

size_t MandelbrotCreating::idxGradient(double t)
{
	size_t i = 0;

	for (; i < cNumGradients - 1; ++i)
	{
		if (t > gradient[i].t && t < gradient[i + 1].t)
			return i;
	}

	return 0;
}

void MandelbrotCreating::colorLerp(double t,
			int r1, int g1, int b1,
			int r2, int g2, int b2,
			int &ro, int &go, int &bo)
{
	ro = r1 + (r2 - r1) * t;
	go = g1 + (g2 - g1) * t;
	bo = b1 + (b2 - b1) * t;
}

void MandelbrotCreating::palette(double t, int &r, int &g, int &b)
{
	double scale = 20;
	r = (int)(128 + 127 * sin(0.016 * scale * t + 4));
	g = (int)(128 + 127 * sin(0.013 * scale * t + 2));
	b = (int)(128 + 127 * sin(0.01  * scale * t + 1));
}

double MandelbrotCreating::fractionalIter(double zx, double zy, size_t numIter)
{
	double mag = sqrt(zx * zx + zy * zy);
	return numIter + 1 - log2(log2(mag));
}

size_t MandelbrotCreating::mandelbrot(double cx, double cy, double &zx, double &zy, size_t numIterMax)
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

void MandelbrotCreating::progressPrint()
{
	++mIdxProgress;
	if (mIdxLine != 0 && mIdxLine != mBmp.height && mIdxProgress < 100)
		return;
	mIdxProgress = 0;

	char buf[59];
	char *pBufStart = buf;
	char *pBuf = pBufStart;
	char *pBufEnd = pBuf + sizeof(buf);

	pBuf[0] = 0;

	dInfo("\r");
	pBuf += progressStr(pBuf, pBufEnd, mIdxLine, mBmp.height);

	fprintf(stdout, "%s\r", pBufStart);
	fflush(stdout);
}

bool MandelbrotCreating::servicesStart()
{
	mpPool = ThreadPooling::create();
	if (!mpPool)
	{
		procWrnLog("could not create process");
		return false;
	}

	mpPool->cntWorkerSet(3);

	start(mpPool);

	return true;
}

void MandelbrotCreating::processInfo(char *pBuf, char *pBufEnd)
{
#if 1
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
	pBuf += progressStr(pBuf, pBufEnd, mIdxLine, mBmp.height);
}

/* static functions */

