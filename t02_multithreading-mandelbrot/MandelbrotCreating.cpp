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

		mBmp.width = 1920;
		mBmp.height = 1200;

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

		ok = FileBmp::create("mandelbrot.bmp", &mBmp);
		if (!ok)
			return procErrLog(-1, "could not create BMP file");

		//userInfLog("Processing");

		mpLine = mpBuffer;
		mIdxLine = 0;

		//progressPrint();

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

void MandelbrotCreating::colorMandelbrot(char *pData, size_t idxLine, size_t idxPixel)
{
	double w2 = mBmp.width >> 1;
	double h2 = mBmp.height >> 1;
	double idxX = idxPixel - w2;
	double idxY = idxLine - h2;
	double scaleX = 2.0;
	double scaleY = scaleX * mBmp.height / mBmp.width;
	double offsX = -0.5;
	double offsY = 0.0;
	double cx = scaleX * idxX / w2 + offsX;
	double cy = scaleY * idxY / h2 + offsY;
	size_t numIterMax = 40;
	int r = 0, g = 0, b = 0;

	size_t numIter = mandelbrot(cx, cy, numIterMax);
	double fracIter = fractionalIter(cx, cy, numIter);

	if (numIter < numIterMax)
	{
		r = 9 * numIter % 255;
		g = 7 * numIter % 255;
		b = 5 * numIter % 255;
	}

	//palette(fracIter, r, g, b);
#if 1
	if (idxLine < 5 && !idxPixel)
	{
		procDbgLog("Index X      %.0f", idxX);
		procDbgLog("Index Y      %.0f", idxY);

		procDbgLog("Complex X    %.3f", cx);
		procDbgLog("Complex Y    %.3f", cy);

		procDbgLog("Iterations   %u", numIter);
		procDbgLog("Frac. iter.  %.3f", fracIter);

		procDbgLog("R/G/B        %d/%d/%d", r, g, b);
	}
#endif
	*pData++ = r;
	*pData++ = g;
	*pData++ = b;
}

void MandelbrotCreating::palette(double fracIter, int &r, int &g, int &b)
{
	r = (int)(128 + 127 * sin(0.016 * fracIter + 4));
	g = (int)(128 + 127 * sin(0.013 * fracIter + 2));
	b = (int)(128 + 127 * sin(0.01  * fracIter + 1));
}

double MandelbrotCreating::fractionalIter(double cx, double cy, size_t numIter)
{
	double mag = sqrt(cx * cx + cy * cy);
	return numIter + 1 - log2(log2(mag));
}

size_t MandelbrotCreating::mandelbrot(double cx, double cy, size_t numIterMax)
{
	double zx = 0.0;
	double zy = 0.0;
	double xt;

	size_t i = 0;

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

