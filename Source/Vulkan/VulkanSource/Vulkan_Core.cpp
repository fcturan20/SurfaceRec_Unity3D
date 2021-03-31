#include "Vulkan_Core.h"
#include "TuranAPI/Logger_Core.h"

namespace Vulkan {
	Vulkan_Core::Vulkan_Core() : FirstTriangle_CommandBuffers, VK_States() {
		//Set static GFX_API variable as created Vulkan_Core, because there will only one GFX_API in run-time
		//And we will use this SELF to give commands to GFX_API in window callbacks
		SELF = this;
		TuranAPI::LOG_STATUS("Vulkan systems are starting!");

		//Set error callback to handle all glfw errors (including initialization error)!
		//glfwSetErrorCallback(Vulkan_Core::GFX_Error_Callback);

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		Save_Monitors();

		//Note that: Vulkan initialization need a Window to be created before, so we should create one with GLFW
		Create_Window("Turan Engine");

		Initialization();
		Create_Renderer();

		TuranAPI::LOG_NOTCODED("Vulkan FileSystem isn't coded!\n", false);
		TuranAPI::LOG_NOTCODED("Vulkan's IMGUI support isn't coded!\n", false);

		TuranAPI::LOG_STATUS("Vulkan systems are started!");
	}

	void Vulkan_Core::Create_MainWindow() {
		TuranAPI::LOG_STATUS("Creating a window named");

		//Create window as it will share resources with Renderer Context to get display texture!
		GLFWwindow* glfw_window = glfwCreateWindow(1280, 720, "Vulkan Window", NULL, nullptr);
		WINDOW* Vulkan_Window = new WINDOW(1280, 720, GFX_API::WINDOW_MODE::WINDOWED, &CONNECTED_Monitors[0], CONNECTED_Monitors[0].REFRESH_RATE, "Vulkan Window", GFX_API::V_SYNC::VSYNC_OFF);
		//glfwSetWindowMonitor(glfw_window, NULL, 0, 0, Vulkan_Window->Get_Window_Mode().x, Vulkan_Window->Get_Window_Mode().y, Vulkan_Window->Get_Window_Mode().z);
		Vulkan_Window->GLFW_WINDOW = glfw_window;

		//Check and Report if GLFW fails
		if (glfw_window == NULL) {
			TuranAPI::LOG_CRASHING("Error: We failed to create the window because of GLFW!");
			glfwTerminate();
		}

		Main_Window = Vulkan_Window;
	}

