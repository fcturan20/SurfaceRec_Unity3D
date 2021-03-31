#pragma once
#include "Vulkan_ENUMs.h"
#include "GFX/GFX_Core.h"

#include "Renderer/Vulkan_Resource.h"
#include "Renderer/VK_GPUContentManager.h"
#include "Renderer/Vulkan_Renderer_Core.h"
#include "Vulkan_FileSystem.h"

//RenderGraph will have specify which features it needs, so initialize Vulkan as little requirements as possible with as much info about system as possible.
//When application that uses this DLL, Application Developer should define a RenderGraph to do some Rendering.
//While specifying, he will specify features and we will test them later in somewhere (Maybe Bind_RenderGraph is good place for this)
namespace Engine{}
namespace Vulkan {
	class VK_API Vulkan_Core : public GFX_API::GFX_Core {
		void Create_MainWindow();

		Vulkan_States VK_States;
		VkPipelineLayout FirstTriangle_PipelineLayout;
		VkRenderPass FirstTriangle_RenderPass;
		VkPipeline FirstTriangle_GraphicsPipeline = VK_NULL_HANDLE;
		VK_ShaderPipeline* FirstTriangle_ShaderPipeline = nullptr;
		VkCommandPool FirstTriangle_CommandPool = {};
		vector<VkCommandBuffer> FirstTriangle_CommandBuffers;
		VkSemaphore Wait_GettingFramebuffer;
		VkSemaphore Wait_RenderingFirstTriangle;
		VkFence SwapchainFences[3];


		virtual void Initialization() override;
		virtual void Check_Computer_Specs() override;
		virtual void Save_Monitors() override;
		virtual void Create_Renderer() override;

		//Window Operations
		virtual void Change_Window_Resolution(unsigned int width, unsigned int height) override;

		//Input (Keyboard-Controller) Operations
		virtual void Take_Inputs() override;

		//Resource Destroy Operations
		virtual void Destroy_GFX_Resources() override;


		static void GFX_Error_Callback(int error_code, const char* description);

		//Validation Layers are actived if TURAN_DEBUGGING is defined
		void Create_Instance();
		//Initialization calls this function if TURAN_DEBUGGING is defined
		void Setup_Debugging();
		void Create_Surface_forWindow(WINDOW* vulkan_window);
		//A Graphics Queue is created for learning purpose
		void Setup_LogicalDevice();
		void Create_SwapChain_forWindow(WINDOW* vulkan_window);
		void Create_RenderPass();
		//I want to render first triangle, so create here
		void Create_GraphicsPipeline();
		//This function compiles SPIR-V from GLSL and instead of storing SPIR-V, uses SPIR-V to create a Shader Module
		void Create_ShaderPipeline(VK_ShaderPipeline* Pipeline_to_Create, GFX_API::Material_Type* MaterialType_to_Use);
		void Create_Framebuffers(WINDOW* vulkan_window);
		void Create_CommandPool();
		void Create_CommandBuffers();
		void Begin_RenderPass(unsigned int CB_Index);
		void Begin_DrawingFirstTriangle(unsigned int CB_Index);
		void Finish_DrawingProgresses(unsigned int CB_Index);
		void Create_Semaphores();
	public:
		Vulkan_Core();
	};
}