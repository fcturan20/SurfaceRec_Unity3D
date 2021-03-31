#include "IMGUI_Core.h"
#include "Includes/imgui.h"
#include "Includes/imgui_stdlib.h"

namespace GFX_API {
	IMGUI_Core::IMGUI_Core() {
		std::cout << "IMGUI_Core's constructor has finished!\n";
		WindowManager = new IMGUI_WindowManager;
		Is_IMGUI_Open = true;
	}
	bool IMGUI_Core::Check_IMGUI_Version() {
		//Check version here, I don't care here for now!
		return IMGUI_CHECKVERSION();
	}

	void* IMGUI_Core::Create_Context(void* gpu_window_context) {
		//Create Context here!
		IMGUI_CHECKVERSION();
		void* Context = ImGui::CreateContext();
		if (Context == nullptr) {
			std::cout << "Error: Context is nullptr after creation!\n";
		}

		//Set Input Handling settings here! 
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;


		//Set color style to dark by default for now!
		ImGui::StyleColorsDark();

		
		//Set context's GFX_API settings!
		if (GFX_IMGUI == nullptr) {
			TuranAPI::Breakpoint("GFX_IMGUI isn't initialized, initializing failed for IMGUI!");
			return nullptr;
		}
		GFX_IMGUI->Initialize(gpu_window_context);

		return Context;
	}

	void IMGUI_Core::Set_as_MainViewport() {
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
		ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
	}

	void IMGUI_Core::Set_Current_Context(void* context) {
		ImGui::SetCurrentContext((ImGuiContext*)context);
	}

	void IMGUI_Core::Destroy_IMGUI_Resources() {
		std::cout << "IMGUI resources are being destroyed!\n";
		GFX_IMGUI->Destroy_IMGUI_GFX_Resources();
		ImGui::DestroyContext();
		Is_IMGUI_Open = false;
	}

	bool IMGUI_Core::Show_DemoWindow() {
		bool x = true;
		ImGui::ShowDemoWindow(&x);
		return x;
	}

	bool IMGUI_Core::Show_MetricsWindow() {
		bool x = true;
		ImGui::ShowMetricsWindow(&x);
		return x;
	}

	//IMGUI FUNCTIONALITY!

	void IMGUI_Core::New_Frame() {
		GFX_IMGUI->GFX_New_Frame();
		ImGui::NewFrame();
	}

	void IMGUI_Core::Render_Frame() {
		ImGui::Render();
		GFX_IMGUI->Render_IMGUI(ImGui::GetDrawData());
		Platform_Settings();
	}

	void IMGUI_Core::Platform_Settings() {
		GFX_IMGUI->Set_Platform_Settings();
	}

	bool IMGUI_Core::Create_Window(const char* title, bool& should_close, const bool& has_menubar) {
		ImGuiWindowFlags window_flags = 0;
		window_flags |= (has_menubar ? ImGuiWindowFlags_MenuBar : 0);
		return ImGui::Begin(title, &should_close, window_flags);
	}

	void IMGUI_Core::End_Window() {
		ImGui::End();
	}

	void IMGUI_Core::Text(const char* text) {
		ImGui::Text(text);
	}

	bool IMGUI_Core::Button(const char* button_name) {
		return ImGui::Button(button_name);
	}

	bool IMGUI_Core::Checkbox(const char* name, bool* variable) {
		return ImGui::Checkbox(name, variable);
	}

	bool IMGUI_Core::Input_Text(const char* name, string* text) {
		if (ImGui::InputText(name, text, ImGuiInputTextFlags_EnterReturnsTrue)) {
			return true;
		}
		return false;
	}

	bool IMGUI_Core::Begin_Menubar() {
		return ImGui::BeginMenuBar();
	}

	void IMGUI_Core::End_Menubar() {
		ImGui::EndMenuBar();
	}

	bool IMGUI_Core::Begin_Menu(const char* name) {
		return ImGui::BeginMenu(name);
	}

	void IMGUI_Core::End_Menu() {
		ImGui::EndMenu();
	}

	bool IMGUI_Core::Menu_Item(const char* name, const char* shortcut) {
		return ImGui::MenuItem(name, shortcut);
	}

	bool IMGUI_Core::Input_Paragraph_Text(const char* name, string* Text) {
		if (ImGui::InputTextMultiline(name, Text, ImVec2(0,0), ImGuiInputTextFlags_EnterReturnsTrue)) {
			return true;
		}
		return false;
	}

	//Puts the next item to the same line with last created item
	//Use between after last item's end - before next item's begin!
	void IMGUI_Core::Same_Line() {
		ImGui::SameLine();
	}

	bool IMGUI_Core::Begin_Tree(const char* name) {
		return ImGui::TreeNode(name);
	}

	void IMGUI_Core::End_Tree() {
		ImGui::TreePop();
	}

