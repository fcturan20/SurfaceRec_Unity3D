#pragma once
#include "GFX/GFX_Includes.h"
#include "IMGUI_GFX.h"
#include "IMGUI_WINDOW.h"
#include "GFX/Renderer/GPU_ContentManager.h"


namespace GFX_API {
	class GFXAPI IMGUI_Core {
	public:
		IMGUI_GFX* GFX_IMGUI;
		IMGUI_WindowManager* WindowManager;
		GPU_ContentManager* GPUContentManager;
		bool Is_IMGUI_Open;
		IMGUI_Core();
		bool Check_IMGUI_Version();
		void* Create_Context(void* gpu_window_context);
		void Set_Current_Context(void* context);
		void Destroy_IMGUI_Resources();
		void Platform_Settings();
		void Set_as_MainViewport();

		bool Show_DemoWindow();
		bool Show_MetricsWindow();

		//IMGUI FUNCTIONALITY!
		void New_Frame();
		void Render_Frame();
		bool Create_Window(const char* title, bool& should_close, const bool& has_menubar = false);
		void End_Window();
		void Text(const char* text);
		bool Button(const char* button_name);
		bool Checkbox(const char* name, bool* variable);
		//This is not string, because dear ImGui needs a buffer to set a char!
		bool Input_Text(const char* name, string* Text);
		//Create a menubar for a IMGUI window!
		bool Begin_Menubar();
		void End_Menubar();
		//Create a menu button! Returns true if it is clicked!
		bool Begin_Menu(const char* name);
		void End_Menu();
		//Create a item for a menu! Shortcut argument is just use for the future support, nothing functional for now.
		bool Menu_Item(const char* name, const char* Shortcut = nullptr);
		//Write a paragraph text!
		bool Input_Paragraph_Text(const char* name, string* Text);
		//Put the next item to the same line with last created item
		//Use between after last item's end - before next item's begin!
		void Same_Line();
		bool Begin_Tree(const char* name);
		void End_Tree();
		//Create a select list that extends when clicked and get the selected_index in one-line of code!
		//Returns if any item is selected in the list! Selected item's index is the selected_index's pointer's value!
		bool SelectList_OneLine(const char* name, unsigned int* selected_index, const vector<string>* item_names);
		bool SelectList_OneLine(const char* name, unsigned int* selected_index, const vector<const char*>& item_names);
		void Selectable(const char* name, bool* is_selected);
		//Create a box of selectable items in one-line of code!
		//Returns if any item is selected in the list! Selected item's index is the selected_index's pointer's value!
		bool Selectable_ListBox(const char* name, int* selected_index, vector<string>* item_names);
		//Create a box of checkable items in one-line of code! Nice feature to edit a list
		void CheckListBox(const char* name, Bitset* items_status, vector<string>* item_names);
		//Display a texture that is in the GPU memory, for example a Render Target or a Texture
		void Display_Texture(unsigned int TEXTURE_ASSETID, const unsigned int& Display_WIDTH, const unsigned int& Display_HEIGHT, bool should_Flip_Vertically = false);
		bool Begin_TabBar();
		void End_TabBar();
		bool Begin_TabItem(const char* name);
		void End_TabItem();
		vec2 GetLastItemRectMin();
		vec2 GetLastItemRectMax();
		vec2 GetItemWindowPos();
		vec2 GetMouseWindowPos();
		void Separator();

		//Add here Unsigned Int, Unsigned Short & Short, Unsigned Char & Char sliders too!

		bool Slider_Int(const char* name, int* data, int min, int max);
		bool Slider_Float(const char* name, float* data, float min, float max);
		bool Slider_Vec2(const char* name, vec2* data, float min, float max);
		bool Slider_Vec3(const char* name, vec3* data, float min, float max);
		bool Slider_Vec4(const char* name, vec4* data, float min, float max);
	};
}