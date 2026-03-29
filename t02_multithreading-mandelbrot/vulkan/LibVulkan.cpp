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
static InstanceVulkan inst;


/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumerateInstanceLayerProperties.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkLayerProperties.html
 * - apt install vulkan-validationlayers-dev
 */
static string validationLayerFind()
{
	uint32_t numLayers;
	VkResult res;
	string str;

	res = vkEnumerateInstanceLayerProperties(&numLayers, NULL);
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate layer properties");
		return str;
	}

	//dbgLog("Layer count: %u", numLayers);

	vector<VkLayerProperties> props(numLayers);

	res = vkEnumerateInstanceLayerProperties(&numLayers, props.data());
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate layer properties");
		return str;
	}

	vector<VkLayerProperties>::iterator iter;

	iter = props.begin();
	for (; iter != props.end(); ++iter)
	{
		//dbgLog("%-33s - %s", iter->layerName, iter->description);

		if (!strcmp(iter->layerName, "VK_LAYER_KHRONOS_validation"))
			str = iter->layerName;
	}

	if (!str.size())
		dbgLog("standard validation layer not found");

	return str;
}

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumerateInstanceExtensionProperties.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkExtensionProperties.html
 */
static string debugReportExtFind()
{
	uint32_t numExtensions;
	VkResult res;
	string str;

	res = vkEnumerateInstanceExtensionProperties(NULL, &numExtensions, NULL);
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate extension properties");
		return str;
	}

	vector<VkExtensionProperties> exts(numExtensions);

	res = vkEnumerateInstanceExtensionProperties(NULL, &numExtensions, exts.data());
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate extension properties");
		return str;
	}

	vector<VkExtensionProperties>::iterator iter;

	iter = exts.begin();
	for (; iter != exts.end(); ++iter)
	{
		//dbgLog("%s", iter->extensionName);

		if (!strcmp(iter->extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
			str = iter->extensionName;
	}

	if (!str.size())
		dbgLog("debug report extension not found");

	return str;
}

static void validationLayerEnable()
{
	string layer, extension;

	layer = validationLayerFind();
	if (!layer.size())
		return;

	dbgLog("validation layer found: %s", layer.c_str());

	extension = debugReportExtFind();
	if (!extension.size())
		return;

	dbgLog("debug report extension found: %s", extension.c_str());

	inst.haveValLayer = true;
	inst.layers.push_back(layer);
	inst.extensions.push_back(extension);
}

InstanceVulkan instanceVulkanGet()
{
	lock_guard<mutex> lock(mtxInstance);

	if (inst.ok)
		return inst;

	validationLayerEnable();

	return inst;
}

