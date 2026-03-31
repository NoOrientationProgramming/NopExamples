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

#ifndef MSG_DISPATCHING_H
#define MSG_DISPATCHING_H

#include <string>
#include <list>
#include <vector>
#include "Processing.h"
#include "FileSearching.h"
#include "MetricsGenerating.h"

struct FileMetrics {
	size_t numLines;
	std::string license;
	std::string name;
	std::string dir;
	std::string type;
};

class MsgDispatching : public Processing
{

public:

	static MsgDispatching *create()
	{
		return new dNoThrow MsgDispatching;
	}

protected:

	virtual ~MsgDispatching() {}

private:

	MsgDispatching();
	MsgDispatching(const MsgDispatching &) = delete;
	MsgDispatching &operator=(const MsgDispatching &) = delete;

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success process();
	void processInfo(char *pBuf, char *pBufEnd);

	bool metricsStart();
	bool metricsAllDone();
	void resultsCollect();
	void resultsPager();

	/* member variables */
	//uint32_t mStartMs;
	FileSearching *mpSearch;
	std::list<MetricsGenerating *> mLstMetrics;
	std::vector<FileMetrics> mResults;

	/* static functions */

	/* static variables */

	/* constants */

};

#endif

