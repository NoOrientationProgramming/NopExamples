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

#ifdef _WIN32
#include <windows.h>
#endif

#include "Supervising.h"
#include "SystemDebugging.h"
#include "ThreadPooling.h"
#include "LibMandel.h"

#include "env.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 1
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

#define dForEach_SdState(gen) \
		gen(StSdStart) \
		gen(StSdAppDoneWait) \

#define dGenSdStateEnum(s) s,
dProcessStateEnum(SdState);

#if 0
#define dGenSdStateString(s) #s,
dProcessStateStr(SdState);
#endif

using namespace std;

Supervising::Supervising()
	: Processing("Supervising")
	//, mStartMs(0)
	, mStateSd(StSdStart)
	, mpMbCreate(NULL)
	, mIdxLineDone(10)
{
	mState = StStart;
}

/* member functions */

Success Supervising::process()
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

		if (env.port)
		{
			userInfLog("");
			userInfLog("  Starting in server mode: %u", env.port);
		}

		ok = servicesStart();
		if (!ok)
			return procErrLog(-1, "could not start services");

		hideCursor();
		progressPrint();

		mState = StMain;

		break;
	case StMain:

		progressPrint();

		success = mpMbCreate->success();
		if (success == Pending)
			break;

		if (success != Positive)
			return procErrLog(-1, "could not create Mandelbrot picture");

		progressPrint();
		resultPrint();

		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

Success Supervising::shutdown()
{
	switch (mStateSd)
	{
	case StSdStart:

		cancel(mpMbCreate);

		mStateSd = StSdAppDoneWait;

		break;
	case StSdAppDoneWait:

		if (mpMbCreate->progress())
			break;

		showCursor();

		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

bool Supervising::servicesStart()
{
	// Debugging
	SystemDebugging *pDbg;

	pDbg = SystemDebugging::create(this);
	if (!pDbg)
	{
		procWrnLog("could not create process");
		return false;
	}

	pDbg->listenLocalSet();

	pDbg->procTreeDisplaySet(false);
	start(pDbg);

	// Thread Pool
	ThreadPooling *pPool;

	pPool = ThreadPooling::create();
	if (!pPool)
	{
		procWrnLog("could not create process");
		return false;
	}

	pPool->cntWorkerSet(3);
	//pPool->procTreeDisplaySet(false);

	start(pPool);

	// Mandelbrot

	gradientBuild();

	mpMbCreate = MandelbrotCreating::create();
	if (!mpMbCreate)
	{
		procWrnLog("could not create process");
		return false;
	}

	mpMbCreate->nameFile = env.nameFile;

	ConfigMandelbrot *pMandel = &mpMbCreate->cfg;

	pMandel->imgWidth = 1920;
	pMandel->imgHeight = 1200;
	//pMandel->imgWidth = 2560;
	//pMandel->imgHeight = 1600;
	//pMandel->imgWidth = 3840;
	//pMandel->imgHeight = 2400;
	//pMandel->imgWidth = 7680;
	//pMandel->imgHeight = 4800;

	pMandel->forceDouble = env.forceDouble;
	pMandel->useDouble = pMandel->zoom > zoomFloatMax || pMandel->forceDouble;
#if APP_HAS_AVX2
	pMandel->disableSimd = env.disableSimd;
	pMandel->disableSimd = true;
#endif
	pMandel->numIterMax = 2000;
	pMandel->posX = -0.743643887037151;
	pMandel->posY = 0.131825904205330;
	pMandel->zoom = env.zoom;
#if 0
	pMandel->zoom = 17000; // float
	pMandel->zoom = 170000; // double
#endif
	configPrint(pMandel);

	start(mpMbCreate);

	return true;
}

void Supervising::configPrint(ConfigMandelbrot *pCfg)
{
	userInfLog("");
	userInfLog("  Image width       %14u [pixel]", pCfg->imgWidth);
	userInfLog("  Image height      %14u [pixel]", pCfg->imgHeight);
	userInfLog("");

	userInfLog("  Datatype          %14s%s",
					pCfg->useDouble ? "double" : "float",
					pCfg->forceDouble ? " (forced)" : "");
#if APP_HAS_AVX2
	userInfLog("  SIMD              %14s", pCfg->disableSimd ? "Disabled" : "Enabled");
#endif
	userInfLog("  Max. iter. per pixel        %u", pCfg->numIterMax);
	userInfLog("  Pos X             %14.3f", pCfg->posX);
	userInfLog("  Pos Y             %14.3f", pCfg->posY);
	userInfLog("  Zoom              %14.3e", pCfg->zoom);
	userInfLog("");
}

void Supervising::progressPrint()
{
	size_t idxLineDone = mpMbCreate->mIdxLineDone;

	if (idxLineDone == mIdxLineDone)
		return;
	mIdxLineDone = idxLineDone;

	char buf[59];
	char *pBufStart = buf;
	char *pBuf = pBufStart;
	char *pBufEnd = pBuf + sizeof(buf);

	pBuf[0] = 0;

	dInfo("\r  ");
	pBuf += progressStr(pBuf, pBufEnd,
			(int)idxLineDone,
			(int)mpMbCreate->cfg.imgHeight);

	fprintf(stdout, "%s\r", pBufStart);
	fflush(stdout);
}

void Supervising::resultPrint()
{
	size_t ips, numIter = mpMbCreate->mNumIterations;
	uint32_t durMs = mpMbCreate->mDurationMs;
	ConfigMandelbrot *pCfg = &mpMbCreate->cfg;

	userInfLog("\n");
	userInfLog("  Duration          %14zu [ms]", durMs);
	userInfLog("  Iterations        %14.3e", (double)numIter);
	ips = (size_t)(((double)numIter) / durMs);
	userInfLog("  Iter. per second  %14.3e", (double)ips);
	userInfLog("  Pixel * IPS       %14.3e", ((double)ips) * pCfg->imgWidth * pCfg->imgHeight);
	userInfLog("");
}

void Supervising::hideCursor()
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

void Supervising::showCursor()
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

void Supervising::processInfo(char *pBuf, char *pBufEnd)
{
#if 1
	dInfo("State\t\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