	void Vulkan_Core::Initialization() {
		//GLFW initialized in Vulkan_Core::Vulkan_Core()
		
		Create_Instance();
#ifdef VULKAN_DEBUGGING
		Setup_Debugging();
#endif
		Create_Surface_forWindow((WINDOW*)Main_Window);
		Check_Computer_Specs();
		Setup_LogicalDevice();
		Create_SwapChain_forWindow((WINDOW*)Main_Window);
		GFX_API::Material_Type FirstTriangle_MatType;
		FirstTriangle_MatType.NAME = "FirstTriangle_MatType";
		FirstTriangle_MatType. = TuranAPI_ENUMs::VULKAN;
		Create_ShaderPipeline(FirstTriangle_ShaderPipeline, &FirstTriangle_MatType);
		Create_RenderPass();
		Create_GraphicsPipeline();
		Create_Framebuffers((WINDOW*)Main_Window);
		Create_CommandPool();
		Create_CommandBuffers();
		Create_Semaphores();
	}
	void Vulkan_Core::GFX_Error_Callback(int error_code, const char* description) {
		TuranAPI::LOG_CRASHING(description, true);
	}
	void Vulkan_Core::Create_Instance() {
		//APPLICATION INFO
		VkApplicationInfo App_Info = {};
		App_Info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		App_Info.pApplicationName = "Vulkan DLL";
		App_Info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		App_Info.pEngineName = "GFX API";
		App_Info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		App_Info.apiVersion = VK_API_VERSION_1_0;
		VK_States.Application_Info = App_Info;

		//CHECK SUPPORTED EXTENSIONs
		uint32_t extension_count;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		Vector<VkExtensionProperties> SupportedExtensions(LASTUSEDALLOCATOR, 0, extension_count);
		//Doesn't construct VkExtensionProperties object, so we have to use resize!
		SupportedExtensions.resize(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, SupportedExtensions.data());
		for (unsigned int i = 0; i < extension_count; i++) {
			VK_States.Supported_InstanceExtensionList.push_back(SupportedExtensions[i]);
			std::cout << "Supported Extension: " << SupportedExtensions[i].extensionName << " is added to the Vector!\n";
		}
		std::cout << "Supported Extension Count: " << extension_count << std::endl;
		VK_States.Is_RequiredInstanceExtensions_Supported();

		//CHECK SUPPORTED LAYERS
		vkEnumerateInstanceLayerProperties(&VK_States.Supported_LayerNumber, nullptr);
		VK_States.Supported_LayerList = new VkLayerProperties[VK_States.Supported_LayerNumber];
		vkEnumerateInstanceLayerProperties(&VK_States.Supported_LayerNumber, VK_States.Supported_LayerList);
		for (unsigned int i = 0; i < VK_States.Supported_LayerNumber; i++) {
			std::cout << VK_States.Supported_LayerList[i].layerName << " layer is supported!\n";
		}

		//INSTANCE CREATION INFO
		VkInstanceCreateInfo InstCreation_Info = {};
		InstCreation_Info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstCreation_Info.pApplicationInfo = &App_Info;

		//Extensions
		InstCreation_Info.enabledExtensionCount = VK_States.Required_InstanceExtensionNames->size();
		Vector<const char*> ExtensionNames(LASTUSEDALLOCATOR, 0, 10);
		for (unsigned int i = 0; i < VK_States.Required_InstanceExtensionNames->size(); i++) {
			ExtensionNames.push_back((*VK_States.Required_InstanceExtensionNames)[i]);
			std::cout << "Added an Extension: " << (*VK_States.Required_InstanceExtensionNames)[i] << std::endl;
		}
		InstCreation_Info.ppEnabledExtensionNames = ExtensionNames.data();

		//Validation Layers
#ifdef VULKAN_DEBUGGING
		const char* Validation_Layers[1] = {
			"VK_LAYER_KHRONOS_validation"
		};
		InstCreation_Info.enabledLayerCount = 1;
		InstCreation_Info.ppEnabledLayerNames = Validation_Layers;
#else
		InstCreation_Info.enabledLayerCount = 0;
		InstCreation_Info.ppEnabledLayerNames = nullptr;
#endif

		VK_States.Instance_CreationInfo = InstCreation_Info;

		if (vkCreateInstance(&InstCreation_Info, nullptr, &VK_States.Vulkan_Instance) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Failed to create a Vulkan Instance!");
		}
		TuranAPI::LOG_STATUS("Vulkan Instance is created successfully!");

	}
	void Vulkan_Core::Setup_Debugging() {
		VkDebugUtilsMessengerCreateInfoEXT DebugMessenger_CreationInfo = {};
		DebugMessenger_CreationInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		DebugMessenger_CreationInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		DebugMessenger_CreationInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		DebugMessenger_CreationInfo.pfnUserCallback = Vulkan_States::VK_DebugCallback;
		DebugMessenger_CreationInfo.pNext = nullptr;
		DebugMessenger_CreationInfo.pUserData = nullptr;

		if (VK_States.vkCreateDebugUtilsMessengerEXT()(VK_States.Vulkan_Instance, &DebugMessenger_CreationInfo, nullptr, &VK_States.Debug_Messenger) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Vulkan's Debug Callback system failed to start!");
		}
		TuranAPI::LOG_STATUS("Vulkan Debug Callback system is started!");
	}
	void Vulkan_Core::Create_Surface_forWindow(WINDOW* Vulkan_Window) {
		VkSurfaceKHR Window_Surface = {};
		if (glfwCreateWindowSurface(VK_States.Vulkan_Instance, Vulkan_Window->GLFW_WINDOW, nullptr, &Window_Surface) != VK_SUCCESS) {
			TuranAPI::LOG_ERROR("GLFW failed to create a window surface");
		}
		else {
			TuranAPI::LOG_STATUS("GLFW created a window surface!");
		}
		Vulkan_Window->Window_Surface = Window_Surface;
	}
	void Vulkan_Core::Check_Computer_Specs() {
		TuranAPI::LOG_STATUS("Started to check Computer Specifications!");

		//CHECK GPUs
		uint32_t GPU_NUMBER = 0;
		vkEnumeratePhysicalDevices(VK_States.Vulkan_Instance, &GPU_NUMBER, nullptr);
		Vector<VkPhysicalDevice> Physical_GPU_LIST(LASTUSEDALLOCATOR, 0, GPU_NUMBER);
		for (unsigned int i = 0; i < GPU_NUMBER; i++) {
			Physical_GPU_LIST.push_back(VkPhysicalDevice());
		}
		vkEnumeratePhysicalDevices(VK_States.Vulkan_Instance, &GPU_NUMBER, Physical_GPU_LIST.data());

		if (GPU_NUMBER == 0) {
			TuranAPI::LOG_CRASHING("There is no GPU that has Vulkan support! Updating your drivers or Upgrading the OS may help");
		}

		//GET GPU INFORMATIONs, QUEUE FAMILIES etc
		for (unsigned int i = 0; i < GPU_NUMBER; i++) {
			GPU* Vulkan_GPU = new GPU;
			Vulkan_GPU->Physical_Device = Physical_GPU_LIST[i];
			vkGetPhysicalDeviceProperties(Vulkan_GPU->Physical_Device, &Vulkan_GPU->Device_Properties);
			vkGetPhysicalDeviceFeatures(Vulkan_GPU->Physical_Device, &Vulkan_GPU->Device_Features);
			const char* VendorName = VK_States.Convert_VendorID_toaString(Vulkan_GPU->Device_Properties.vendorID);

			//SAVE BASIC INFOs TO THE GPU OBJECT
			Vulkan_GPU->MODEL = Vulkan_GPU->Device_Properties.deviceName;
			Vulkan_GPU->DRIVER_VERSION = Vulkan_GPU->Device_Properties.driverVersion;
			Vulkan_GPU->API_VERSION = Vulkan_GPU->Device_Properties.apiVersion;
			Vulkan_GPU->DRIVER_VERSION = Vulkan_GPU->Device_Properties.driverVersion;

			//CHECK IF GPU IS DISCRETE OR INTEGRATED
			switch (Vulkan_GPU->Device_Properties.deviceType) {
			case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				Vulkan_GPU->GPU_TYPE = GFX_API::DISCRETE_GPU;
				break;
			case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				Vulkan_GPU->GPU_TYPE = GFX_API::INTEGRATED_GPU;
				break;
			default:
				//const char* CrashingError = Text_Add("Vulkan_Core::Check_Computer_Specs failed to find GPU's Type (Only Discrete and Integrated GPUs supported!), Type is:",
					//std::to_string(Vulkan_GPU->Device_Properties.deviceType).c_str());
				TuranAPI::LOG_CRASHING("There is an error about GPU!");
				break;
			}

			//GET QUEUE FAMILIES, SAVE THEM TO GPU OBJECT, CHECK AND SAVE GRAPHICS,COMPUTE,TRANSFER QUEUEFAMILIES INDEX
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(Vulkan_GPU->Physical_Device, &queueFamilyCount, nullptr);
			Vulkan_GPU->QueueFamilies.resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(Vulkan_GPU->Physical_Device, &queueFamilyCount, Vulkan_GPU->QueueFamilies.data());

			for (unsigned int queuefamily_index = 0; queuefamily_index < Vulkan_GPU->QueueFamilies.size(); queuefamily_index++) {
				VkQueueFamilyProperties* QueueFamily = &Vulkan_GPU->QueueFamilies[queuefamily_index];
				if (QueueFamily->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					Vulkan_GPU->is_GraphicOperations_Supported = true;
					Vulkan_GPU->Graphics_QueueFamily = queuefamily_index;
					std::cout << "Graphics Queue Family Index: " << queuefamily_index << std::endl;
				}
				if (QueueFamily->queueFlags & VK_QUEUE_COMPUTE_BIT) {
					Vulkan_GPU->is_ComputeOperations_Supported = true;
					Vulkan_GPU->Transfer_QueueFamily = queuefamily_index;
				}
				if (QueueFamily->queueFlags & VK_QUEUE_TRANSFER_BIT) {
					Vulkan_GPU->is_TransferOperations_Supported = true;
					Vulkan_GPU->Compute_QueueFamily = queuefamily_index;
				}
				//Check Presentation Support
				VkBool32 is_Presentation_Supported;
				vkGetPhysicalDeviceSurfaceSupportKHR(Vulkan_GPU->Physical_Device, queuefamily_index, ((WINDOW*)ONSCREEN_Windows[0])->Window_Surface, &is_Presentation_Supported);
				if (is_Presentation_Supported) {
					Vulkan_GPU->is_DisplayOperations_Supported = true;
					Vulkan_GPU->Presentation_QueueFamilyIndex = queuefamily_index;
				}
			}
			DEVICE_GPUs.push_back(Vulkan_GPU);
		}
		TuranAPI::LOG_STATUS("Probably one GPU is detected!");
		if (DEVICE_GPUs.size() == 1) {
			GPU_TO_RENDER = DEVICE_GPUs[0];
			TuranAPI::LOG_STATUS("The renderer GPU selected as first GPU, because there is only one GPU");
		}
		else {
			TuranAPI::LOG_WARNING("There are more than one GPUs, please select one to use in rendering operations!");
			std::cout << "GPU index: ";
			int i = 0;
			std::cin >> i;
			while (i >= DEVICE_GPUs.size()) {
				std::cout << "Retry please, GPU index: ";
				int i = 0;
				std::cin >> i;
			}
			GPU_TO_RENDER = DEVICE_GPUs[i];
			TuranAPI::LOG_STATUS("GPU: " + GPU_TO_RENDER->MODEL + "is selected for rendering operations!");
		}


		TuranAPI::LOG_STATUS("Finished checking Computer Specifications!");
	}
	void Vulkan_Core::Setup_LogicalDevice() {
		TuranAPI::LOG_STATUS("Starting to setup logical device");
		GPU* Vulkan_GPU = (GPU*)this->GPU_TO_RENDER;
		//We don't need for now, so leave it empty. But GPU has its own feature list already
		VkPhysicalDeviceFeatures Features = {};

		//Queue Creation Processes
		if (Vulkan_GPU->is_GraphicOperations_Supported) {
			VkDeviceQueueCreateInfo GraphicsQueue;
			GraphicsQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			GraphicsQueue.queueCount = 1;
			GraphicsQueue.queueFamilyIndex = Vulkan_GPU->Graphics_QueueFamily;
			GraphicsQueue.pQueuePriorities = new float(1.0f);
			GraphicsQueue.pNext = nullptr;
			GraphicsQueue.flags = 0;
			Vulkan_GPU->QueueCreationInfos.push_back(GraphicsQueue);
		}
		if (Vulkan_GPU->is_DisplayOperations_Supported) {
			if (Vulkan_GPU->Graphics_QueueFamily == Vulkan_GPU->Presentation_QueueFamilyIndex) {
				TuranAPI::LOG_STATUS("Vulkan: Presentation Queue and Graphics Queue are the same, so don't need to create a Presentation Queue");
			}
			else {
				VkDeviceQueueCreateInfo DisplayQueue;
				DisplayQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				DisplayQueue.queueCount = 1;
				DisplayQueue.queueFamilyIndex = Vulkan_GPU->Presentation_QueueFamilyIndex;
				DisplayQueue.pQueuePriorities = new float(1.0f);
				DisplayQueue.pNext = nullptr;
				DisplayQueue.flags = 0;
				Vulkan_GPU->QueueCreationInfos.push_back(DisplayQueue);
			}
		}
		if (!Vulkan_GPU->is_GraphicOperations_Supported && !Vulkan_GPU->is_DisplayOperations_Supported) {
			TuranAPI::LOG_CRASHING("GPU doesn't support Graphics or Display Queue, so logical device creation failed!");
			return;
		}

		Vulkan_GPU->Logical_Device_CreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		Vulkan_GPU->Logical_Device_CreationInfo.flags = 0;
		Vulkan_GPU->Logical_Device_CreationInfo.pQueueCreateInfos = Vulkan_GPU->QueueCreationInfos.data();
		Vulkan_GPU->Logical_Device_CreationInfo.queueCreateInfoCount = static_cast<uint32_t>(Vulkan_GPU->QueueCreationInfos.size());
		Vulkan_GPU->Logical_Device_CreationInfo.pEnabledFeatures = &Features;
		VK_States.Is_RequiredDeviceExtensions_Supported(Vulkan_GPU);


		Vulkan_GPU->Logical_Device_CreationInfo.enabledExtensionCount = Vulkan_GPU->Required_DeviceExtensionNames->size();
		Vulkan_GPU->Logical_Device_CreationInfo.ppEnabledExtensionNames = Vulkan_GPU->Required_DeviceExtensionNames->data();

		Vulkan_GPU->Logical_Device_CreationInfo.enabledLayerCount = 0;

		if (vkCreateDevice(Vulkan_GPU->Physical_Device, &Vulkan_GPU->Logical_Device_CreationInfo, nullptr, &Vulkan_GPU->Logical_Device) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Vulkan failed to create a Logical Device!");
			return;
		}
		TuranAPI::LOG_STATUS("Vulkan created a Logical Device!");

		if (Vulkan_GPU->is_GraphicOperations_Supported) {
			VkQueue Graphics_Queue = {};
			vkGetDeviceQueue(Vulkan_GPU->Logical_Device, Vulkan_GPU->Graphics_QueueFamily, 0, &Graphics_Queue);
			Vulkan_GPU->Queues.push_back(Graphics_Queue);
		}
		if (Vulkan_GPU->is_DisplayOperations_Supported) {
			if (Vulkan_GPU->Presentation_QueueFamilyIndex == Vulkan_GPU->Graphics_QueueFamily) {
				TuranAPI::LOG_STATUS("Vulkan: Didn't create a VkQueue for Display, because it is same with Graphics");
			}
			else {
				VkQueue Display_Queue = {};
				vkGetDeviceQueue(Vulkan_GPU->Logical_Device, Vulkan_GPU->Presentation_QueueFamilyIndex, 0, &Display_Queue);
				Vulkan_GPU->Queues.push_back(Display_Queue);
			}
		}
		TuranAPI::LOG_STATUS("Created logical device succesfully!");
	}
	void Vulkan_Core::Create_SwapChain_forWindow(WINDOW* window) {
		TuranAPI::LOG_STATUS("Started to create SwapChain for GPU according to a Window");
		WINDOW* Vulkan_Window = (WINDOW*)ONSCREEN_Windows[0];
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;

		//Check GPU Surface Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Vulkan_GPU->Physical_Device, Vulkan_Window->Window_Surface, &Vulkan_GPU->SurfaceCapabilities);

		uint32_t FormatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Vulkan_GPU->Physical_Device, Vulkan_Window->Window_Surface, &FormatCount, nullptr);
		Vulkan_GPU->SurfaceFormats.resize(FormatCount);
		if (FormatCount != 0) {
			vkGetPhysicalDeviceSurfaceFormatsKHR(Vulkan_GPU->Physical_Device, Vulkan_Window->Window_Surface, &FormatCount, Vulkan_GPU->SurfaceFormats.data());
		}

