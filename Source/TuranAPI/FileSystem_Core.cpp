#include "FileSystem_Core.h"
//Data Formats created by Flatbuffers!
#include <flatbuffers/flatbuffers.h>

#include <fstream>
#include <sstream>

namespace TuranAPI {
	string* FileSystem::Read_TextFile(const char* path) {
		string* finalstring = new string;
		std::ifstream textfile;
		textfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			textfile.open(path);
			std::stringstream string_data;
			string_data << textfile.rdbuf();
			textfile.close();
			*finalstring = string_data.str().c_str();
			return finalstring;
		}
		catch (std::ifstream::failure error)
		{
			std::cout << "Error: Text file couldn't read: " << path << std::endl;
			return finalstring;
		}
	}

	void FileSystem::Write_TextFile(const char* text, const char* path, bool write_to_end) {
		std::ofstream Output_File;
		if (write_to_end) {
			std::cout << "Before ofstream!\n";

			Output_File.open(path, std::ios::out | std::ios::app);

			Output_File << text << std::endl;
			Output_File.close();

			std::cout << "After ofstream!\n";
		}
		else {
			std::cout << "Before open!\n";

			Output_File.open(path, std::ios::out | std::ios::trunc);
			std::cout << "After open!\n";
			Output_File << text << std::endl;
			std::cout << "After appending!\n";
			Output_File.close();
			std::cout << "After closing!\n";
		}
	}

	void FileSystem::Write_TextFile(const string* text, const string* path, bool write_to_end) {
		std::ofstream Output_File;
		if (write_to_end) {

			Output_File.open(*path, std::ios::out | std::ios::app);

			Output_File << *text << std::endl;
			Output_File.close();
		}
		else {

			Output_File.open(*path, std::ios::out | std::ios::trunc);

			Output_File << *text << std::endl;
			Output_File.close();
		}
	}

	//If read fails, data_ptr is set to nullptr!
	void* FileSystem::Read_BinaryFile(const char* path, unsigned int& size) {
		std::ifstream Binary_File;
		Binary_File.open(path, std::ios::binary | std::ios::in | std::ios::ate);
		if (!(Binary_File.is_open())) {
			std::cout << "There is no such file: " << path << std::endl;
			return nullptr;
		}

		Binary_File.seekg(0, std::ios::end);
		int length = Binary_File.tellg();
		Binary_File.seekg(0, std::ios::beg);
		char* read_data = new char[length];
		Binary_File.read(read_data, length);
		Binary_File.close();
		size = length;
		return read_data;
	}

	//Use only for creating new binary files! 
	//If you want to overwrite to a file, Overwrite_BinaryFile is available!
	void FileSystem::Write_BinaryFile(const char* path, void* data, unsigned int size) {
		//Try to create to a file (Check if the operation fails)
		std::ofstream Output_File(path, std::ios::binary | std::ios::out);
		if (!Output_File.is_open()) {
			//TuranAPI::Breakpoint(Text_Add(path, " couldn't be outputted!\n"));
			return;
		}

		//Write to a file and finish all of the operation!
		Output_File.write((const char*)data, size);
		Output_File.close();
		std::cout << path << " is outputted successfully!\n";
	}

	//This function will delete the previous content of the file and fill with new data!
	void FileSystem::Overwrite_BinaryFile(const char* path, void* data, unsigned int size) {
		//ios::trunc is used to clear the file before outputting the data!
		std::ofstream Output_File(path, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!Output_File.is_open()) {
			std::cout << "Error: " << path << " couldn't be outputted!\n";
			return;
		}

		if (data == nullptr) {
			std::cout << "data is nullptr!\n";
		}
		if (size == 0) {
			std::cout << "data size is 0!\n";
		}
		//Write to a file and finish all of the operation!
		Output_File.write((const char*)data, size);
		Output_File.close();
		std::cout << path << " is outputted successfully!\n";

	}

	void FileSystem::Delete_File(const char* path) {
		std::remove(path);
	}
}