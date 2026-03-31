/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 31.03.2026

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

#include "env.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 0
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
	, mpApp(NULL)
{
	mState = StStart;
}

/* member functions */

Success Supervising::process()
{
	//uint32_t curTimeMs = millis();
	//uint32_t diffMs = curTimeMs - mStartMs;
	//Success success;
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

		mState = StMain;

		break;
	case StMain:

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

		cancel(mpApp);

		mStateSd = StSdAppDoneWait;

		break;
	case StSdAppDoneWait:

		if (mpApp->progress())
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
	SystemDebugging *pDbg;

	pDbg = SystemDebugging::create(this);
	if (!pDbg) {
		procWrnLog("could not create process");
		return false;
	}

	pDbg->listenLocalSet();
	pDbg->procTreeDisplaySet(false);
	start(pDbg);

	mpApp = MsgDispatching::create();
	if (!mpApp) {
		procWrnLog("could not create process");
		return false;
	}

	start(mpApp);

	return true;
}

void Supervising::processInfo(char *pBuf, char *pBufEnd)
{
	(void)pBuf;
	(void)pBufEnd;
#if 0
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

