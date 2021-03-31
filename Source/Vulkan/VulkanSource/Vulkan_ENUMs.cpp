#include "Vulkan_Includes.h"

namespace Vulkan {
	Vulkan_States::Vulkan_States()  {

	}
	const char* const* Vulkan_States::Get_Supported_LayerNames(const VkLayerProperties* list, uint32_t length) {
		const char** NameList = new const char* [length];
		for (unsigned int i = 0; i < length; i++) {
			NameList[i] = list[i].layerName;
		}
		return NameList;
	}
	
	
	void Vulkan_States::Is_RequiredInstanceExtensions_Supported() {
		Required_InstanceExtensionNames = new vector<const char*>;
		//DEFINE REQUIRED EXTENSIONS
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (unsigned int i = 0; i < glfwExtensionCount; i++) {
			Required_InstanceExtensionNames->push_back(glfwExtensions[i]);
			std::cout << glfwExtensions[i] << " GLFW extension is required!\n";
		}

		#ifdef VULKAN_DEBUGGING
			Required_InstanceExtensionNames->push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		#endif

		//CHECK IF ALL OF THE REQUIRED EXTENSIONS SUPPORTED
		for (unsigned int i = 0; i < Required_InstanceExtensionNames->size(); i++) {
			bool Is_Found = false;
			for (unsigned int supported_extension_index = 0; supported_extension_index < Supported_InstanceExtensionList.size(); supported_extension_index++) {
				if (strcmp((*Required_InstanceExtensionNames)[i], Supported_InstanceExtensionList[supported_extension_index].extensionName)) {
					Is_Found = true;
					break;
				}
			}
			if (Is_Found == false) {
				TuranAPI::LOG_CRASHING("A required extension isn't supported!");
			}
		}
		TuranAPI::LOG_STATUS("All of the Vulkan Instance extensions are checked!");
	}

	void Vulkan_States::Is_RequiredDeviceExtensions_Supported(GPU* Vulkan_GPU) {
		Vulkan_GPU->Required_DeviceExtensionNames = new Vector<const char*>(LASTUSEDALLOCATOR, 0, 1000);
		//GET SUPPORTED DEVICE EXTENSIONS
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(Vulkan_GPU->Physical_Device, nullptr, &extensionCount, nullptr);
		Vulkan_GPU->Supported_DeviceExtensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(Vulkan_GPU->Physical_Device, nullptr, &extensionCount, Vulkan_GPU->Supported_DeviceExtensions.data());


		//CODE REQUIRED DEVICE EXTENSIONS
		Vulkan_GPU->Required_DeviceExtensionNames->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);


		TuranAPI::LOG_STATUS("Required Extensions for Vulkan API is set!");

		//CHECK IF ALL OF THE REQUIRED EXTENSIONS SUPPORTED
		for (unsigned int i = 0; i < Vulkan_GPU->Required_DeviceExtensionNames->size(); i++) {
			bool Is_Found = false;
			for (unsigned int supported_extension_index = 0; supported_extension_index < Vulkan_GPU->Supported_DeviceExtensions.size(); supported_extension_index++) {
				std::cout << "Checking against: " << Vulkan_GPU->Supported_DeviceExtensions[supported_extension_index].extensionName << std::endl;
				if (strcmp((*Vulkan_GPU->Required_DeviceExtensionNames)[i], Vulkan_GPU->Supported_DeviceExtensions[supported_extension_index].extensionName)) {
					Is_Found = true;
					break;
				}
			}
			if (Is_Found == false) {
				TuranAPI::LOG_CRASHING((*Vulkan_GPU->Required_DeviceExtensionNames)[i]);
			}
		}
		TuranAPI::LOG_STATUS("Checked Required Device Extensions for the GPU!");
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan_States::VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Message_Severity, VkDebugUtilsMessageTypeFlagsEXT Message_Type, const VkDebugUtilsMessengerCallbackDataEXT* pCallback_Data, void* pUserData) {
		String Callback_Type = "";
		switch (Message_Type) {
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			Callback_Type = "VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT : Some event has happened that is unrelated to the specification or performance\n";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			Callback_Type = "VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake\n";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			Callback_Type = "VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan\n";
			break;
		default:
			TuranAPI::LOG_CRASHING("Vulkan Callback has returned a unsupported Message_Type");
			return true;
			break;
		}

		switch (Message_Severity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			TuranAPI::LOG_STATUS(pCallback_Data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			TuranAPI::LOG_WARNING(pCallback_Data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			TuranAPI::LOG_STATUS(pCallback_Data->pMessage);
			break;
		default:
			TuranAPI::LOG_CRASHING("Vulkan Callback has returned a unsupported Message_Severity");
			return true;
			break;
		}

		TuranAPI::LOG_NOTCODED("Vulkan Callback has lots of data to debug such as used object's name, object's type etc\n", false);
		return false;
	}

	PFN_vkCreateDebugUtilsMessengerEXT Vulkan_States::vkCreateDebugUtilsMessengerEXT() {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Vulkan_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func == nullptr) {
			TuranAPI::LOG_ERROR("Vulkan failed to load vkCreateDebugUtilsMessengerEXT function!");
			return nullptr;
		}
		return func;
	}

	PFN_vkDestroyDebugUtilsMessengerEXT Vulkan_States::vkDestroyDebugUtilsMessengerEXT() {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Vulkan_Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func == nullptr) {
			TuranAPI::LOG_ERROR("Vulkan failed to load vkDestroyDebugUtilsMessengerEXT function!");
			return nullptr;
		}
		return func;
	}
	

	const char* Vulkan_States::Convert_VendorID_toaString(uint32_t VendorID) {
		switch (VendorID) {
		case 0x1002:
			return "AMD";
		case 0x10DE:
			return "Nvidia";
		case 0x8086:
			return "Intel";
		case 0x13B5:
			return "ARM";
		default:
			TuranAPI::LOG_CRASHING("Vulkan_Core::Check_Computer_Specs failed to find GPU's Vendor, Vendor ID is: " + VendorID);
			return "NULL";
		}
	}
	WINDOW::WINDOW(unsigned int width, unsigned int height, GFX_API::GFX_ENUM display_mode, GFX_API::MONITOR* display_monitor, unsigned int refresh_rate, const char* window_name, GFX_API::GFX_ENUM v_sync)
		: GFX_API::WINDOW(width, height, display_mode, display_monitor, refresh_rate, window_name, v_sync)
		, SwapChain_Images(LASTUSEDALLOCATOR, 1, 3), Swapchain_Framebuffers(LASTUSEDALLOCATOR, 1, 3), SwapChain_ImageViews(LASTUSEDALLOCATOR, 1, 3) {}

	GPU::GPU() : QueueFamilies(LASTUSEDALLOCATOR, 1, 4), QueueCreationInfos(LASTUSEDALLOCATOR, 1, 10), Queues(LASTUSEDALLOCATOR, 1, 4), Supported_DeviceExtensions(LASTUSEDALLOCATOR, 100, 10000),
		SurfaceFormats(LASTUSEDALLOCATOR, 10, 100), PresentationModes(LASTUSEDALLOCATOR, 1, 10){

	}
}