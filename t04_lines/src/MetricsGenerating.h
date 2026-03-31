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

#ifndef METRICS_GENERATING_H
#define METRICS_GENERATING_H

#include <string>
#include "Processing.h"

class MetricsGenerating : public Processing
{

public:

	static MetricsGenerating *create()
	{
		return new dNoThrow MetricsGenerating;
	}

	/* input */
	std::string mFilename;

	/* output */
	size_t mNumLines;
	std::string mLicense;
	std::string mName;
	std::string mDir;
	std::string mType;

protected:

	virtual ~MetricsGenerating() {}

private:

	MetricsGenerating();
	MetricsGenerating(const MetricsGenerating &) = delete;
	MetricsGenerating &operator=(const MetricsGenerating &) = delete;

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success process();
	void processInfo(char *pBuf, char *pBufEnd);

	/* member variables */
	//uint32_t mStartMs;

	/* static functions */

	/* static variables */

	/* constants */

};

#endif

