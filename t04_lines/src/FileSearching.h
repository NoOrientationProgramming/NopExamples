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

#ifndef FILE_SEARCHING_H
#define FILE_SEARCHING_H

#include <string>
#include <list>
#include "Processing.h"

class FileSearching : public Processing
{

public:

	static FileSearching *create()
	{
		return new dNoThrow FileSearching;
	}

	/* input */
	std::string mExt;

	/* output */
	std::list<std::string> mFiles;

protected:

	virtual ~FileSearching() {}

private:

	FileSearching();
	FileSearching(const FileSearching &) = delete;
	FileSearching &operator=(const FileSearching &) = delete;

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success process();
	void processInfo(char *pBuf, char *pBufEnd);

	void dirScan(const std::string &path,
	             std::list<std::string> &subdirs,
	             std::list<std::string> &matched);

	/* member variables */
	//uint32_t mStartMs;
	std::list<std::string> mDirsToSearch;

	/* static functions */

	/* static variables */

	/* constants */

};

#endif

