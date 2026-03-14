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

#include "MandelbrotCreating.h"

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

const size_t cBytesPerPixel = 3 * sizeof(char);

MandelbrotCreating::MandelbrotCreating()
	: Processing("MandelbrotCreating")
	//, mStartMs(0)
	, mpPool(NULL)
	, mpData(NULL)
	, mSzData(0)
	, mSzLine(0)
	, mSzPadding(0)
	, mSzBuffer(0)
	, mBmp()
	, mIdxLine(0)
	, mpLine(NULL)
{
	mState = StStart;
}

/* member functions */

Success MandelbrotCreating::process()
{
	//uint32_t curTimeMs = millis();
	//uint32_t diffMs = curTimeMs - mStartMs;
	Success success;
	bool ok;
#if 0
	dStateTrace;
#endif
	switch (mState)
	{
	case StStart:

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

		mSzBuffer = mSzLine* mBmp.height;
		procDbgLog("Buffer size   %u", mSzBuffer);

		mpData = new dNoThrow char[mSzData];
		if (!mpData)
			return procErrLog(-1, "could not allocate data buffer");

		ok = FileBmp::create("mandelbrot.bmp", &mBmp);
		if (!ok)
			return procErrLog(-1, "could not create BMP file");

		userInfLog("Processing");

		mpLine = mpData;
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

		userInfLog("\nProcessing: Done");

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

	if (mpData)
		delete[] mpData;

	return Positive;
}

Success MandelbrotCreating::linesProcess()
{
	size_t numRemaining, numBurst = 53;
	size_t lenData = mSzLine - sizeof(uint32_t);
	char *pData;

	numRemaining = mBmp.height - mIdxLine;
	numBurst = PMIN(numRemaining, numBurst);

	for (; numBurst; --numBurst)
	{
		pData = mpLine + sizeof(uint32_t);

		lineFill(mIdxLine, pData, lenData);
		mBmp.lineAppend(pData, lenData);

		mpLine += mSzLine;

		++mIdxLine;
		progressPrint();
	}

	if (mIdxLine < mBmp.height)
		return Pending;

	return Positive;
}

void MandelbrotCreating::lineFill(size_t idx, char *pData, size_t len)
{
	size_t numPixels = len / cBytesPerPixel;

	(void)idx;
	(void)pData;
	(void)len;

	if (!idx)
		procDbgLog("Pixels per line %u", numPixels);
}

void MandelbrotCreating::progressPrint()
{
	char buf[59];
	char *pBufStart = buf;
	char *pBuf = pBufStart;
	char *pBufEnd = pBuf + sizeof(buf);

	buf[0] = 0;

	dInfo("\r");
	pBuf += progressStr(pBuf, pBufEnd, mIdxLine, mBmp.height);

	fprintf(stdout, "%s", pBufStart);
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

