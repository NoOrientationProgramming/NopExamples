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

#ifndef USER_INTERACTING_H
#define USER_INTERACTING_H

#include <string>

#include "Processing.h"
#include "TcpTransfering.h"

class UserInteracting : public Processing
{

public:

	static UserInteracting *create(SOCKET fdPeer)
	{
		return new dNoThrow UserInteracting(fdPeer);
	}

	UserInteracting(SOCKET fdPeer);

protected:

	UserInteracting() = delete;
	virtual ~UserInteracting() {}

private:

	UserInteracting(const UserInteracting &) = delete;
	UserInteracting &operator=(const UserInteracting &) = delete;

	Success process();
	Success shutdown();
	Success msgReceive(std::string &msg);
	void processInfo(char *pBuf, char *pBufEnd);

	uint32_t mStateSd;
	SOCKET mFdPeer;
	TcpTransfering *mpConn;
	std::string mMsgLast;
	bool mQuitByUser;

};

#endif

