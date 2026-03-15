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
#ifdef _WIN32
#include <windows.h>
#endif

#include "MandelbrotCreating.h"
#include "LibTime.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 0
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
	, mBmp()
	, mSzBuffer(0)
	, mIdxLineFiller(0)
	, mIdxLineDone(0)
	, mIdxProgress(0)
	, mNumIterations(0)
	, mpLineFiller(NULL)
	, mpLineDone(NULL)
{
	// Image
	cfg.imgWidth = 1920;
	cfg.imgHeight = 1200;

	// Mandelbrot
	cfg.numIterMax = 2000;
	cfg.posX = -0.743643887037151;
	cfg.posY = 0.131825904205330;
	cfg.zoom = 170000;

	// Filling
	cfg.numBurst = 51;

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

		mpLineFiller = mpLineDone = mpBuffer;

		MandelBlockFilling::gradientBuild();

		hideCursor();
		progressPrint();

		mState = StMain;

		break;
	case StMain:

		ok = fillersStart();
		if (!ok)
			return procErrLog(-1, "could not start filler");

		success = linesProcess();
		if (success == Pending)
			break;

		if (success != Positive)
			return procErrLog(-1, "could not process lines");

		userInfLog("\n");
		userInfLog("Iterations    %u", mNumIterations);
		userInfLog("Duration      %ums\n", diffMs);

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
	showCursor();
	return Positive;
}

bool MandelbrotCreating::fillersStart()
{
#if 1
	if (mIdxLineDone != mIdxLineFiller)
		return true;
#endif
	size_t numRemaining, numBurst = 53;

	numRemaining = cfg.imgHeight - mIdxLineFiller;
	if (!numRemaining)
		return true;

	numBurst = PMIN(numRemaining, numBurst);

	MandelBlockFilling *pFill;

	for (; numBurst; --numBurst)
	{
		pFill = MandelBlockFilling::create();
		if (!pFill)
		{
			procErrLog(-1, "could not create process");
			return false;
		}

		pFill->mpCfg = &cfg;

		memset(mpLineFiller, 0, sizeof(BlockMandelHeader));
		pFill->mpLine = mpLineFiller;
		pFill->mIdxLine = mIdxLineFiller;
#if 1
		if (mIdxLineFiller)
			pFill->procTreeDisplaySet(false);
#endif
#if 0
		start(pFill);
#else
#if 0
		start(pFill, DrivenByNewInternalDriver);
#else
		start(pFill, DrivenByExternalDriver);
#if 1
		ThreadPooling::procAdd(pFill);
#else
		ThreadPooling::procAdd(pFill, 2);
#endif
#endif
#endif
		whenFinishedRepel(pFill);

		// Next line
		mpLineFiller += cfg.szLine;
		++mIdxLineFiller;
	}

	return true;
}

Success MandelbrotCreating::linesProcess()
{
	BlockMandelHeader *pHdr;
	char *pData;
	bool ok;

	while (1)
	{
		pHdr = (BlockMandelHeader *)mpLineDone;

		ok = pHdr->success & FlagFillingDone;
		if (!ok)
			break;

		ok = pHdr->success & FlagFillingPositive;
		if (!ok)
			return procErrLog(-1, "error filling line %u @ %p", mIdxLineDone, mpLineDone);

		mNumIterations += pHdr->numIter;

		pData = mpLineDone + sizeof(BlockMandelHeader);

		ok = mBmp.lineAppend(pData, cfg.szData);
		if (!ok)
			return procErrLog(-1, "could not append line");

		progressPrint();

		mpLineDone += cfg.szLine;
		++mIdxLineDone;

		if (mIdxLineDone < mBmp.height)
			continue;

		progressPrint();

		return Positive;
	}

	return Pending;
}

void MandelbrotCreating::progressPrint()
{
	++mIdxProgress;
	if (mIdxLineDone != 0 && mIdxLineDone != mBmp.height && mIdxProgress < 100)
		return;
	mIdxProgress = 0;

	char buf[59];
	char *pBufStart = buf;
	char *pBuf = pBufStart;
	char *pBufEnd = pBuf + sizeof(buf);

	pBuf[0] = 0;

	dInfo("\r");
	pBuf += progressStr(pBuf, pBufEnd, mIdxLineDone, mBmp.height);

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
	//mpPool->procTreeDisplaySet(false);

	start(mpPool);

	return true;
}

void MandelbrotCreating::hideCursor()
{
#ifdef _WIN32
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;

	info.dwSize = 100;
	info.bVisible = FALSE;

	SetConsoleCursorInfo(consoleHandle, &info);
#else
	fprintf(stdout, "\033[?25l");
	fflush(stdout);
#endif
}

void MandelbrotCreating::showCursor()
{
#ifdef _WIN32
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;

	info.dwSize = 100;
	info.bVisible = TRUE;

	SetConsoleCursorInfo(consoleHandle, &info);
#else
	fprintf(stdout, "\033[?25h");
	fflush(stdout);
#endif
}

void MandelbrotCreating::processInfo(char *pBuf, char *pBufEnd)
{
#if 0
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
	pBuf += progressStr(pBuf, pBufEnd, mIdxLineFiller, mBmp.height);
	dInfo("\n");

	pBuf += progressStr(pBuf, pBufEnd, mIdxLineDone, mBmp.height);
	dInfo("\n");
}

/* static functions */

