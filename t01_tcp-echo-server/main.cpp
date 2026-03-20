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

#ifndef APP_HAS_TCLAP
#define APP_HAS_TCLAP 0
#endif

#ifdef __unix__
#include "signal.h"
#endif
#include <iostream>
#include <thread>
#if APP_HAS_TCLAP
#include <tclap/CmdLine.h>
#endif

#if APP_HAS_TCLAP
#include "TclapOutput.h"
#endif
#include "Supervising.h"
#include "LibDspc.h"

#include "env.h"

using namespace std;
using namespace chrono;
#if APP_HAS_TCLAP
using namespace TCLAP;
#endif

#define dPortListeningDefault "5000"
const int cPortMax = 64000;

Environment env;
Processing *pApp = NULL;

#if APP_HAS_TCLAP
class AppHelpOutput : public TclapOutput {};
#endif

// OS signal handler => Tell the application what to do on Ctrl-C
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
	// Register OS signal handlers
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
	env.haveTclap = 1;
	env.verbosity = 0;
#if defined(__unix__)
	env.coreDump = false;
#endif
	env.portListening = atoi(dPortListeningDefault);

#if APP_HAS_TCLAP
	int res;

	CmdLine cmd("Command description message", ' ', appVersion());

	AppHelpOutput aho;
#if 1
	aho.package = dPackageName;
	aho.versionApp = dVersion;
	aho.nameApp = dAppName;
	aho.copyright = " (C) 2025 DSP-Crowd Electronics GmbH";
#endif
	cmd.setOutput(&aho);

	ValueArg<int> argVerbosity("v", "verbosity", "Verbosity: high => more output", false, 0, "uint8");
	cmd.add(argVerbosity);
#if defined(__unix__)
	SwitchArg argCoreDump("", "core-dump", "Enable core dumps", false);
	cmd.add(argCoreDump);
#endif
	ValueArg<uint16_t> argPortListening("", "port-listening", "Listening port of TCP server. Default: " dPortListeningDefault,
								false, env.portListening, "uint16");
	cmd.add(argPortListening);

	cmd.parse(argc, argv);

	res = argVerbosity.getValue();
	if (res > 0 && res < 6)
		env.verbosity = res;
#if defined(__unix__)
	env.coreDump = argCoreDump.getValue();
#endif
	res = argPortListening.getValue();
	if (res > 0 && res <= cPortMax)
		env.portListening = res;
#else
	env.haveTclap = 0;
	env.verbosity = 2;

	if (argc >= 2)
	{
		if (!strcmp(argv[1], "--help"))
		{
			cout << "This is a help dummy" << endl;
			return 0;
		}

		env.verbosity = atoi(argv[1]);
	}
#endif
	levelLogSet(env.verbosity);

	pApp = Supervising::create();
	if (!pApp)
	{
		cerr << "could not create process" << endl;
		return 1;
	}

	while (1)
	{
		pApp->treeTick();
		this_thread::sleep_for(chrono::milliseconds(10));

		if (pApp->progress())
			continue;

		break;
	}

	Success success = pApp->success();
	Processing::destroy(pApp);

	Processing::applicationClose();

	return !(success == Positive);
}

