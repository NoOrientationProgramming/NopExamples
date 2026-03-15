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

GradientStop MandelBlockFilling::gradient[cNumGradients] = {};

MandelBlockFilling::MandelBlockFilling()
	: Processing("MandelBlockFilling")
	//, mStartMs(0)
	, pCfg(NULL)
	, idxLine(0)
{
	mState = StStart;
}

/* member functions */

Success MandelBlockFilling::process()
{
	//uint32_t curTimeMs = millis();
	//uint32_t diffMs = curTimeMs - mStartMs;
	//Success success;
#if 0
	dStateTrace;
#endif
	switch (mState)
	{
	case StStart:

		if (!pCfg)
			return procErrLog(-1, "config pointer not set");

		mState = StMain;

		break;
	case StMain:

		if (idxLine < 5)
			procDbgLog("Line %u finished", idxLine);

		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

void MandelBlockFilling::processInfo(char *pBuf, char *pBufEnd)
{
#if 1
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

void MandelBlockFilling::gradientBuild()
{
	GradientStop *pKey1, *pKey2, *pGrad;
	size_t i, s, k = 0;
	double t;

	for (; k < cNumKeysGradient - 1; ++k)
	{
		for (s = 0; s < cScaleGradient; ++s)
		{
			pKey1 = &keysGradient[k];
			pKey2 = pKey1 + 1;

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

void MandelBlockFilling::colorLerp(double t,
			int r1, int g1, int b1,
			int r2, int g2, int b2,
			int &ro, int &go, int &bo)
{
	ro = r1 + (r2 - r1) * t;
	go = g1 + (g2 - g1) * t;
	bo = b1 + (b2 - b1) * t;
}