		uint32_t PresentationModesCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(Vulkan_GPU->Physical_Device, Vulkan_Window->Window_Surface, &PresentationModesCount, nullptr);
		Vulkan_GPU->PresentationModes.resize(PresentationModesCount);
		if (PresentationModesCount != 0) {
			vkGetPhysicalDeviceSurfacePresentModesKHR(Vulkan_GPU->Physical_Device, Vulkan_Window->Window_Surface, &PresentationModesCount, Vulkan_GPU->PresentationModes.data());
		}

		//Choose Surface Format
		VkSurfaceFormatKHR Window_SurfaceFormat = {};
		for (unsigned int i = 0; i < Vulkan_GPU->SurfaceFormats.size(); i++) {
			VkSurfaceFormatKHR& SurfaceFormat = Vulkan_GPU->SurfaceFormats[i];
			if (SurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && SurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				Window_SurfaceFormat = SurfaceFormat;
			}
		}

		//Choose Surface Presentation Mode
		VkPresentModeKHR Window_PresentationMode = {};
		for (unsigned int i = 0; i < Vulkan_GPU->PresentationModes.size(); i++) {
			VkPresentModeKHR& PresentationMode = Vulkan_GPU->PresentationModes[i];
			if (PresentationMode == VK_PRESENT_MODE_FIFO_KHR) {
				Window_PresentationMode = PresentationMode;
			}
		}


