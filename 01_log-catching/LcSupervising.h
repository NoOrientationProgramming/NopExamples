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

#ifndef LC_SUPERVISING_H
#define LC_SUPERVISING_H

#include "Processing.h"

class LcSupervising : public Processing
{

public:

	static LcSupervising *create()
	{
		return new (std::nothrow) LcSupervising;
	}

	bool mDebug;

protected:

	LcSupervising();
	virtual ~LcSupervising() {}

private:

	LcSupervising(const LcSupervising &) : Processing("") {}
	LcSupervising &operator=(const LcSupervising &) { return *this; }

	/*
	 * Naming of functions:  objectVerb()
	 * Example:              peerAdd()
	 */

	/* member functions */
	Success initialize();
	Success process();

	/* member variables */
	Processing *mpApp;

	/* static functions */

	/* static variables */

	/* constants */

};

#endif

