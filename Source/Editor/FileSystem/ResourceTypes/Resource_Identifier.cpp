#include "Resource_Identifier.h"

namespace TuranEditor {
	string Resource_Identifier::Get_Name() {
		std::string NAME = PATH;
		NAME = NAME.substr(NAME.find_last_of('/') + 1);
		NAME = NAME.substr(0, NAME.find_last_of('.'));
		return NAME;
	}
}