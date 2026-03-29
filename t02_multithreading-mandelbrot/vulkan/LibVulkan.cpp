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
#include <vector>

#include "LibVulkan.h"
#include "Processing.h"

using namespace std;

static mutex mtxInstance;

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumerateInstanceLayerProperties.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkLayerProperties.html
 * - apt install vulkan-validationlayers-dev
 */
static Success validationLayerCreate()
{
	uint32_t numLayers;
	VkResult res;

	res = vkEnumerateInstanceLayerProperties(&numLayers, NULL);
	if (res != VK_SUCCESS)
		return errLog(-1, "could not enumerate layer properties");

	//dbgLog("Layer count: %u", numLayers);

	vector<VkLayerProperties> props(numLayers);

	res = vkEnumerateInstanceLayerProperties(&numLayers, props.data());
	if (res != VK_SUCCESS)
		return errLog(-1, "could not enumerate layer properties");

	vector<VkLayerProperties>::iterator iter;
	bool found = false;

	iter = props.begin();
	for (; iter != props.end(); ++iter)
	{
		//dbgLog("%-33s - %s", iter->layerName, iter->description);

		if (!strstr(iter->layerName, "validation"))
			found = true;
	}

	if (!found)
	{
		dbgLog("standard validation layer not supported");
		return -1;
	}

	return Positive;
}

InstanceVulkan instanceVulkanGet()
{
	lock_guard<mutex> lock(mtxInstance);

	InstanceVulkan inst;
	Success success;

	inst.haveValLayer = false;

	success = validationLayerCreate();
	if (success == Positive)
		inst.haveValLayer = true;

	return inst;
}

