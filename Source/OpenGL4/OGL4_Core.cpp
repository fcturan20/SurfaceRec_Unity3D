#include "OGL4_Core.h"
#include "IMGUI/IMGUI_OGL4.h"
#include "OGL4_Display.h"
#include "Renderer/OGL4_GPUContentManager.h"

using namespace TuranAPI;

namespace OpenGL4 {
	OpenGL4_Core::OpenGL4_Core() : GFX_API::GFX_Core() {
		//Set static GFX_API variable to created OGL3_SYS, because there will only one GFX_API in run-time
		//And we will use this GFX_API_OBJ to give commands to GFX_API in window callbacks
		SELF = this;
		LOG_STATUS("OpenGL 4 systems are starting!");

		Initialization();
		Save_Monitors();

		//Create renderer class and renderer window
		Create_Renderer();

		ContentManager = new OpenGL4::GPU_ContentManager;
		((OpenGL4::GPU_ContentManager*)ContentManager)->RENDERER = RENDERER;
		LOG_STATUS("GFXContentManager initialization finished!");


		WINDOW* OGL4_WINDOW = (WINDOW*)Main_Window;
		IMGUI->Create_Context(((WINDOW*)Main_Window)->GLFWWINDOW);
		IMGUI->GPUContentManager = ContentManager;
		glfwMakeContextCurrent(((WINDOW*)Main_Window)->GLFWWINDOW);

		LOG_STATUS("OpenGL 4 systems are started!");
	}
	OpenGL4_Core::~OpenGL4_Core() {
		std::cout << "OpenGL4_Core destructor is called!\n";
		Destroy_GFX_Resources();
	}


	//WINDOW OPERATIONs

