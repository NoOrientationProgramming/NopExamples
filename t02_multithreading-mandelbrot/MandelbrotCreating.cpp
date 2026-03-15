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

MandelbrotCreating::~MandelbrotCreating()
{
	if (!mpBuffer)
		return;

	delete[] mpBuffer;
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

		cfg.szData = mBmp.width * cBytesPerPixel;
		cfg.szLine = ((cfg.szData + 3) & ~3);
		cfg.szPadding = cfg.szLine - cfg.szData;

		cfg.szLine += sizeof(BlockMandelHeader);

		procDbgLog("Line header      %u", sizeof(BlockMandelHeader));
		procDbgLog("Data size        %u", cfg.szData);
		procDbgLog("Line padding     %u", cfg.szPadding);

		procDbgLog("Line size        %u", cfg.szLine);

		mSzBuffer = cfg.szLine * mBmp.height;
		procDbgLog("Buffer size      %u", mSzBuffer);

		mpBuffer = new dNoThrow char[mSzBuffer];
		if (!mpBuffer)
			return procErrLog(-1, "could not allocate data buffer");

		procDbgLog("Buffer start     %p", mpBuffer);
		procDbgLog("Buffer end       %p", mpBuffer + mSzBuffer);

		nameFile += ".bmp";
		ok = FileBmp::create(nameFile.c_str(), &mBmp);
		if (!ok)
			return procErrLog(-1, "could not create BMP file");

		mpLine = mpBuffer;
		mIdxLine = 0;

		MandelBlockFilling::gradientBuild();

		//progressPrint();

		lineFillersStart();

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

bool MandelbrotCreating::lineFillersStart()
{
	MandelBlockFilling *pFill;
	char *pLine = mpBuffer;
	size_t i = 0;

	for (; i < cfg.imgHeight; ++i)
	{
		pFill = MandelBlockFilling::create();
		if (!pFill)
		{
			procErrLog(-1, "could not create process");
			return false;
		}

		pFill->mpCfg = &cfg;

		memset(pLine, 0, sizeof(BlockMandelHeader));
		pFill->mpLine = pLine;
		pFill->mIdxLine = i;

		pLine += cfg.szLine;

		start(pFill);
		whenFinishedRepel(pFill);
	}

	return true;
}

Success MandelbrotCreating::shutdown()
{
	mBmp.close();
	return Positive;
}

Success MandelbrotCreating::linesProcess()
{
	bool ok;

	ok = mpLine[0] & FlagFillingDone;
	if (!ok)
		return Pending;

	ok = mpLine[0] & FlagFillingPositive;
	if (!ok)
		return procErrLog(-1, "error filling line %u @ %p", mIdxLine, mpLine);

	char *pData;

	pData = mpLine + sizeof(BlockMandelHeader);

	ok = mBmp.lineAppend(pData, cfg.szData);
	if (!ok)
		return procErrLog(-1, "could not append line");

	progressPrint();

	mpLine += cfg.szLine;
	++mIdxLine;

	if (mIdxLine < mBmp.height)
		return Pending;

	progressPrint();

	return Positive;
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

