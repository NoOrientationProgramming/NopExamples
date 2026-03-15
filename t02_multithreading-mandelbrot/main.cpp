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

#if defined(_WIN32)
#include <windows.h>
#else
#include <signal.h>
#endif
#include <iostream>
#include <chrono>
#include <thread>

#include "Supervising.h"
#include "LibFilesys.h"

using namespace std;
using namespace chrono;

Supervising *pApp = NULL;

#if defined(_WIN32)
BOOL WINAPI applicationCloseRequest(DWORD signal)
{
	if (signal != CTRL_C_EVENT)
		return FALSE;

	cout << endl;
	pApp->unusedSet();

	return TRUE;
}
#else
void applicationCloseRequest(int signum)
{
	(void)signum;

	cout << endl;
	pApp->unusedSet();
}
#endif

int main(int argc, char *argv[])
{
#if defined(_WIN32)
	// https://learn.microsoft.com/en-us/windows/console/setconsolectrlhandler
	BOOL okWin = SetConsoleCtrlHandler(applicationCloseRequest, TRUE);
	if (!okWin)
	{
		errLog(-1, "could not set ctrl handler");
		return 1;
	}
#else
	// http://man7.org/linux/man-pages/man7/signal.7.html
	signal(SIGINT, applicationCloseRequest);
	signal(SIGTERM, applicationCloseRequest);
#endif
	if (argc >= 2)
		levelLogSet(atoi(argv[1]));

	pApp = Supervising::create();
	if (!pApp)
	{
		errLog(-1, "could not create process");
		return 1;
	}

	while (1)
	{
		for (int i = 0; i < 12; ++i)
			pApp->treeTick();

		if (!pApp->progress())
			break;

		this_thread::sleep_for(chrono::milliseconds(15));
	}

	Success success = pApp->success();
	Processing::destroy(pApp);

	Processing::applicationClose();

	filesStdClose();

	return !(success == Positive);
}

