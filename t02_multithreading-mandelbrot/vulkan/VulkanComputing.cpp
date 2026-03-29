/*
  This file is part of the DSP-Crowd project
  https://www.dsp-crowd.com

  Author(s):
      - Johannes Natter, office@dsp-crowd.com

  File created on 29.03.2026

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

#include "VulkanComputing.h"

#define dForEach_ProcState(gen) \
		gen(StStart) \
		gen(StMain) \
		gen(StNop) \

#define dGenProcStateEnum(s) s,
dProcessStateEnum(ProcState);

#if 1
#define dGenProcStateString(s) #s,
dProcessStateStr(ProcState);
#endif

using namespace std;

/* Literature
 * - https://gpuopen.com/learn/understanding-vulkan-objects/
 * - https://github.com/Erkaman/vulkan_minimal_compute/blob/master/src/main.cpp
 */

VulkanComputing::VulkanComputing()
	: Processing("VulkanComputing")
	//, mStartMs(0)
{
	mState = StStart;
}

/* member functions */

Success VulkanComputing::process()
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

		break;
	case StNop:

		break;
	default:
		break;
	}

	return Pending;
}

void VulkanComputing::processInfo(char *pBuf, char *pBufEnd)
{
#if 1
	dInfo("State\t\t\t%s\n", ProcStateString[mState]);
#endif
}

/* static functions */

StructComputing *VulkanComputing::structComputeCreate()
{
	// Create template compute-structure
	return NULL;
}