		VkExtent2D Window_ImageExtent = { Vulkan_Window->Get_Window_Mode().x, Vulkan_Window->Get_Window_Mode().y };
		uint32_t image_count = 0;
		if (Vulkan_GPU->SurfaceCapabilities.maxImageCount > Vulkan_GPU->SurfaceCapabilities.minImageCount) {
			image_count = Vulkan_GPU->SurfaceCapabilities.minImageCount + 1;
		}
		else {
			TuranAPI::LOG_NOTCODED("I didn't understand what happens now, Vulkan_GPU->SurfaceCapabilities.maxImageCount < Vulkan_GPU->SurfaceCapabilities.minImageCount in Vulkan_Core::Create_SwapChain_forWindow", true);
		}
		Vulkan_Window->Window_SwapChainCreationInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		Vulkan_Window->Window_SwapChainCreationInfo.flags = 0;
		Vulkan_Window->Window_SwapChainCreationInfo.pNext = nullptr;
		Vulkan_Window->Window_SwapChainCreationInfo.presentMode = Window_PresentationMode;
		Vulkan_Window->Window_SwapChainCreationInfo.surface = Vulkan_Window->Window_Surface;
		Vulkan_Window->Window_SwapChainCreationInfo.minImageCount = image_count;
		Vulkan_Window->Window_SwapChainCreationInfo.imageFormat = Window_SurfaceFormat.format;
		Vulkan_Window->Window_SwapChainCreationInfo.imageColorSpace = Window_SurfaceFormat.colorSpace;
		Vulkan_Window->Window_SwapChainCreationInfo.imageExtent = Window_ImageExtent;
		Vulkan_Window->Window_SwapChainCreationInfo.imageArrayLayers = 1;
		Vulkan_Window->Window_SwapChainCreationInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		Vulkan_Window->Window_SwapChainCreationInfo.clipped = VK_TRUE;
		Vulkan_Window->Window_SwapChainCreationInfo.preTransform = Vulkan_GPU->SurfaceCapabilities.currentTransform;
		Vulkan_Window->Window_SwapChainCreationInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		Vulkan_Window->Window_SwapChainCreationInfo.oldSwapchain = nullptr;

		Vulkan_Window->Window_SwapChainCreationInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		Vulkan_Window->Window_SwapChainCreationInfo.queueFamilyIndexCount = 0;
		Vulkan_Window->Window_SwapChainCreationInfo.pQueueFamilyIndices = nullptr;

		if (vkCreateSwapchainKHR(Vulkan_GPU->Logical_Device, &Vulkan_Window->Window_SwapChainCreationInfo, nullptr, &Vulkan_Window->Window_SwapChain) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Failed to create a SwapChain for a Window");
		}
		TuranAPI::LOG_STATUS("Finished creating SwapChain for GPU according to a Window");

		//Get Swapchain images
		uint32_t created_imagecount = 0;
		vkGetSwapchainImagesKHR(Vulkan_GPU->Logical_Device, Vulkan_Window->Window_SwapChain, &created_imagecount, nullptr);
		Vulkan_Window->SwapChain_Images.resize(created_imagecount);
		vkGetSwapchainImagesKHR(Vulkan_GPU->Logical_Device, Vulkan_Window->Window_SwapChain, &created_imagecount, Vulkan_Window->SwapChain_Images.data());

		TuranAPI::LOG_STATUS("Finished getting VkImages of Swapchain");

