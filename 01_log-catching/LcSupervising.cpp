/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 27.07.2023

  Copyright (C) 2023-now Authors and www.dsp-crowd.com

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

#include "LcSupervising.h"
#include "SystemDebugging.h"
#include "LogCatching.h"

using namespace std;

LcSupervising::LcSupervising()
	: Processing("LcSupervising")
	, mDebug(false)
	, mpApp(NULL)
{}

/* member functions */

Success LcSupervising::initialize()
{
	if (mDebug)
	{
		SystemDebugging *pDbg = SystemDebugging::create(this);
		if (!pDbg)
			return procErrLog(-1, "could not create process");

		//pDbg->procTreeDisplaySet(false);
		pDbg->portStartSet(3030);
		pDbg->listenLocalSet();

		start(pDbg);
	}

	mpApp = start(LogCatching::create());
	if (!mpApp)
		return procErrLog(-1, "could not create process");

	mpApp->procTreeDisplaySet(true);

	return Positive;
}

Success LcSupervising::process()
{
	return mpApp->success();
}

/* static functions */

