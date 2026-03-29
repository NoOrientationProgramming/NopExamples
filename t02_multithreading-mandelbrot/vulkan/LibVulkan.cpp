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
static InstanceVulkan instInternal;

static VkDebugUtilsMessengerEXT vlkMessenger = VK_NULL_HANDLE;

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

	//dbgLog("layer count: %u", numLayers);

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

	//dbgLog("extension count: %u", numExtensions);

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
#
static void validationLayerAdd(vector<const char *> &layers, vector<const char *> &extensions)
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
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkDebugUtilsMessengerCreateInfoEXT.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_debug_utils.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/PFN_vkDebugUtilsMessengerCallbackEXT.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkDebugUtilsMessageSeverityFlagBitsEXT.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkDebugUtilsMessageTypeFlagBitsEXT.html
 */
static VkBool32 vulkanMessage(
				VkDebugUtilsMessageSeverityFlagBitsEXT severity,
				VkDebugUtilsMessageTypeFlagsEXT types,
				const VkDebugUtilsMessengerCallbackDataEXT *pData,
				void *pUser)
{
	(void)types;
	(void)pUser;

	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		dbgLog("\n### %s", pData->pMessage);
	else
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		infLog("\n### %s", pData->pMessage);
	else
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		wrnLog("\n### %s", pData->pMessage);
	else
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		errLog(-1, "\n### %s", pData->pMessage);
	else
		errLog(-1, "\n### %x %s", severity, pData->pMessage);

	return VK_FALSE;
}

static void messengerCreate(vector<const char *> &layers, vector<const char *> &extensions)
{
	if (!layers.size() || !extensions.size())
		return;

	PFN_vkCreateDebugUtilsMessengerEXT pFctCreate;
	VkInstance &vlk = instInternal.vlk;

	pFctCreate = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vlk,
					"vkCreateDebugUtilsMessengerEXT");
	if (!pFctCreate)
	{
		dbgLog("could not load vkCreateDebugUtilsMessengerEXT");
		return;
	}

	VkResult res;

	VkDebugUtilsMessengerCreateInfoEXT infoCreate = {};
	infoCreate.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	infoCreate.messageSeverity =
		/* VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	infoCreate.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	infoCreate.pfnUserCallback = vulkanMessage;

	res = pFctCreate(vlk, &infoCreate, NULL, &vlkMessenger);
	if (res != VK_SUCCESS)
		dbgLog("could not create Vulkan messenger");

	dbgLog("Vulkan messenger created");
}

static void vlkMessengerDestroy()
{
	if (!instInternal.ok || vlkMessenger == VK_NULL_HANDLE)
		return;

	dbgLog("destroying Vulkan messenger");

	PFN_vkDestroyDebugUtilsMessengerEXT pFctDestroy;

	pFctDestroy = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instInternal.vlk,
					"vkDestroyDebugUtilsMessengerEXT");
	if (!pFctDestroy)
	{
		dbgLog("could not load vkDestroyDebugUtilsMessengerEXT");
		return;
	}

	pFctDestroy(instInternal.vlk, vlkMessenger, NULL);
	vlkMessenger = VK_NULL_HANDLE;

	dbgLog("destroying Vulkan messenger: done");
}

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkDestroyInstance.html
 */
static void vlkGlobalDestruct()
{
	dbgLog("global Vulkan deinit");

	vlkMessengerDestroy();

	if (instInternal.ok)
	{
		vkDestroyInstance(instInternal.vlk, NULL);
		instInternal.vlk = VK_NULL_HANDLE;
		instInternal.ok = false;
	}

	dbgLog("global Vulkan deinit: done");
}

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkCreateInstance.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkResult.html
 */
InstanceVulkan instanceVulkanGet()
{
	lock_guard<mutex> lock(mtxInstance);

	if (instInternal.ok)
		return instInternal;

	vector<const char *> layers, extensions;
	VkResult res;

	memset(&instInternal, 0, sizeof(instInternal));
	instInternal.vlk = VK_NULL_HANDLE;

	validationLayerAdd(layers, extensions);

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

	res = vkCreateInstance(&infoCreate, NULL, &instInternal.vlk);
	if (res != VK_SUCCESS)
	{
		dbgLog("could not create Vulkan instance: %d", res);
		return instInternal;
	}

	dbgLog("Vulkan instance created");

	Processing::globalDestructorRegister(vlkGlobalDestruct);

	uint32_t versionInst;

	res = vkEnumerateInstanceVersion(&versionInst);
	if (res != VK_SUCCESS)
		dbgLog("could not get vulkan version");
	else
	{
		uint32_t major = VK_VERSION_MAJOR(versionInst);
		uint32_t minor = VK_VERSION_MINOR(versionInst);
		uint32_t patch = VK_VERSION_PATCH(versionInst);

		dbgLog("Version %u.%u.%u", major, minor, patch);
	}

	messengerCreate(layers, extensions);

	instInternal.ok = true;

	return instInternal;
}

/*
 * Literature
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkEnumeratePhysicalDevices.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDevice.html
 * - https://docs.vulkan.org/refpages/latest/refpages/source/vkGetPhysicalDeviceProperties.html
 */
void devicesVulkanList(InstanceVulkan &inst)
{
	uint32_t numDevices;
	VkResult res;

	res = vkEnumeratePhysicalDevices(inst.vlk, &numDevices, NULL);
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate physical devices");
		return;
	}

	//dbgLog("device count: %u", numDevices);

	vector<VkPhysicalDevice> devs(numDevices);

	res = vkEnumeratePhysicalDevices(inst.vlk, &numDevices, devs.data());
	if (res != VK_SUCCESS)
	{
		dbgLog("could not enumerate physical devices");
		return;
	}

	vector<VkPhysicalDevice>::iterator iter;
	VkPhysicalDeviceProperties props;

	iter = devs.begin();
	for (; iter != devs.end(); ++iter)
	{
		vkGetPhysicalDeviceProperties(*iter, &props);
		dbgLog("%s", props.deviceName);
	}
}

