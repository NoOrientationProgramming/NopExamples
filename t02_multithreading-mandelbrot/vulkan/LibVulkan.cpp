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

#include <string>
#include <vector>
#include <mutex>

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
static const char *validationLayerFind()
{
	const char *pStdVal = "VK_LAYER_KHRONOS_validation";
	uint32_t numLayers;
	const char *pVal;
	VkResult res;

	res = vkEnumerateInstanceLayerProperties(&numLayers, NULL);
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate layer properties");
		return NULL;
	}

	//dbgLog("Layer count: %u", numLayers);

	vector<VkLayerProperties> props(numLayers);

	res = vkEnumerateInstanceLayerProperties(&numLayers, props.data());
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate layer properties");
		return NULL;
	}

	vector<VkLayerProperties>::iterator iter;

	iter = props.begin();
	for (; iter != props.end(); ++iter)
	{
		//dbgLog("%-33s - %s", iter->layerName, iter->description);

		if (!strcmp(iter->layerName, pStdVal))
			pVal = pStdVal;;
	}

	if (!pVal)
		dbgLog("standard validation layer not found");

	return pVal;
}

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumerateInstanceExtensionProperties.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkExtensionProperties.html
 */
static const char *debugReportExtFind()
{
	const char *pStdVal = "VK_EXT_debug_utils";
	uint32_t numExtensions;
	VkResult res;
	const char *pExt;

	res = vkEnumerateInstanceExtensionProperties(NULL, &numExtensions, NULL);
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate extension properties");
		return NULL;
	}

	vector<VkExtensionProperties> exts(numExtensions);

	res = vkEnumerateInstanceExtensionProperties(NULL, &numExtensions, exts.data());
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate extension properties");
		return NULL;
	}

	vector<VkExtensionProperties>::iterator iter;

	iter = exts.begin();
	for (; iter != exts.end(); ++iter)
	{
		//dbgLog("%s", iter->extensionName);

		if (!strcmp(iter->extensionName, pStdVal))
			pExt = pStdVal;
	}

	if (!pExt)
		dbgLog("debug utils not found");

	return pExt;
}

static void validationLayerEnable(vector<const char *> &layers, vector<const char *> &extensions)
{
	const char *pLayer;
	const char *pExt;

	pLayer = validationLayerFind();
	if (!pLayer)
		return;

	dbgLog("validation layer found: %s", pLayer);

	pExt = debugReportExtFind();
	if (!pExt)
		return;

	dbgLog("debug utils found: %s", pExt);

	layers.push_back(pLayer);
	extensions.push_back(pExt);
}

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkCreateInstance.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkResult.html
 */
InstanceVulkan instanceVulkanGet()
{
	lock_guard<mutex> lock(mtxInstance);

	if (inst.ok)
		return inst;

	vector<const char *> layers, extensions;
	VkResult res;

	inst.ok = false;

	validationLayerEnable(layers, extensions);

	// Create instance

	VkApplicationInfo infoApp = {};
	infoApp.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	infoApp.pApplicationName = "Std app";
	infoApp.applicationVersion = 0;
	infoApp.pEngineName = "NaegVulkan";
	infoApp.engineVersion = 0;
	infoApp.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo infoCreate = {};
	infoCreate.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	infoCreate.flags = 0;
	infoCreate.pApplicationInfo = &infoApp;
#if 1
	if (layers.size())
	{
		infoCreate.enabledLayerCount = layers.size();
		infoCreate.ppEnabledLayerNames = layers.data();
	}

	if (extensions.size())
	{
		infoCreate.enabledExtensionCount = extensions.size();
		infoCreate.ppEnabledExtensionNames = extensions.data();
	}
#else
	infoCreate.enabledLayerCount = 0;
	infoCreate.ppEnabledLayerNames = NULL;
	infoCreate.enabledExtensionCount = 0;
	infoCreate.ppEnabledExtensionNames = NULL;
#endif
	res = vkCreateInstance(&infoCreate, NULL, &inst.vlk);
	if (res != VK_SUCCESS)
	{
		dbgLog("could not create Vulkan instance: %d", res);
		return inst;
	}

	dbgLog("Vulkan instance created");

	inst.ok = true;

	return inst;
}

