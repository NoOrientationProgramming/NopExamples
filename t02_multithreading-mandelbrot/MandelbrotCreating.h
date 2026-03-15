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

#ifndef MANDELBROT_CREATING_H
#define MANDELBROT_CREATING_H

#include "Processing.h"
#include "MandelBlockFilling.h"
#include "ThreadPooling.h"
#include "LibBmp.h"

class MandelbrotCreating : public Processing
{

public:

	static MandelbrotCreating *create()
	{
		return new dNoThrow MandelbrotCreating;
	}

	std::string nameFile;
	ConfigMandelbrot cfg;

protected:

	virtual ~MandelbrotCreating();

private:

	MandelbrotCreating();
	MandelbrotCreating(const MandelbrotCreating &) = delete;
	MandelbrotCreating &operator=(const MandelbrotCreating &) = delete;

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success process();
	Success shutdown();
	void processInfo(char *pBuf, char *pBufEnd);

	bool lineFillersStart();
	Success linesProcess();

	void progressPrint();
	bool servicesStart();

	void hideCursor();
	void showCursor();

	/* member variables */
	uint32_t mStartMs;
	ThreadPooling *mpPool;
	char *mpBuffer;
	FileBmp mBmp;

	size_t mSzBuffer;

	size_t mIdxLineFiller;
	size_t mIdxLineDone;
	size_t mIdxProgress;

	char *mpLineFiller;
	char *mpLineDone;

	/* static functions */

	/* static variables */

	/* constants */

};

#endif

