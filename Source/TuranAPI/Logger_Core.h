#pragma once
#include "TuranAPI/API_includes.h"
#include "FileSystem_Core.h"

namespace TuranAPI {
	namespace Logging {
		enum class TURANAPI LOG_TYPE : char {
			CRASHING_ERROR = 0, ERROR = 1, WARNING = 2, STATUS = 3, NOT_CODEDPATH = 4
		};

		struct TURANAPI LOG {
		public:
			LOG_TYPE TYPE;
			string TEXT;
		};

		class TURANAPI Logger {
			string MainLogFile_Path;
			string WarningLogFile_Path;
			string ErrorLogFile_Path;
			string NotCodedLogFile_Path;
			vector<LOG> LOGs;
		public:
			static Logger* SELF;
			Logger();
			~Logger();
			void Write_LOGs_toTextFiles();
			void Log_CrashingError(const char* log);
			void Log_HandledError(const char* log);
			void Log_Warning(const char* log);
			void Log_Status(const char* log);
			void Log_NotCodedPath(const char* log, bool stop_running);

			//String
			void Log_CrashingError(const string& log);
			void Log_HandledError(const string& log);
			void Log_Warning(const string& log);
			void Log_Status(const string& log);
			void Log_NotCodedPath(const string& log, bool stop_running);
		};

	}
}
#define LOG_CRASHING(LOG_string)							TuranAPI::Logging::Logger::SELF->Log_CrashingError(LOG_string)
#define LOG_ERROR(LOG_string)								TuranAPI::Logging::Logger::SELF->Log_HandledError(LOG_string)
#define LOG_WARNING(LOG_string)								TuranAPI::Logging::Logger::SELF->Log_Warning(LOG_string)
#define LOG_STATUS(LOG_string)								TuranAPI::Logging::Logger::SELF->Log_Status(LOG_string)
#define LOG_NOTCODED(LOG_string, Stop_Application_BOOL)		TuranAPI::Logging::Logger::SELF->Log_NotCodedPath(LOG_string, Stop_Application_BOOL)
#define WRITE_LOGs_toFILEs()								TuranAPI::Logging::Logger::SELF->Write_LOGs_toTextFiles()