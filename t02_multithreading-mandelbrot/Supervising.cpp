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

		mState = StMain;

		break;
	case StMain:

		success = mpMbCreate->success();
		if (success == Pending)
			break;

		if (success != Positive)
			return procErrLog(-1, "could not create Mandelbrot picture");

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
	start(mpMbCreate);

	return true;
}

void Supervising::processInfo(char *pBuf, char *pBufEnd)
{
#if 1
	dInfo("State\t\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

