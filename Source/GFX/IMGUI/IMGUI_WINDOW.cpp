#include "IMGUI_WINDOW.h"

namespace GFX_API {
	IMGUI_WINDOW::IMGUI_WINDOW(const char* name) {
		std::cout << "Window name isn't set!\n";
		Window_Name = name;
		std::cout << "Window name is set!\n";
	}

	IMGUI_WINDOW::~IMGUI_WINDOW() {
		std::cout << "Editor Window is closed: " << Window_Name << std::endl;
	}




	bool IMGUI_WINDOW::Get_Is_Window_Open() {
		return Is_Window_Open;
	}

	IMGUI_WindowManager::IMGUI_WindowManager() {
		std::cout << "IMGUI WindowManager's constructor has finished!\n";
	}
	IMGUI_WindowManager::~IMGUI_WindowManager() {
		std::cout << "IMGUI WINDOW MANAGER is closing!\n";
		ALL_IMGUI_WINDOWs.clear();
	}
	void IMGUI_WindowManager::Run_IMGUI_WINDOWs() {
		for (unsigned int i = 0; i < ALL_IMGUI_WINDOWs.size(); i++) {
			IMGUI_WINDOW* WINDOW = ALL_IMGUI_WINDOWs[i];
			std::cout << "Running the Window: " << WINDOW->Window_Name << std::endl;
			WINDOW->Run_Window();
		}

		if (Windows_toClose.size() > 0) {
			for (unsigned int window_delete_i = 0; window_delete_i < Windows_toClose.size(); window_delete_i++) {
				IMGUI_WINDOW* window_to_close = Windows_toClose[window_delete_i];
				for (unsigned int i = 0; i < ALL_IMGUI_WINDOWs.size(); i++) {
					if (window_to_close == ALL_IMGUI_WINDOWs[i]) {
						delete window_to_close;
						window_to_close = nullptr;
						ALL_IMGUI_WINDOWs[i] = nullptr;
						ALL_IMGUI_WINDOWs.erase(ALL_IMGUI_WINDOWs.begin() + i);

						//Inner loop will break!
						break;
					}
				}
			}
			
			Windows_toClose.clear();
		}
		
		if (Windows_toOpen.size() > 0) {
			unsigned int previous_size = ALL_IMGUI_WINDOWs.size();

			for (unsigned int i = 0; i < Windows_toOpen.size(); i++) {
				IMGUI_WINDOW* window_to_open = Windows_toOpen[i];
				ALL_IMGUI_WINDOWs.push_back(window_to_open);
			}
			Windows_toOpen.clear();
			//We should run new windows!
			for (; previous_size < ALL_IMGUI_WINDOWs.size(); previous_size++) {
				ALL_IMGUI_WINDOWs[previous_size]->Run_Window();
			}
		}
	}
	void IMGUI_WindowManager::Register_WINDOW(IMGUI_WINDOW* WINDOW) {
		std::cout << "Registering a Window!\n";
		Windows_toOpen.push_back(WINDOW);
	}
	void IMGUI_WindowManager::Delete_WINDOW(IMGUI_WINDOW* WINDOW) {
		Windows_toClose.push_back(WINDOW);
	}
}