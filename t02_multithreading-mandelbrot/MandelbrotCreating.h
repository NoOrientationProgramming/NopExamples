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

protected:

	virtual ~MandelbrotCreating() {}

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

	Success linesProcess();
	void lineFill(size_t idx, char *pData, size_t len);
	void colorTest(char *pData, size_t idxLine, size_t idxPixel);
	void gradientBuild();
	void colorMandelbrot(char *pData, size_t idxLine, size_t idxPixel);
	size_t idxGradient(double t);
	void colorLerp(double t,
				int r1, int g1, int b1,
				int r2, int g2, int b2,
				int &ro, int &go, int &bo);
	void palette(double t, int &r, int &g, int &b);
	double fractionalIter(double zx, double zy, size_t numIter);
	size_t mandelbrot(double cx, double cy, double &zx, double &zy, size_t numIterMax);
	void progressPrint();
	bool servicesStart();

	/* member variables */
	uint32_t mStartMs;
	ThreadPooling *mpPool;
	char *mpBuffer;
	char *mpLine;
	FileBmp mBmp;
	size_t mSzData;
	size_t mSzLine;
	size_t mSzPadding;
	size_t mSzBuffer;
	size_t mIdxLine;
	size_t mIdxProgress;

	/* static functions */

	/* static variables */

	/* constants */

};

#endif

