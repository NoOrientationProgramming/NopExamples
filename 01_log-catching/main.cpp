/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  Copyright (C) 2017-now Authors and www.dsp-crowd.com
*/

#include <signal.h>
#include <tclap/CmdLine.h>
#include <iostream>

#include "TclapOutput.h"
#include "LcSupervising.h"
#include "LogCatching.h"

using namespace std;
using namespace TCLAP;

mutex envMtx;

LcSupervising *pApp = NULL;

class AppHelpOutput : public TclapOutput {};

/*
Literature
- http://man7.org/linux/man-pages/man7/signal.7.html
  - for enums: kill -l
  - sys/signal.h
  SIGHUP  1     hangup
  SIGINT  2     interrupt
  SIGQUIT 3     quit
  SIGILL  4     illegal instruction (not reset when caught)
  SIGTRAP 5     trace trap (not reset when caught)
  SIGABRT 6     abort()
  SIGPOLL 7     pollable event ([XSR] generated, not supported)
  SIGFPE  8     floating point exception
  SIGKILL 9     kill (cannot be caught or ignored)
- https://www.usna.edu/Users/cs/aviv/classes/ic221/s16/lec/19/lec.html
- http://www.alexonlinux.com/signal-handling-in-linux
*/
void applicationCloseRequest(int signum)
{
	(void)signum;
	cout << endl;
	pApp->unusedSet();
}

void licensesPrint()
{
	cout << endl;
	cout << "This program uses the following external components" << endl;
	cout << endl;

	cout << "TCLAP" << endl;
	cout << "https://tclap.sourceforge.net/" << endl;
	cout << "MIT" << endl;
	cout << endl;
}

int main(int argc, char *argv[])
{
	CmdLine cmd("Command description message", ' ', "unknown");

	AppHelpOutput aho;
	cmd.setOutput(&aho);

	SwitchArg argDebug("d", "debug", "Enable debugging daemon", false);
	cmd.add(argDebug);
	ValueArg<int> argVerbosity("v", "verbosity", "Verbosity: high => more output", false, 3, "int");
	cmd.add(argVerbosity);
	SwitchArg argLicenses("", "licenses", "Show dependencies", false);
	cmd.add(argLicenses);

	ValueArg<int> argNumLines("n", "num-lines", "Number of lines to be saved", false, 100, "int");
	cmd.add(argNumLines);

	ValueArg<string> argNameBase("b", "name-base", "Basename of the output file", false, "app", "string");
	cmd.add(argNameBase);

	cmd.parse(argc, argv);

	levelLogSet(argVerbosity.getValue());

	/* https://www.gnu.org/software/libc/manual/html_node/Termination-Signals.html */
	signal(SIGINT, applicationCloseRequest);
	signal(SIGTERM, applicationCloseRequest);

	if (argLicenses.getValue())
	{
		licensesPrint();
		return 0;
	}

	pApp = LcSupervising::create();
	if (!pApp)
	{
		errLog(-1, "could not create process");
		return 1;
	}

	pApp->mDebug = argDebug.getValue();

	LogCatching::numLines = argNumLines.getValue();
	LogCatching::nameBase = argNameBase.getValue();

	pApp->procTreeDisplaySet(true);

	size_t coreBurst;

	while (1)
	{
		for (coreBurst = 0; coreBurst < 16; ++coreBurst)
			pApp->treeTick();

		this_thread::sleep_for(chrono::milliseconds(2));

		if (pApp->progress())
			continue;

		break;
	}

	Success success = pApp->success();
	Processing::destroy(pApp);

	Processing::applicationClose();

	return !(success == Positive);

}