		TuranAPI::LOG_STATUS("Started getting VkImageViews of Swapchain");
		Vulkan_Window->SwapChain_ImageViews.resize(Vulkan_Window->SwapChain_Images.size());
		for (unsigned int i = 0; i < Vulkan_Window->SwapChain_Images.size(); i++) {
			VkImageViewCreateInfo ImageView_ci = {};
			ImageView_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ImageView_ci.image = Vulkan_Window->SwapChain_Images[i];
			ImageView_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			//I'm tired, so set the value manually!
			ImageView_ci.format = Window_SurfaceFormat.format;
			ImageView_ci.flags = 0;
			ImageView_ci.pNext = nullptr;
			ImageView_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			ImageView_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			ImageView_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			ImageView_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			ImageView_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ImageView_ci.subresourceRange.baseArrayLayer = 0;
			ImageView_ci.subresourceRange.baseMipLevel = 0;
			ImageView_ci.subresourceRange.layerCount = 1;
			ImageView_ci.subresourceRange.levelCount = 1;

			if (vkCreateImageView(Vulkan_GPU->Logical_Device, &ImageView_ci, nullptr, &Vulkan_Window->SwapChain_ImageViews[i]) != VK_SUCCESS) {
				TuranAPI::LOG_CRASHING("Image View creation has failed!");
			}
			TuranAPI::LOG_STATUS("Created an Image View successfully!");
		}
	}
	void Vulkan_Core::Create_ShaderPipeline(ShaderPipeline* Pipeline_to_Create, GFX_API::Material_Type_Resource* MaterialType_to_Use) {
		ShaderPipeline* ShaderPipe = new ShaderPipeline;
		if (MaterialType_to_Use->LANGUAGE != TuranAPI_ENUMs::VULKAN) {
			TuranAPI::LOG_ERROR("Vulkan: A Material Type's ShaderPipeline failed to create because it's not coded for Vulkan!");
			return;
		}

		TuranAPI::LOG_STATUS("Started to create ShaderPipeline for Material Type");
		TuranAPI::LOG_STATUS("Started with Vertex Shader!");
		//Create Vertex Shader Module
		VkShaderModuleCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ci.flags = 0;
		ci.pNext = nullptr;
		unsigned int vertexcode_size = 0;
		ci.pCode = reinterpret_cast<const uint32_t*>
			(TAPIFILESYSTEM::Read_BinaryFile("C:/dev/TuranEngine/Vulkan/Content/FirstShaderVert.spv", vertexcode_size, LASTUSEDALLOCATOR));
		ci.codeSize = vertexcode_size;
		//ci.codeSize = glslang_program_SPIRV_get_size(program) * sizeof(unsigned int);
		//ci.pCode = glslang_program_SPIRV_get_ptr(program);

		VkShaderModule VertexShader_Module = {};
		if (vkCreateShaderModule(((GPU*)GPU_TO_RENDER)->Logical_Device, &ci, 0, &VertexShader_Module) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Vertex Shader Module is failed at creation!");
		}
		TuranAPI::LOG_STATUS("Vertex Shader Module is successfully created!");

		VkShaderModule FragmentShader_Module = {};
		VkShaderModuleCreateInfo fragmentci = {};
		fragmentci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		unsigned int fragmentcode_size = 0;
		fragmentci.pCode = reinterpret_cast<const uint32_t*>
			(TAPIFILESYSTEM::Read_BinaryFile("C:/dev/TuranEngine/Vulkan/Content/FirstShaderFrag.spv", fragmentcode_size, LASTUSEDALLOCATOR));
		fragmentci.codeSize = fragmentcode_size;


		if (vkCreateShaderModule(((GPU*)GPU_TO_RENDER)->Logical_Device, &fragmentci, 0, &FragmentShader_Module) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Fragment Shader Module is failed at creation!");
		}
		TuranAPI::LOG_STATUS("Fragment Shader Module is successfully created!");
		//Create Shader Pipeline Stage
		VkPipelineShaderStageCreateInfo Vertex_ShaderStage = {};
		Vertex_ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		Vertex_ShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		Vertex_ShaderStage.module = VertexShader_Module;
		Vertex_ShaderStage.pName = "main";

		VkPipelineShaderStageCreateInfo Fragment_ShaderStage = {};
		Fragment_ShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		Fragment_ShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		Fragment_ShaderStage.module = FragmentShader_Module;
		Fragment_ShaderStage.pName = "main";

		ShaderPipe->ShaderStageCreateInfos.push_back(Vertex_ShaderStage);
		ShaderPipe->ShaderStageCreateInfos.push_back(Fragment_ShaderStage);
		ShaderPipe->ShaderModules.push_back(VertexShader_Module);
		ShaderPipe->ShaderModules.push_back(FragmentShader_Module);
		FirstTriangle_ShaderPipeline = ShaderPipe;
		TuranAPI::LOG_STATUS("Shader Pipeline is created!");
	}
	void Vulkan_Core::Create_RenderPass() {
		TuranAPI::LOG_STATUS("Started to create Render Pass");
		VkAttachmentDescription Attachment_Description = {};
		Attachment_Description.format = ((WINDOW*)Main_Window)->Window_SwapChainCreationInfo.imageFormat;
		Attachment_Description.samples = VK_SAMPLE_COUNT_1_BIT;
		Attachment_Description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Attachment_Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		Attachment_Description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		Attachment_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		Attachment_Description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Attachment_Description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		Attachment_Description.flags = 0;

		VkAttachmentReference Attachment_Reference = {};
		Attachment_Reference.attachment = 0;
		Attachment_Reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription Subpass_Description = {};
		Subpass_Description.colorAttachmentCount = 1;
		Subpass_Description.pColorAttachments = &Attachment_Reference;
		Subpass_Description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		Subpass_Description.flags = 0;

		VkSubpassDependency Subpass_Dependencies = {};
		Subpass_Dependencies.dependencyFlags = 0;
		Subpass_Dependencies.srcSubpass = VK_SUBPASS_EXTERNAL;
		Subpass_Dependencies.dstSubpass = 0;
		Subpass_Dependencies.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		Subpass_Dependencies.srcAccessMask = 0;
		Subpass_Dependencies.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		Subpass_Dependencies.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		//RenderPass Creation
		VkRenderPassCreateInfo RenderPass_CreationInfo = {};
		RenderPass_CreationInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		RenderPass_CreationInfo.subpassCount = 1;
		RenderPass_CreationInfo.pSubpasses = &Subpass_Description;
		RenderPass_CreationInfo.attachmentCount = 1;
		RenderPass_CreationInfo.pAttachments = &Attachment_Description;
		RenderPass_CreationInfo.flags = 0;
		RenderPass_CreationInfo.pNext = nullptr;
		RenderPass_CreationInfo.dependencyCount = 1;
		RenderPass_CreationInfo.pDependencies = &Subpass_Dependencies;

		if (vkCreateRenderPass(((GPU*)GPU_TO_RENDER)->Logical_Device, &RenderPass_CreationInfo, nullptr, &FirstTriangle_RenderPass) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Render Pass creation has failed!");
		}
		TuranAPI::LOG_STATUS("Finished creating Render Pass");
	}
	void Vulkan_Core::Create_GraphicsPipeline() {
		TuranAPI::LOG_STATUS("Started to create Graphics Pipeline");
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;
		VkPipelineVertexInputStateCreateInfo VertexInputState = {};
		{
			VertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VertexInputState.flags = 0;
			VertexInputState.pNext = nullptr;
			VertexInputState.pVertexAttributeDescriptions = nullptr;
			VertexInputState.pVertexBindingDescriptions = nullptr;
			VertexInputState.vertexAttributeDescriptionCount = 0;
			VertexInputState.vertexBindingDescriptionCount = 0;
		}

		VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = {};
		{
			InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			InputAssemblyState.primitiveRestartEnable = false;
			InputAssemblyState.flags = 0;
			InputAssemblyState.pNext = nullptr;
		}

		VkPipelineViewportStateCreateInfo RenderViewportState = {};
		//Because there are pointers for this structs, I created here
		VkViewport RenderViewport = {};
		VkRect2D RenderScissor = {};
		{
			RenderViewport.width = (float)ONSCREEN_Windows[0]->Get_Window_Mode().x;
			RenderViewport.height = (float)ONSCREEN_Windows[0]->Get_Window_Mode().y;
			RenderViewport.minDepth = 0.0f;
			RenderViewport.maxDepth = 1.0f;
			RenderViewport.x = 0;
			RenderViewport.y = 0;

			RenderScissor.offset = { 0,0 };
			RenderScissor.extent = { (uint32_t)ONSCREEN_Windows[0]->Get_Window_Mode().x , (uint32_t)ONSCREEN_Windows[0]->Get_Window_Mode().y };

			RenderViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			RenderViewportState.scissorCount = 1;
			RenderViewportState.pScissors = &RenderScissor;
			RenderViewportState.viewportCount = 1;
			RenderViewportState.pViewports = &RenderViewport;
			RenderViewportState.pNext = nullptr;
			RenderViewportState.flags = 0;
		}

		VkPipelineRasterizationStateCreateInfo RasterizationState = {};
		{
			RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			RasterizationState.cullMode = VK_CULL_MODE_NONE;
			RasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
			RasterizationState.lineWidth = 1.0f;
			RasterizationState.depthClampEnable = VK_FALSE;
			RasterizationState.rasterizerDiscardEnable = VK_FALSE;

			RasterizationState.depthBiasEnable = VK_FALSE;
			RasterizationState.depthBiasClamp = 0.0f;
			RasterizationState.depthBiasConstantFactor = 0.0f;
			RasterizationState.depthBiasSlopeFactor = 0.0f;
		}

		VkPipelineMultisampleStateCreateInfo MSAAState = {};
		{
			MSAAState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			MSAAState.sampleShadingEnable = VK_FALSE;
			MSAAState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			MSAAState.minSampleShading = 1.0f;
			MSAAState.pSampleMask = nullptr;
			MSAAState.alphaToCoverageEnable = VK_FALSE;
			MSAAState.alphaToOneEnable = VK_FALSE;
		}


		VkPipelineColorBlendAttachmentState Attachment_ColorBlendState = {};
		VkPipelineColorBlendStateCreateInfo Pipeline_ColorBlendState = {};
		{
			Attachment_ColorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			Attachment_ColorBlendState.blendEnable = VK_FALSE;
			Attachment_ColorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			Attachment_ColorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			Attachment_ColorBlendState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
			Attachment_ColorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			Attachment_ColorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			Attachment_ColorBlendState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

			Pipeline_ColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			Pipeline_ColorBlendState.logicOpEnable = VK_FALSE;
			Pipeline_ColorBlendState.attachmentCount = 1;
			Pipeline_ColorBlendState.pAttachments = &Attachment_ColorBlendState;
			Pipeline_ColorBlendState.logicOp = VK_LOGIC_OP_COPY; // Optional
			Pipeline_ColorBlendState.blendConstants[0] = 0.0f; // Optional
			Pipeline_ColorBlendState.blendConstants[1] = 0.0f; // Optional
			Pipeline_ColorBlendState.blendConstants[2] = 0.0f; // Optional
			Pipeline_ColorBlendState.blendConstants[3] = 0.0f; // Optional
		}

		//There is no Dynamic State because I don't want to support it in this manual mess
		VkPipelineDynamicStateCreateInfo Dynamic_States = {};
		{
			Dynamic_States.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			Dynamic_States.dynamicStateCount = 0;
			Dynamic_States.pDynamicStates = nullptr;
		}

		//Pipeline Layout creation
		VkPipelineLayoutCreateInfo PipelineLayoutCreationInfo = {};
		{
			PipelineLayoutCreationInfo.pNext = nullptr;
			PipelineLayoutCreationInfo.flags = 0;
			PipelineLayoutCreationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			PipelineLayoutCreationInfo.setLayoutCount = 0; // Optional
			PipelineLayoutCreationInfo.pSetLayouts = nullptr; // Optional
			PipelineLayoutCreationInfo.pushConstantRangeCount = 0; // Optional
			PipelineLayoutCreationInfo.pPushConstantRanges = nullptr; // Optional

			vkCreatePipelineLayout(Vulkan_GPU->Logical_Device, &PipelineLayoutCreationInfo, nullptr, &FirstTriangle_PipelineLayout);
		}

		//There is no Depth-Stencil, so it will be nullptr

		VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = {};
		GraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		GraphicsPipelineCreateInfo.pColorBlendState = &Pipeline_ColorBlendState;
		GraphicsPipelineCreateInfo.pDepthStencilState = nullptr;
		GraphicsPipelineCreateInfo.pDynamicState = &Dynamic_States;
		GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssemblyState;
		GraphicsPipelineCreateInfo.pMultisampleState = &MSAAState;
		GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationState;
		GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputState;
		GraphicsPipelineCreateInfo.pViewportState = &RenderViewportState;
		GraphicsPipelineCreateInfo.layout = FirstTriangle_PipelineLayout;
		GraphicsPipelineCreateInfo.renderPass = FirstTriangle_RenderPass;
		GraphicsPipelineCreateInfo.subpass = 0;
		GraphicsPipelineCreateInfo.stageCount = 2;
		GraphicsPipelineCreateInfo.pStages = FirstTriangle_ShaderPipeline->ShaderStageCreateInfos.data();
		GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;		//Optional
		GraphicsPipelineCreateInfo.basePipelineIndex = -1;					//Optional
		GraphicsPipelineCreateInfo.flags = 0;
		GraphicsPipelineCreateInfo.pNext = nullptr;
		vkCreateGraphicsPipelines(Vulkan_GPU->Logical_Device, nullptr, 1, &GraphicsPipelineCreateInfo, nullptr, &FirstTriangle_GraphicsPipeline);
		TuranAPI::LOG_STATUS("Finished creating Graphics Pipeline");
	}
	void Vulkan_Core::Create_Framebuffers(WINDOW* Vulkan_Window) {
		TuranAPI::LOG_STATUS("Started to create Framebuffers");
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;
		Vulkan_Window->Swapchain_Framebuffers.resize(Vulkan_Window->SwapChain_Images.size());
		for (unsigned int i = 0; i < Vulkan_Window->SwapChain_Images.size(); i++) {
			VkImageView Attachments[] = { Vulkan_Window->SwapChain_ImageViews[i] };

			VkFramebufferCreateInfo FrameBuffer_ci = {};
			FrameBuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			FrameBuffer_ci.renderPass = FirstTriangle_RenderPass;
			FrameBuffer_ci.attachmentCount = 1;
			FrameBuffer_ci.pAttachments = Attachments;
			FrameBuffer_ci.height = Vulkan_Window->Window_SwapChainCreationInfo.imageExtent.height;
			FrameBuffer_ci.width = Vulkan_Window->Window_SwapChainCreationInfo.imageExtent.width;
			FrameBuffer_ci.layers = 1;
			FrameBuffer_ci.pNext = nullptr;
			FrameBuffer_ci.flags = 0;

			if (vkCreateFramebuffer(Vulkan_GPU->Logical_Device, &FrameBuffer_ci, nullptr, &Vulkan_Window->Swapchain_Framebuffers[i]) != VK_SUCCESS) {
				TuranAPI::LOG_CRASHING("Vulkan Framebuffer creation failed!");
			}
		}
		TuranAPI::LOG_STATUS("Finished creating Framebuffers");
	}
	void Vulkan_Core::Create_CommandPool() {
		TuranAPI::LOG_STATUS("Started to create Command Pool!");
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;

		VkCommandPoolCreateInfo CommandPool_ci = {};
		CommandPool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPool_ci.queueFamilyIndex = Vulkan_GPU->Graphics_QueueFamily;
		CommandPool_ci.pNext = nullptr;
		CommandPool_ci.flags = 0;

		if (vkCreateCommandPool(Vulkan_GPU->Logical_Device, &CommandPool_ci, nullptr, &FirstTriangle_CommandPool) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Vulkan Command Buffer allocation failed!");
		}

		TuranAPI::LOG_STATUS("Finished creating Command Pool!");
	}
	void Vulkan_Core::Create_CommandBuffers() {
		TuranAPI::LOG_STATUS("Started to create Command Buffers!");
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;

		FirstTriangle_CommandBuffers.push_back(VkCommandBuffer());
		FirstTriangle_CommandBuffers.push_back(VkCommandBuffer());
		FirstTriangle_CommandBuffers.push_back(VkCommandBuffer());

		VkCommandBufferAllocateInfo CommandBuffer_ai = {};
		CommandBuffer_ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBuffer_ai.commandPool = FirstTriangle_CommandPool;
		CommandBuffer_ai.commandBufferCount = FirstTriangle_CommandBuffers.size();
		std::cout << "\n\nCommand buffers size: " << FirstTriangle_CommandBuffers.size() << std::endl;
		CommandBuffer_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBuffer_ai.pNext = nullptr;
		if (vkAllocateCommandBuffers(Vulkan_GPU->Logical_Device, &CommandBuffer_ai, FirstTriangle_CommandBuffers.data()) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Vulkan Command Buffer allocation failed!");
		}
		TuranAPI::LOG_STATUS("Allocated Command Buffers");

		for (unsigned int CB_index = 0; CB_index < FirstTriangle_CommandBuffers.size(); CB_index++) {
			VkCommandBufferBeginInfo CommandBuffer_bi = {};
			CommandBuffer_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			CommandBuffer_bi.pInheritanceInfo = nullptr;
			CommandBuffer_bi.flags = 0;
			CommandBuffer_bi.pNext = nullptr;

			if (vkBeginCommandBuffer(FirstTriangle_CommandBuffers[CB_index], &CommandBuffer_bi) != VK_SUCCESS) {
				TuranAPI::LOG_CRASHING("Command buffer begin has failed!");
			}

			Begin_RenderPass(CB_index);
			Begin_DrawingFirstTriangle(CB_index);
			Finish_DrawingProgresses(CB_index);

		}
		TuranAPI::LOG_STATUS("Finished creating Command Buffers!");
	}
	void Vulkan_Core::Begin_RenderPass(unsigned int CB_Index) {
		TuranAPI::LOG_STATUS("Started to begin Render Pass!");
		VkClearValue ClearColor_Value = { 0.4f, 0.7f, 0.5f, 1.0f };
		VkRect2D RenderScissor = {};
		RenderScissor.offset = { 0,0 };
		RenderScissor.extent = { (uint32_t)ONSCREEN_Windows[0]->Get_Window_Mode().x , (uint32_t)ONSCREEN_Windows[0]->Get_Window_Mode().y };

		VkRenderPassBeginInfo RenderPass_bi = {};
		RenderPass_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPass_bi.renderPass = FirstTriangle_RenderPass;
		RenderPass_bi.renderArea = RenderScissor;
		RenderPass_bi.framebuffer = ((WINDOW*)ONSCREEN_Windows[0])->Swapchain_Framebuffers[CB_Index];
		RenderPass_bi.pNext = nullptr;
		RenderPass_bi.clearValueCount = 1;
		RenderPass_bi.pClearValues = &ClearColor_Value;

		vkCmdBeginRenderPass(FirstTriangle_CommandBuffers[CB_Index], &RenderPass_bi, VK_SUBPASS_CONTENTS_INLINE);

		TuranAPI::LOG_STATUS("Finished beginning Render Pass!");
	}
	void Vulkan_Core::Begin_DrawingFirstTriangle(unsigned int CB_Index) {
		TuranAPI::LOG_STATUS("Started to send First Triangle's drawing commands!");

		vkCmdBindPipeline(FirstTriangle_CommandBuffers[CB_Index], VK_PIPELINE_BIND_POINT_GRAPHICS, FirstTriangle_GraphicsPipeline);
		vkCmdDraw(FirstTriangle_CommandBuffers[CB_Index], 3, 1, 0, 0);

		TuranAPI::LOG_STATUS("Finished sending First Triangle's drawing commmands!");
	}
	void Vulkan_Core::Finish_DrawingProgresses(unsigned int CB_Index) {
		TuranAPI::LOG_STATUS("Started to close Rendering Operations!");

		vkCmdEndRenderPass(FirstTriangle_CommandBuffers[CB_Index]);
		vkEndCommandBuffer(FirstTriangle_CommandBuffers[CB_Index]);

		TuranAPI::LOG_STATUS("Finished closing Rendering Operations!");
	}
	void Vulkan_Core::Create_Semaphores() {
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;
		VkSemaphoreCreateInfo Semaphore_ci = {};
		Semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		Semaphore_ci.flags = 0;
		Semaphore_ci.pNext = nullptr;
		if (vkCreateSemaphore(Vulkan_GPU->Logical_Device, &Semaphore_ci, nullptr, &Wait_GettingFramebuffer) != VK_SUCCESS||
			vkCreateSemaphore(Vulkan_GPU->Logical_Device, &Semaphore_ci, nullptr, &Wait_RenderingFirstTriangle) != VK_SUCCESS) {
			TuranAPI::LOG_CRASHING("Semaphore creation has failed!");
		}

		//Set fences as signaled for the first frame!
		for (unsigned int i = 0; i < 3; i++) {
			VkFenceCreateInfo Fence_ci = {};
			Fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			Fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			Fence_ci.pNext = nullptr;
			if (vkCreateFence(Vulkan_GPU->Logical_Device, &Fence_ci, nullptr, &SwapchainFences[i]) != VK_SUCCESS) {
				TuranAPI::LOG_CRASHING("Fence creation has failed!");
			}
		}
	}


	void Vulkan_Core::Save_Monitors() {
		int monitor_count;
		GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
		TuranAPI::LOG_STATUS("Probably one monitor is detected!");
		for (unsigned int i = 0; i < monitor_count; i++) {
			GLFWmonitor* monitor = monitors[i];

			//Get monitor name provided by OS! It is a driver based name, so it maybe incorrect!
			const char* monitor_name = glfwGetMonitorName(monitor);
			GFX_API::MONITOR* gfx_monitor = new GFX_API::MONITOR(monitor, monitor_name);

			//Get videomode to detect at which resolution the OS is using the monitor
			const GLFWvidmode* monitor_vid_mode = glfwGetVideoMode(monitor);
			gfx_monitor->Set_Monitor_VidMode(monitor_vid_mode->width, monitor_vid_mode->height, monitor_vid_mode->blueBits, monitor_vid_mode->refreshRate);

			//Get monitor's physical size, developer may want to use it!
			glfwGetMonitorPhysicalSize(monitor, &gfx_monitor->PHYSICAL_WIDTH, &gfx_monitor->PHYSICAL_HEIGHT);

			CONNECTED_Monitors.push_back(gfx_monitor);
		}
	}

	void Vulkan_Core::Create_Renderer() {
		TuranAPI::LOG_NOTCODED("Vulkan_Core::Create_Renderer isn't coded!\n", false);
	}
	void Vulkan_Core::New_Frame() {
		GFX_API::GFX_Core::New_Frame();
		GPU* VK_GPU = (GPU*)GPU_TO_RENDER;
		WINDOW* VK_WINDOW = (WINDOW*)ONSCREEN_Windows[0];

	}

	void Vulkan_Core::Close_Window(GFX_API::WINDOW* window) {
		WINDOW* Vulkan_Window = ((WINDOW*)window);
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;
		
		vkDeviceWaitIdle(Vulkan_GPU->Logical_Device);
		for (unsigned int i = 0; i < Vulkan_Window->SwapChain_ImageViews.size(); i++) {
			vkDestroyImageView(Vulkan_GPU->Logical_Device, Vulkan_Window->SwapChain_ImageViews[i], nullptr);
			vkDestroyFramebuffer(Vulkan_GPU->Logical_Device, Vulkan_Window->Swapchain_Framebuffers[i], nullptr);
		}
		vkDestroySwapchainKHR(Vulkan_GPU->Logical_Device, Vulkan_Window->Window_SwapChain, nullptr);
		vkDestroySurfaceKHR(VK_States.Vulkan_Instance, Vulkan_Window->Window_Surface, nullptr);
		glfwDestroyWindow(Vulkan_Window->GLFW_WINDOW);

		//Then delete it from global GFX_WINDOW vector
		int window_list_index;
		for (int i = 0; i < ONSCREEN_Windows.size(); i++) {
			if (window == ONSCREEN_Windows[i])
				window_list_index = i;
		}
		ONSCREEN_Windows.erase(window_list_index);
	}

	void Vulkan_Core::Destroy_GFX_Resources() {
		TuranAPI::LOG_STATUS("Vulkan Core started to being destroyed!");

		TuranAPI::LOG_NOTCODED("IMGUI destroying process isn't coded!\n", false);
		//delete RENDERER;
		for (unsigned int i = 0; i < GFX->CONNECTED_Monitors.size(); i++) {
			GFX_API::MONITOR* monitor = GFX->CONNECTED_Monitors[i];
			delete monitor;
		}
		for (unsigned int i = 0; i < ONSCREEN_Windows.size(); i++) {
			GFX_API::WINDOW* window = ONSCREEN_Windows[i];
			Close_Window(window);
			delete window;
		}

		//WRITE VK OBJECT DESTRUCTIONs HERE! DON'T FORGET THAT YOU SHOULD START DESTRUCTION FROM LAST CREATED TO FIRST CREATED!
		GPU* Vulkan_GPU = (GPU*)GPU_TO_RENDER;
		vkDestroySemaphore(Vulkan_GPU->Logical_Device, Wait_GettingFramebuffer, nullptr);
		vkDestroySemaphore(Vulkan_GPU->Logical_Device, Wait_RenderingFirstTriangle, nullptr);
		for (unsigned int i = 0; i < 3; i++) {
			vkDestroyFence(Vulkan_GPU->Logical_Device, SwapchainFences[i], nullptr);
		}

		vkDestroyPipeline(Vulkan_GPU->Logical_Device, FirstTriangle_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(Vulkan_GPU->Logical_Device, FirstTriangle_PipelineLayout, nullptr);
		vkDestroyRenderPass(Vulkan_GPU->Logical_Device, FirstTriangle_RenderPass, nullptr);
		vkDestroyCommandPool(Vulkan_GPU->Logical_Device, FirstTriangle_CommandPool, nullptr);

		//Shader Module Deleting
		for (size_t i = 0; i < FirstTriangle_ShaderPipeline->ShaderModules.size(); i++) {
			VkShaderModule& ShaderModule = FirstTriangle_ShaderPipeline->ShaderModules[i];
			vkDestroyShaderModule(Vulkan_GPU->Logical_Device, ShaderModule, nullptr);
		}

		//GPU deleting
		for (unsigned int i = 0; i < DEVICE_GPUs.size(); i++) {
			GPU* a_Vulkan_GPU = (GPU*)DEVICE_GPUs[i];
			vkDestroyDevice(a_Vulkan_GPU->Logical_Device, nullptr);
		}
		VK_States.vkDestroyDebugUtilsMessengerEXT()(VK_States.Vulkan_Instance, VK_States.Debug_Messenger, nullptr);
		vkDestroyInstance(VK_States.Vulkan_Instance, nullptr);

		glfwTerminate();

		TuranAPI::LOG_STATUS("Vulkan Resources are destroyed!");
	}

	//CODE ALL OF THE BELOW FUNCTIONS!!!!
	void Vulkan_Core::Change_Window_Resolution(GFX_API::WINDOW* window, unsigned int width, unsigned int height) {

	}
	void Vulkan_Core::Set_Window_Focus(GFX_API::WINDOW* window, bool is_focused) {

	}
	void Vulkan_Core::Bind_Window_Context(GFX_API::WINDOW* window) {

	}

	//Callbacks

	//Renderer Operations
	void Vulkan_Core::Render_IMGUI() {

	}
	void Vulkan_Core::Load_GFX_Files() {

	}

	//Input (Keyboard-Controller) Operations
	void Vulkan_Core::Take_Inputs() {
		TuranAPI::LOG_STATUS("Take inputs!");
		glfwPollEvents();
	}


	GFX_API::GFX_Core* Start_VulkanSystems(TuranAPI::TAPI_Systems* TAPISystems) {
		TuranAPI::MemoryManagement::TMemoryManager::SELF = &TAPISystems->MemoryManager;
		TuranAPI::Logging::Logger::SELF = &TAPISystems->Log_Sys;
		TuranAPI::IMGUI_WindowManager::SELF = &TAPISystems->IMGUI_WindowSys;
		TuranAPI::Active_Profiling_Session::SELF = (TuranAPI::Profiling_Session*) & (TAPISystems->Profiling_Session);
		TuranAPI::LOG_STATUS("TuranAPI systems are started for Vulkan");

		//GFX_Core uses Memory and Logging systems in constructor
		//So we need to give these systems to the DLL before initializing
		GFX_API::Start_GFXDLL(TAPISystems);
		//We can safely create Vulkan now because GFX_Core has all systems it needs
		return new Vulkan_Core;
	}

	void Close_VulkanDLL() {
		std::cout << "Started to close GFX DLL!\n";
		delete GFX;
		GFX_API::Close_GFXDLL();
		std::cout << "Started to delete Vulkan DLL!\n";

		TuranAPI::Active_Profiling_Session::SELF = nullptr;
		TMemoryManager::SELF = nullptr;
		TuranAPI::Logging::Logger::SELF = nullptr;
		TuranAPI::IMGUI_WindowManager::SELF = nullptr;
		GFX = nullptr;
	}
}