	bool IMGUI_Core::SelectList_OneLine(const char* name, unsigned int* selected_index, const vector<string>* item_names) {
		bool is_new_item_selected = false;
		const string& preview_str = (*item_names)[*selected_index];
		if (ImGui::BeginCombo(name, preview_str.c_str()))	// The second parameter is the index of the label previewed before opening the combo.
		{
			for (size_t n = 0; n < item_names->size(); n++)
			{
				string item_name = (*item_names)[n];
				bool is_selected = (*selected_index == n);
				if (ImGui::Selectable((*item_names)[n].c_str(), is_selected)) {
					*selected_index = n;
					is_new_item_selected = true;
				}
			}
			ImGui::EndCombo();
		}
		return is_new_item_selected;
	}
	bool IMGUI_Core::SelectList_OneLine(const char* name, unsigned int* selected_index, const vector<const char*>& item_names) {
		bool is_new_item_selected = false;
		const char* preview_str = item_names[*selected_index];
		if (ImGui::BeginCombo(name, preview_str))	// The second parameter is the index of the label previewed before opening the combo.
		{
			for (size_t n = 0; n < item_names.size(); n++)
			{
				string item_name = item_names[n];
				bool is_selected = (*selected_index == n);
				if (ImGui::Selectable(item_names[n], is_selected)) {
					*selected_index = n;
					is_new_item_selected = true;
				}
			}
			ImGui::EndCombo();
		}
		return is_new_item_selected;
	}

	//If selected, argument "is_selected" is set to its opposite!
	void IMGUI_Core::Selectable(const char* name, bool* is_selected) {
		ImGui::Selectable(name, is_selected);
	}

	bool IMGUI_Core::Selectable_ListBox(const char* name, int* selected_index, vector<string>* item_names) {
		int already_selected_index = *selected_index;
		bool is_new_selected = false;
		if (ImGui::ListBoxHeader(name)) {
			for (unsigned int i = 0; i < item_names->size(); i++) {
				bool is_selected = false;
				string item_name = (*item_names)[i];
				Selectable(item_name.c_str(), &is_selected);
				if (is_selected && (already_selected_index != i)) {
					*selected_index = i;
					is_new_selected = true;
				}
			}

			ImGui::ListBoxFooter();
		}
		return is_new_selected;
	}

	void IMGUI_Core::CheckListBox(const char* name, Bitset* items_status, vector<string>* item_names) {
		if (ImGui::ListBoxHeader(name)) {
			for (unsigned int i = 0; i < item_names->size(); i++) {
				bool x = items_status->GetBit_Value(i);
				Checkbox((*item_names)[i].c_str(), &x);
				x ? items_status->SetBit_True(i) : items_status->SetBit_False(i);
				std::cout << "Current Index: " << i << std::endl;
				std::cout << "Current Name: " << (*item_names)[i] << std::endl;
				std::cout << "Current Value: " << x << std::endl;
			}
			ImGui::ListBoxFooter();
		}
	}

	void IMGUI_Core::Display_Texture(unsigned int TEXTURE_AssetID, const unsigned int& Display_WIDTH, const unsigned int& Display_HEIGHT, bool should_Flip_Vertically) {
		unsigned int data = *(unsigned int*)GPUContentManager->Find_GFXTexture_byID(TEXTURE_AssetID)->GL_ID;
		if (should_Flip_Vertically) {
			ImGui::Image((void*)(intptr_t)data, ImVec2(Display_WIDTH, Display_HEIGHT), ImVec2(0, 1), ImVec2(1, 0));
		}
		else {
			ImGui::Image((void*)(intptr_t)data, ImVec2(Display_WIDTH, Display_HEIGHT));
		}
	}
	bool IMGUI_Core::Begin_TabBar() {
		return ImGui::BeginTabBar("");
	}
	void IMGUI_Core::End_TabBar() {
		ImGui::EndTabBar();
	}
	bool IMGUI_Core::Begin_TabItem(const char* name) {
		return ImGui::BeginTabItem(name);
	}
	void IMGUI_Core::End_TabItem() {
		ImGui::EndTabItem();
	}
	void IMGUI_Core::Separator() {
		ImGui::Separator();
	}
	vec2 IMGUI_Core::GetLastItemRectMin() {
		return vec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
	}
	vec2 IMGUI_Core::GetLastItemRectMax() {
		return vec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y);
	}
	vec2 IMGUI_Core::GetItemWindowPos() {
		return vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
	}
	vec2 IMGUI_Core::GetMouseWindowPos() {
		return vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	}







	bool IMGUI_Core::Slider_Int(const char* name, int* data, int min, int max) { return ImGui::SliderInt(name, data, min, max); }
	bool IMGUI_Core::Slider_Float(const char* name, float* data, float min, float max) { return ImGui::SliderFloat(name, data, min, max); }
	bool IMGUI_Core::Slider_Vec2(const char* name, vec2* data, float min, float max) { return ImGui::SliderFloat2(name, (float*)data, min, max); }
	bool IMGUI_Core::Slider_Vec3(const char* name, vec3* data, float min, float max) { return ImGui::SliderFloat3(name, (float*)data, min, max); }
	bool IMGUI_Core::Slider_Vec4(const char* name, vec4* data, float min, float max) { return ImGui::SliderFloat4(name, (float*)data, min, max); }
}