	void OpenGL4_Core::Change_Window_Resolution(unsigned int width, unsigned int height) {
		glfwMakeContextCurrent(((WINDOW*)Main_Window)->GLFWWINDOW);
		Main_Window->Change_Width_Height(width, height);
		glViewport(0, 0, width, height);
	}
	void OpenGL4_Core::Show_RenderTarget_onWindow(unsigned int RenderTarget_GFXID) {
		LOG_NOTCODED("OpenGL4_Core::Show_RenderTarget_onWindow isn't coded because glFramebufferTexture2D doesn't work on default Framebuffer of OpenGL!\n", false);
		glfwMakeContextCurrent(((WINDOW*)Main_Window)->GLFWWINDOW);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		unsigned int RenderTarget_GLID = *(unsigned int*)GFXContentManager->Find_GFXTexture_byID(RenderTarget_GFXID)->GL_ID;
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTarget_GLID, 0);
	}

	void OpenGL4_Core::GFX_Error_Callback(int error_code, const char* description) {
		LOG_CRASHING(description, true);
	}

	void OpenGL4_Core::Initialization() {
		//Set error callback to handle all glfw errors (including initialization error)!
		glfwSetErrorCallback(OpenGL4_Core::GFX_Error_Callback);

		//Initialize GLFW with OpenGL3
		int status = glfwInit();
		if (!status) {
			LOG_ERROR("GLFW initialization error!");
		}


		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	void OpenGL4_Core::Check_Computer_Specs() {

	}

	void OpenGL4_Core::Save_Monitors() {
		int monitor_count;
		GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
		std::cout << "Detected monitors count: " << monitor_count << std::endl;
		for (unsigned int i = 0; i < monitor_count; i++) {
			GLFWmonitor* monitor = monitors[i];

			//Get monitor name provided by OS! It is a driver based name, so it maybe incorrect!
			const char* monitor_name = glfwGetMonitorName(monitor);
			GFX_API::MONITOR gfx_monitor(monitor, monitor_name);

			//Get videomode to detect at which resolution the OS is using the monitor
			const GLFWvidmode* monitor_vid_mode = glfwGetVideoMode(monitor);
			gfx_monitor.Set_Monitor_VidMode(monitor_vid_mode->width, monitor_vid_mode->height, monitor_vid_mode->blueBits, monitor_vid_mode->refreshRate);

			//Get monitor's physical size, developer may want to use it!
			glfwGetMonitorPhysicalSize(monitor, &gfx_monitor.PHYSICAL_WIDTH, &gfx_monitor.PHYSICAL_HEIGHT);

			CONNECTED_Monitors.push_back(gfx_monitor);
		}
		LOG_STATUS("Saved monitors!");
	}

	//Create window in windowed mode, in final production: A window context should be known (Which screen mode, which GUIs to render etc.) and create this window according to it!s
	void OpenGL4_Core::Create_MainWindow() {
		//Create window as it will share resources with Renderer Context to get display texture!
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		GLFWwindow* window_id = glfwCreateWindow(1280, 720, "OpenGL Window", NULL, NULL);
		Main_Window = new WINDOW(1280, 720, GFX_API::WINDOW_MODE::WINDOWED, &CONNECTED_Monitors[0], CONNECTED_Monitors[0].REFRESH_RATE, "OpenGL Window", GFX_API::V_SYNC::VSYNC_OFF);
		WINDOW* OGL4_WINDOW = (WINDOW*)Main_Window;
		OGL4_WINDOW->GLFWWINDOW = window_id;
		//glfwSetWindowMonitor(window_id, NULL, 0, 0, Main_Window->Get_Window_Mode().x, Main_Window->Get_Window_Mode().y, Main_Window->Get_Window_Mode().z);
		glfwMakeContextCurrent(window_id);

		//Check and Report if GLFW fails
		if (window_id == NULL) {
			LOG_CRASHING("Failed to create main window because of GLFW!");
			glfwTerminate();
		}
		
		glfwSetKeyCallback(window_id, KeyboardCallback);
		glfwSetMouseButtonCallback(window_id, MouseCallback);
		glfwSwapInterval(0);
	}

	void OpenGL4_Core::Create_Renderer() {
		//Create renderer class!
		RENDERER = new OpenGL4_Renderer;
		RENDERER->SELF = RENDERER;

		Create_MainWindow();


		//Check and Report if GLAD fails
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			LOG_ERROR("We failed to create the window because of GLAD");
		}

		//Setup Debug Callback
		glEnable(GL_DEBUG_OUTPUT);
		glDisable(GL_SCISSOR_TEST);


		Check_Computer_Specs();

		IMGUI->GFX_IMGUI = new IMGUI_OGL4;
		//Set IMGUI's main renderer to IMGUI_OGL3!
		//All of the IMGUI contexts will use IMGUI's functionality to specify their rendering data!
		//But IMGUI will always use this class to render these datas!
		//So, GFX_IMGUI shouldn't be Window Context specific!
		if (!IMGUI->Check_IMGUI_Version()) {
			LOG_ERROR("Error: IMGUI version check error!");
		}
	}

	//CALLBACKs

	void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		LOG_NOTCODED("OpenGL4_Core::MessageCallback isn't coded!", true);
	}
	//Set true if user uses window, otherwise false
	void OpenGL4_Core::window_focus_callback(GLFWwindow* window, int focused) {
		LOG_ERROR("Window Focus Callback isn't coded!");
	}
	//Set window and framebuffer size when windowed window is resized
	void OpenGL4_Core::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		SELF->Change_Window_Resolution(width, height);
		std::cout << "Callback Width: " << width << std::endl;
		std::cout << "Callback Height: " << height << std::endl;
	}
	void OpenGL4_Core::window_close_callback(GLFWwindow* window) {
		LOG_ERROR("Window Close Callback isn't coded!");
	}
	void OpenGL4_Core::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GFX_API::KEYBOARD_KEYs gfxkey = Convert_Key_to_Engine(key);
		bool& state = ((OpenGL4_Core*)GFX)->IO.isKeyPressed[GFX_API::GetKeyIndex(gfxkey)];
		if (action == GLFW_PRESS) {
			state = true;
		}
		else {
			state = false;
		}
	}
	void OpenGL4_Core::MouseCallback(GLFWwindow* window, int key, int action, int mods) {
		GFX_API::MOUSE_BUTTONs gfxkey = Convert_GLFWButton_toGFXButton(key);
		bool& state = ((OpenGL4_Core*)GFX)->IO.isMouseClicked[GFX_API::GetMouseButtonIndex(gfxkey)];
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			state = true;
		}
		else {
			state = false;
		}
	}


	//RENDERING OPERATIONs
	void OpenGL4_Core::Run_RenderGraphs() {
		glfwMakeContextCurrent(((WINDOW*)Main_Window)->GLFWWINDOW);
		glViewport(0, 0, 1920, 1080);
		glEnable(GL_PROGRAM_POINT_SIZE);
		for (unsigned int i = 0; i < RenderGraphs.size(); i++) {
			RenderGraphs[i]->Run_RenderGraph();
		}
		RenderGraphs.clear();
	}

	//Input (Keyboard-Controller) Operations

	//1) Send keys status' from the last frame to the Engine!
	//2) Take this frame's key status' from GLFW! But it will be used next frame!
	void OpenGL4_Core::Take_Inputs() {
		/*
		//For each key, set key pressed status with GLFW check!
		//This sends keys' status from last frame!
		if (true)
		{
			//KEYBOARD INPUT HANDLING

			for (KEYBOARD_KEYs key : Keyboard_Controller::Return_ALL_Keys()) {
				if (glfwGetKey(glfwGetCurrentContext(), Convert_Key_to_GLFW_Key(key)) == GLFW_PRESS) {
					Keyboard_Controller::Set_Key_is_Pressed(key, true);
					cout << "Key: " << key << " is pressed!\n";
				}
				//If Key isn't pressed this frame, set it as false because Maybe at previous frame it was pressed!
				else {
					Keyboard_Controller::Set_Key_is_Pressed(key, false);
				}
			}

			//MOUSE INPUT HANDLING

			switch (Mouse_Controller::CURSOR_MODE) {
			case MOUSE_INPUT_UNLIMITED:
				glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				break;
			case MOUSE_INPUT_HIDDEN:
				glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				break;
			case MOUSE_INPUT_NORMAL:
				glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				break;

			default:
				cout << "Unsupported Cursor Mode in Mouse_Controller class!\n";
			}
			for (MOUSE_BUTTONs button : Mouse_Controller::Return_ALL_Buttons()) {
				if (glfwGetMouseButton(glfwGetCurrentContext(), Convert_MouseButton_to_GLFW_Key(button)) == GLFW_PRESS) {
					Mouse_Controller::Set_Button_is_Pressed(button, true);
					cout << "Mouse button: " << button << " is pressed!\n";
				}
				//If Mouse button isn't pressed this frame, set it as false because maybe at previous frame it was pressed!
				else {
					Mouse_Controller::Set_Button_is_Pressed(button, false);
				}
			}


			double xpos, ypos;
			glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
			Mouse_Controller::MOUSE_CURRENT_POSITION = vec2(xpos, ypos);

		}
		*/
		//Take this frame's key inputs!
		glfwPollEvents();
	}


	//GFX Resource Destroy Operations
	void OpenGL4_Core::Destroy_GFX_Resources() {
		CONNECTED_Monitors.clear();

		//First, destroy window and its resources with glfwDestroyWindow
		glfwDestroyWindow(((WINDOW*)Main_Window)->GLFWWINDOW);
		delete Main_Window;

		IMGUI->Destroy_IMGUI_Resources();
		GFXContentManager->Unload_AllResources();
		delete ContentManager;
		delete RENDERER;
		glfwTerminate();
	}

	void OpenGL4_Core::Check_Errors() {
		int error = glGetError();
		if (error == NULL) {
			//LOG_STATUS("There is no OpenGL error!");
		}

		//Print the error!
		if (error == GL_INVALID_OPERATION) {
			//LOG_ERROR("GL_INVALID_OPERATION!");
		}
		if (error == GL_INVALID_ENUM) {
			//LOG_ERROR("GL_INVALID_ENUM!");
		}
		if (error == GL_INVALID_VALUE) {
			//LOG_ERROR("GL_INVALID_VALUE!");
		}
	}

	void OpenGL4_Core::Swapbuffers_ofMainWindow() {
		glfwMakeContextCurrent(((WINDOW*)Main_Window)->GLFWWINDOW);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glfwSwapBuffers(((WINDOW*)Main_Window)->GLFWWINDOW);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		
	}
}