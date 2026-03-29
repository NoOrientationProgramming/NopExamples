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

#include <mutex>

#include "LibVulkan.h"
#include "Processing.h"

using namespace std;

static mutex mtxInstance;

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumerateInstanceLayerProperties.html
 */
static Success validationLayerCreate()
{
	uint32_t numLayers;
	VkResult res;

	res = vkEnumerateInstanceLayerProperties(&numLayers, NULL);
	if (res != VK_SUCCESS)
		return errLog(-1, "could not enumerate layer properties");

	dbgLog("Layer count: %u", numLayers);

	return Positive;
}

InstanceVulkan instanceVulkanGet()
{
	lock_guard<mutex> lock(mtxInstance);

	InstanceVulkan inst;
	Success success;

	inst.a = 0;

	success = validationLayerCreate();
	if (success != Positive)
	{
		wrnLog("could not create validation layer");
		return inst;
	}

	return inst;
}

