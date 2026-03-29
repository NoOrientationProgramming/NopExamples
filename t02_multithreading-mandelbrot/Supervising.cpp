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
#include "UserInteracting.h"
#include "LibMandel.h"

#include "env.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \
		gen(StServerStart) \
		gen(StServer) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 1
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

#define dForEach_SdState(gen) \
		gen(StSdStart) \
		gen(StSdMandelDoneWait) \

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
	, mpListen(NULL)
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

		sleepUsInternalDriveSet(30000);

		ok = basicsStart();
		if (!ok)
			return procErrLog(-1, "could not start basic services");

		if (env.port)
		{
			userInfLog("");
			userInfLog("  Listening on TCP port %u (telnet)", env.port);

			mState = StServerStart;
			break;
		}

		ok = mandelbrotStart();
		if (!ok)
			return procErrLog(-1, "could not start mandelbrot creation");

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
	case StServerStart:

		ok = serverStart();
		if (!ok)
			return procErrLog(-1, "could not start server");

		hideCursor();

		mState = StServer;

		break;
	case StServer:

		peerAdd();

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

		if (!mpMbCreate)
		{
			showCursor();
			return Positive;
		}

		cancel(mpMbCreate);

		mStateSd = StSdMandelDoneWait;

		break;
	case StSdMandelDoneWait:

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

bool Supervising::basicsStart()
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
	if (!env.numThreadsPool)
		return true;

	ThreadPooling *pPool;

	pPool = ThreadPooling::create();
	if (!pPool)
	{
		procWrnLog("could not create process");
		return false;
	}

	pPool->cntWorkerSet(env.numThreadsPool);
	//pPool->procTreeDisplaySet(false);

	start(pPool);

	return true;
}

bool Supervising::mandelbrotStart()
{
	libMandelInit();

	mpMbCreate = MandelbrotCreating::create();
	if (!mpMbCreate)
	{
		procWrnLog("could not create process");
		return false;
	}

	mpMbCreate->mNameFile = env.nameFile;

	mpMbCreate->mTypeDriver = env.typeDriver;
	mpMbCreate->mNumThreadsPool = env.numThreadsPool;
	mpMbCreate->mNumFillers = env.numFillers;

	ConfigMandelbrot *pMandel = &mpMbCreate->cfg;

	pMandel->imgWidth = env.imgWidth;
	pMandel->imgHeight = env.imgHeight;

	pMandel->forceDouble = env.forceDouble;
	pMandel->useDouble = env.zoom > cZoomFloatMax || env.forceDouble;
#if APP_HAS_AVX2
	pMandel->disableSimd = env.disableSimd;
#endif
	pMandel->posX = env.posX;
	pMandel->posY = env.posY;
	pMandel->zoom = env.zoom;
	pMandel->numIterMax = env.numIterMax;

	configPrint(pMandel);

	start(mpMbCreate);

	return true;
}

bool Supervising::serverStart()
{
	mpListen = TcpListening::create();
	if (!mpListen)
	{
		procWrnLog("could not create process");
		return false;
	}

	mpListen->portSet(env.port);

	start(mpListen);

	return true;
}

void Supervising::peerAdd()
{
	UserInteracting *pUser;
	SOCKET fdPeer;

	while (1)
	{
		fdPeer = mpListen->nextPeerFd();
		if (fdPeer == INVALID_SOCKET)
			break;

		pUser = UserInteracting::create();
		if (!pUser)
		{
			procDbgLog("could not create process");
			continue;
		}

		pUser->mFd = fdPeer;

		start(pUser);
		whenFinishedRepel(pUser);
	}
}

void Supervising::configPrint(ConfigMandelbrot *pCfg)
{
	userInfLog("");
	userInfLog("  Image width             %14u [pixel]", pCfg->imgWidth);
	userInfLog("  Image height            %14u [pixel]", pCfg->imgHeight);
	userInfLog("");

	userInfLog("  Datatype                %14s%s",
					pCfg->useDouble ? "double" : "float",
					pCfg->forceDouble ? " (forced)" : "");
#if APP_HAS_AVX2
	userInfLog("  SIMD                    %14s", pCfg->disableSimd ? "Disabled" : "Enabled");
#endif
	userInfLog("  Max. iter. per pix.     %14u", pCfg->numIterMax);
	userInfLog("  Pos X                   %14.3f", pCfg->posX);
	userInfLog("  Pos Y                   %14.3f", pCfg->posY);
	userInfLog("  Zoom                    %14.3e", pCfg->zoom);
	userInfLog("");
	userInfLog("  Driver type             %14s", env.typeDriver.c_str());
	userInfLog("  Num. Pool-threads       %14u", env.numThreadsPool);
	userInfLog("  Num. fillers            %14u", env.numFillers);
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
	double ips, numIter = (double)mpMbCreate->mNumIterations;
	double durMs = (double)mpMbCreate->mDurationMs;
	ConfigMandelbrot *pCfg = &mpMbCreate->cfg;

	userInfLog("\n");
	userInfLog("  Duration                %14zu [ms]", (size_t)durMs);
	userInfLog("  Iterations              %14.3e", numIter);
	ips = numIter / (durMs / 1000);
	userInfLog("  Iter. per second        %14.3e", ips);
	userInfLog("  Pixel * IPS             %14.3e", ips * pCfg->imgWidth * pCfg->imgHeight);
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

