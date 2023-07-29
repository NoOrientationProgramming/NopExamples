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

#ifndef LOG_CATCHING_H
#define LOG_CATCHING_H

#include <string>
#include <list>

#include "Processing.h"

class LogCatching : public Processing
{

public:

	static LogCatching *create()
	{
		return new (std::nothrow) LogCatching;
	}

	static uint32_t numLines;
	static std::string nameBase;

protected:

	LogCatching();
	virtual ~LogCatching() {}

private:

	LogCatching(const LogCatching &) : Processing("") {}
	LogCatching &operator=(const LogCatching &) { return *this; }

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success process();
	void processInfo(char *pBuf, char *pBufEnd);

	bool logSave(bool triggeredByUser = false);

	/* member variables */
	uint32_t mState;
	uint32_t mStartMs;
	uint32_t mCntLines;
	std::string mFragmentLine;
	std::list<std::string> mLines;

	/* static functions */
	static void logSaveRequest(int signum);

	/* static variables */
	static LogCatching *pLog;

	/* constants */

};

#endif

