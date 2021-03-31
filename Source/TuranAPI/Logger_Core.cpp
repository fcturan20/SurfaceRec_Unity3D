#include "Logger_Core.h"
#include <string>
//Please don't move the above classes anywhere! I just want to use them here, so don't need to worry about wrong usage


using namespace TuranAPI::Logging;

Logger* Logger::SELF = nullptr;
Logger::Logger() : MainLogFile_Path("C:/dev/TuranEngine/logs.txt"), WarningLogFile_Path("C:/dev/TuranEngine/warnings.txt"),
ErrorLogFile_Path("C:/dev/TuranEngine/errors.txt"), NotCodedLogFile_Path("C:/dev/TuranEngine/notcodedpaths.txt")
{
#ifdef TURAN_DEBUGGING
	std::cout << "Logger constructor started!\n";
	SELF = this;
	TuranAPI::FileSystem::Write_TextFile("TuranAPI: Logging Started!", MainLogFile_Path.c_str(), false);
	TuranAPI::FileSystem::Write_TextFile("TuranAPI: Logging Started!", WarningLogFile_Path.c_str(), false);
	TuranAPI::FileSystem::Write_TextFile("TuranAPI: Logging Started!", ErrorLogFile_Path.c_str(), false);
	TuranAPI::FileSystem::Write_TextFile("TuranAPI: Logging Started!", NotCodedLogFile_Path.c_str(), false);
	std::cout << "Logger constructor finished!\n";
#else
#endif
}
Logger::~Logger() {
#ifdef TURAN_DEBUGGING
	std::cout << "Logger destructor has started!\n";
	std::cout << "Logger destructor has finished!\n";
#else
#endif
}


void Logger::Write_LOGs_toTextFiles() {
#ifdef TURAN_DEBUGGING
	if (LOGs.size() == 0) {
		std::cout << "There is no log to write!\n";
		return;
	}
	string MainLogFile_Text, ErrorLogFile_Text, WarningLogFile_Text, NotCodedLogFile_Text;
	LOG* log_data = nullptr;
	for (unsigned int i = 0; i < LOGs.size(); i++) {
		log_data = &LOGs[i];
		switch (log_data->TYPE)
		{
		case LOG_TYPE::CRASHING_ERROR:
		case LOG_TYPE::ERROR:
			MainLogFile_Text.append(log_data->TEXT);
			MainLogFile_Text.append("\n");
			ErrorLogFile_Text.append(log_data->TEXT);
			ErrorLogFile_Text.append("\n");
			break;
		case LOG_TYPE::WARNING:
			MainLogFile_Text.append(log_data->TEXT);
			MainLogFile_Text.append("\n");
			WarningLogFile_Text.append(log_data->TEXT);
			WarningLogFile_Text.append("\n");
			break;
		case LOG_TYPE::NOT_CODEDPATH:
			MainLogFile_Text.append(log_data->TEXT);
			MainLogFile_Text.append("\n");
			ErrorLogFile_Text.append(log_data->TEXT);
			ErrorLogFile_Text.append("\n");
			NotCodedLogFile_Text.append(log_data->TEXT);
			NotCodedLogFile_Text.append("\n");
			break;
		case LOG_TYPE::STATUS:
			MainLogFile_Text.append(log_data->TEXT);
			MainLogFile_Text.append("\n");
			break;
		default:
			break;
		}
	}
	TuranAPI::FileSystem::Write_TextFile(&MainLogFile_Text, &SELF->MainLogFile_Path, true);
	TuranAPI::FileSystem::Write_TextFile(&ErrorLogFile_Text, &SELF->ErrorLogFile_Path, true);
	TuranAPI::FileSystem::Write_TextFile(&WarningLogFile_Text, &SELF->WarningLogFile_Path, true);
	TuranAPI::FileSystem::Write_TextFile(&NotCodedLogFile_Text, &SELF->NotCodedLogFile_Path, true);

	LOGs.clear();
#else
#endif
}

void Logger::Log_CrashingError(const char* log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];


	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::CRASHING_ERROR;
	Write_LOGs_toTextFiles();
	TuranAPI::Breakpoint(log);
#else
#endif
}

void Logger::Log_HandledError(const char* log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::ERROR;


	std::cout << "Error: " << log_data->TEXT << std::endl;
#else
#endif
}

void Logger::Log_Warning(const char* log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::WARNING;

	std::cout << "Warning: " << log_data->TEXT << std::endl;
#else
#endif
}

void Logger::Log_Status(const char* log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::STATUS;
	std::cout << "Status: " << log_data->TEXT << std::endl;
#else
#endif
}

void Logger::Log_NotCodedPath(const char* log, bool stop_running) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::NOT_CODEDPATH;

	std::cout << "Not Coded Path: " << log_data->TEXT << std::endl;

	if (stop_running) {
		Write_LOGs_toTextFiles();
		TuranAPI::Breakpoint();
	}
#else
#endif
}



//String
void Logger::Log_CrashingError(const string& log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];


	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::CRASHING_ERROR;
	Write_LOGs_toTextFiles();
	TuranAPI::Breakpoint(log.c_str());
#else
#endif
}
void Logger::Log_HandledError(const string& log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::ERROR;


	std::cout << "Error: " << log_data->TEXT << std::endl;
#else
#endif
}
void Logger::Log_Warning(const string& log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::WARNING;

	std::cout << "Warning: " << log_data->TEXT << std::endl;
#else
#endif
}
void Logger::Log_Status(const string& log) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::STATUS;
	std::cout << "Status: " << log_data->TEXT << std::endl;
#else
#endif
}
void Logger::Log_NotCodedPath(const string& log, bool stop_running) {
#ifdef TURAN_DEBUGGING
	LOG* log_data = nullptr;
	LOGs.push_back(LOG());
	log_data = &LOGs[LOGs.size() - 1];
	log_data->TEXT = log;
	log_data->TYPE = LOG_TYPE::NOT_CODEDPATH;

	std::cout << "Not Coded Path: " << log_data->TEXT << std::endl;

	if (stop_running) {
		Write_LOGs_toTextFiles();
		TuranAPI::Breakpoint();
	}
#else
#endif
}
