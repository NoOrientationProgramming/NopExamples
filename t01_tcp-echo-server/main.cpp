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

#include <iostream>
#include <thread>

#include "Supervising.h"
#ifdef __unix__
#include "signal.h"
#endif

using namespace std;

Processing *pApp = NULL;

#ifdef __unix__
void applicationCloseRequest(int signum)
{
	(void)signum;

	if (!pApp)
		return;

	cout << endl;
	pApp->unusedSet();
}
#endif

int main(int argc, char *argv[])
{
#ifdef __unix__
	signal(SIGINT, applicationCloseRequest);
	signal(SIGTERM, applicationCloseRequest);
#endif
	if (argc >= 2)
		levelLogSet(atoi(argv[1]));

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

