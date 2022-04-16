#include "API_includes.h"

namespace TuranAPI {
	void Breakpoint(const char* Breakpoint_Reason) {
		if (Breakpoint_Reason != nullptr) {
			std::cout << "Crashing Reason: " << Breakpoint_Reason << std::endl;
		}
		char input;
		std::cout << "Application hit a breakpoint, would you like to continue? (Note: Be careful, application may crash!) \nTo continue: enter y, to close: enter n \nInput: ";
		std::cin >> input;
		if (input == 'y') {
			return;
		}
		else if (input == 'n') {
			exit(EXIT_FAILURE);
		}
		else {
			std::cout << "Wrong input:\n";
			TuranAPI::Breakpoint();
		}
	}

	//Delete multiple elements of a vector by creating a bool vector at the size of the base vector
	template<typename T>
	unsigned int Delete_Items_from_vector(vector<T>* vector_to_EraseSomething, vector<bool>* ShouldErase_forEachIndex, unsigned int start_index) {
		if (vector_to_EraseSomething->size() != ShouldErase_forEachIndex->size()) {
			TuranAPI::Breakpoint("In Delete Items from vector(), 2 vectors should have same size!");
		}
		unsigned int i = start_index;
		for (i = start_index; i < vector_to_EraseSomething->size(); i++) {
			bool should_erase = (*ShouldErase_forEachIndex)[i];
			if (should_erase) {
				vector_to_EraseSomething->erase(i);
				ShouldErase_forEachIndex->erase(i);
				i = Delete_Items_from_vector<T>(vector_to_EraseSomething, ShouldErase_forEachIndex, i);
			}
		}
		return i;
	}

	void Empty() {

	}
	/*
	string* TuranAPI::Find_TuranAPIEnumsName_byValue(unsigned short Enums_Value) {
		return &TuranAPI_EnumsNames[Enums_Value];
	}

	TuranAPI_ENUMs TuranAPI::Find_TuranAPIEnum_byName(const string& Enum_Name) {
		for (unsigned short i = 0; i < TuranAPI_ENUMsNames.size(); i++) {
			if (Enum_Name == *TuranAPI::Find_TuranAPIEnumsName_byValue(i)) {
				return TuranAPI_ENUMs(i);
			}
		}
	}
	*/
}