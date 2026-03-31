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

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <algorithm>
#include "FileSearching.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 0
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

using namespace std;

static bool dirNameSkip(const string &name)
{
	if (name.empty() || name[0] == '.')
		return true;
	if (name == "build" || name.find("build") != string::npos)
		return true;
	if (name == "resources")
		return true;
	return false;
}

FileSearching::FileSearching()
	: Processing("FileSearching")
	//, mStartMs(0)
{
	mState = StStart;
}

/* member functions */

Success FileSearching::process()
{
	//uint32_t curTimeMs = millis();
	//uint32_t diffMs = curTimeMs - mStartMs;
	//Success success;
#if 0
	dStateTrace;
#endif
	switch (mState)
	{
	case StStart:

		mDirsToSearch.push_back(".");
		mState = StMain;

		break;
	case StMain:

		if (mDirsToSearch.empty())
			return Positive;

		{
			size_t found = 0;

			while (!mDirsToSearch.empty() && found < 100) {
				string dir = mDirsToSearch.front();
				mDirsToSearch.pop_front();

				list<string> subdirs, matched;
				dirScan(dir, subdirs, matched);

				for (list<string>::iterator it = subdirs.begin();
						it != subdirs.end(); ++it)
					mDirsToSearch.push_back(*it);

				for (list<string>::iterator it = matched.begin();
						it != matched.end(); ++it) {
					mFiles.push_back(*it);
					++found;
				}
			}
		}

		if (!mDirsToSearch.empty())
			return Pending;

		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

void FileSearching::dirScan(const string &path,
                            list<string> &subdirs,
                            list<string> &matched)
{
#if defined(_WIN32)
	WIN32_FIND_DATAA ffd;
	HANDLE h = FindFirstFileA((path + "\\*").c_str(), &ffd);
	if (h == INVALID_HANDLE_VALUE)
		return;

	do {
		string name = ffd.cFileName;
		if (name == "." || name == "..")
			continue;

		string full = path + "\\" + name;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (!dirNameSkip(name))
				subdirs.push_back(full);
		} else {
			size_t dot = name.rfind('.');
			if (dot != string::npos && name.substr(dot + 1) == mExt)
				matched.push_back(full);
		}
	} while (FindNextFileA(h, &ffd));

	FindClose(h);
#else
	DIR *d = opendir(path.c_str());
	if (!d)
		return;

	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		string name = entry->d_name;
		if (name == "." || name == "..")
			continue;

		string full = path + "/" + name;

		struct stat st;
		if (stat(full.c_str(), &st) != 0)
			continue;

		if (S_ISDIR(st.st_mode)) {
			if (!dirNameSkip(name))
				subdirs.push_back(full);
		} else if (S_ISREG(st.st_mode)) {
			size_t dot = name.rfind('.');
			if (dot != string::npos && name.substr(dot + 1) == mExt)
				matched.push_back(full);
		}
	}

	closedir(d);
#endif
}

void FileSearching::processInfo(char *pBuf, char *pBufEnd)
{
	(void)pBuf;
	(void)pBufEnd;
#if 0
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

