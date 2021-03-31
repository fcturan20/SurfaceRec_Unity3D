#pragma once
#include "Editor/Editor_Includes.h"


namespace TuranEditor {

	enum class RESOURCETYPEs : char;

	struct Resource_Identifier {
		unsigned int ID;
		string PATH;
		enum class RESOURCETYPEs TYPE;
		void* DATA = nullptr;
		string Get_Name();
	};
}