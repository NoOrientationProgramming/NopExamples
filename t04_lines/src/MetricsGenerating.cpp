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

#include <fstream>
#include <sstream>
#include "MetricsGenerating.h"

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

MetricsGenerating::MetricsGenerating()
	: Processing("MetricsGenerating")
	//, mStartMs(0)
	, mNumLines(0)
	, mLicense("-")
	, mType("Other")
{
	mState = StStart;
}

/* member functions */

Success MetricsGenerating::process()
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

		mState = StMain;

		break;
	case StMain:

		{
			ifstream f(mFilename.c_str());
			if (!f.is_open())
				return procErrLog(-1, "could not open file: %s", mFilename.c_str());

			string line;
			bool hasMit = false;
			bool hasGpl = false;

			while (getline(f, line)) {
				++mNumLines;
				if (!hasMit && line.find("free of charge") != string::npos)
					hasMit = true;
				if (!hasGpl && line.find("version 3 of the License") != string::npos)
					hasGpl = true;
			}
			f.close();

			if (hasMit)
				mLicense = "MIT";
			else if (hasGpl)
				mLicense = "GPLv3";

			// Extract name (basename without extension and without dir)
			string path = mFilename;
			// Normalize path separators
			for (size_t i = 0; i < path.size(); ++i)
				if (path[i] == '\\')
					path[i] = '/';

			size_t lastSlash = path.rfind('/');
			string basename = (lastSlash == string::npos) ? path : path.substr(lastSlash + 1);

			// Remove extension
			size_t dot = basename.rfind('.');
			mName = (dot == string::npos) ? basename : basename.substr(0, dot);

			// Directory (relative, strip leading "./")
			if (lastSlash == string::npos) {
				mDir = ".";
			} else {
				mDir = path.substr(0, lastSlash);
				if (mDir.size() >= 2 && mDir[0] == '.' && mDir[1] == '/')
					mDir = mDir.substr(2);
				if (mDir == ".")
					mDir = ".";
			}

			// Determine type
			size_t nameLen = mName.size();
			if (nameLen >= 3 && mName.substr(nameLen - 3) == "ing")
				mType = "Process";
			else if (nameLen >= 3 && mName.substr(0, 3) == "Lib")
				mType = "Library";
			else
				mType = "Other";
		}

		return Positive;

		break;
	default:
		break;
	}

	return Pending;
}

void MetricsGenerating::processInfo(char *pBuf, char *pBufEnd)
{
	(void)pBuf;
	(void)pBufEnd;
#if 0
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

