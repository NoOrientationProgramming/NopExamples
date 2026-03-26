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

#include "MandelBlockFilling.h"
#include "LibBmp.h"
#include "LibDspc.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \
		gen(StIdle) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 0
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

using namespace std;

MandelBlockFilling::MandelBlockFilling()
	: Processing("MandelBlockFilling")
	//, mStartMs(0)
	, mpCfg(NULL)
	, mIdxLine(0)
	, mpLine(NULL)
	, mNumIter(0)
	, mNumBlock(0)
	, mIdxBlock(0)
	, mNumPixel(0)
	, mIdxPixel(0)
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

		mpData = mpLine;

		mNumPixel = mpCfg->szData / cBytesPerPixel;
		mIdxPixel = 0;

		mNumBlock = ((mNumPixel + cMaskElem) >> cShiftElem);
		mIdxBlock = 0;

		mState = StMain;

		break;
	case StMain:

		success = lineFill();
		if (success == Pending)
			break;
#if 0
		if (mIdxLine < 5)
			procDbgLog("Line %u @ %p finished", mIdxLine, mpLine);
#endif
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

	char *pDataEnd = mpLine + mpCfg->szLine;
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
		numPixelProcessed = PMIN(numRemaining, cNumPixelPerBlock);
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
	if (numPixel == cNumPixelPerBlock && !mpCfg->disableSimd)
	{
#if 1
		if (!idxLine)
			procDbgLog("SIMD %3u: %4u", mIdxBlock, mIdxPixel);
#endif
		mNumIter += colorMandelbrotSimd(mpCfg, pData, idxLine, idxPixel);
		//return;
	}
#endif
#if 0
	if (!mIdxLine)
		procDbgLog("SCAL %3u: %4u", mIdxBlock, mIdxPixel);
#endif
	for (size_t i = 0; i < numPixel; ++i)
	{
		mNumIter += colorMandelbrotScalar(mpCfg, pData, idxLine, idxPixel);

		pData += cBytesPerPixel;
		++idxPixel;
	}

	if (!mpCfg->disableSimd)
		exit(1);
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

