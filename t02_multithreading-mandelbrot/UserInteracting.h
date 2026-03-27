/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 27.03.2026

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

#ifndef USER_INTERACTING_H
#define USER_INTERACTING_H

#include "Processing.h"
#include "TelnetFiltering.h"

class UserInteracting : public Processing
{

public:

	static UserInteracting *create()
	{
		return new dNoThrow UserInteracting;
	}

	SOCKET mFd;

protected:

	virtual ~UserInteracting() {}

private:

	UserInteracting();
	UserInteracting(const UserInteracting &) = delete;
	UserInteracting &operator=(const UserInteracting &) = delete;

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success process();
	void processInfo(char *pBuf, char *pBufEnd);

	void msgMain();

	/* member variables */
	//uint32_t mStartMs;
	TelnetFiltering *mpFilt;

	/* static functions */

	/* static variables */

	/* constants */

};

#endif

