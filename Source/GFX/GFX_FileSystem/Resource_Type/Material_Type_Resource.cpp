#include "Material_Type_Resource.h"


namespace GFX_API {

	//MATERIAL UNIFORM FUNCTIONs

	Material_Uniform::Material_Uniform(const char* variable_name, DATA_TYPE variable_type) : VARIABLE_NAME(variable_name), VARIABLE_TYPE(variable_type) {}
	Material_Uniform::Material_Uniform() {}
	//Please keep in mind that: This doesn't verifies if data matches with variable type!
	bool Material_Uniform::Verify_UniformData() {
		if (DATA != nullptr && VARIABLE_NAME != "") {
			return true;
		}
		return false;
	}




	//SHADER SOURCE FUNCTIONs

	ShaderSource_Resource::ShaderSource_Resource() {

	}

	//MATERIAL TYPE FUNCTIONs

	Material_Type::Material_Type() {

	}









	//MATERIAL INSTANCE FUNCTIONs

	Material_Instance::Material_Instance() {

	}

	void Material_Instance::Set_Uniform_Data(const char* uniform_name, void* pointer_to_data) {
		Material_Uniform* uniform = &UNIFORM_LIST[Find_Uniform_byName(uniform_name)];
		if (pointer_to_data == nullptr) {
			std::cout << "Error: Couldn't set GPU uniform data, because data is nullptr!\n";
			TuranAPI::Breakpoint();
			return;
		}
		if (uniform == nullptr) {
			std::cout << "Error: Found uniform is nullptr!\n";
			TuranAPI::Breakpoint();
			return;
		}
		uniform->DATA = pointer_to_data;
	}

	unsigned int Material_Instance::Find_Uniform_byName(const char* uniform_name) {
		for (unsigned int i = 0; i < UNIFORM_LIST.size(); i++) {
			if (UNIFORM_LIST[i].VARIABLE_NAME == uniform_name)
				return i;
		}
		LOG_ERROR("Intended uniform variable: " + string(uniform_name) + " can't be found in Material Type ID: " + to_string(Material_Type));
		return -1;
	}

}