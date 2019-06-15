// 2018 Mitchell Andrews
#pragma once
#include <iostream>
#include <fstream>
#include "Singleton.h"
#include <vector>
/*
Logger.h
A utility class for creating and managing logs for the engine. You can change the
log file name and priority levels to control what info gets saved and where.
*/

/// Looks like things fucked up
#define YIKES(name) Logger::Log(Logger::LogType::Error, name)

/// A Warning
#define BRUH(name) Logger::Log(Logger::LogType::Warning, name)

class Logger
{
public:
	~Logger();
	/*
	The log priority levels.
	Setting a priority will only allow the logger to output
	the set level and higher.
	*/
	enum class LogType : int
	{
		None = 0,
		Info,
		Trace,
		Debug,
		Warning,
		Error
	};

	enum class LogFilter : int
	{
		None = 0,
		Info = 0 << 0,
		Trace = 0 << 1,
		Debug = 0 << 2,
		Warning = 0 << 3,
		Error = 0 << 4
	};

	//************************************
	// Method:    SetLogFile
	// FullName:  Logger::SetLogFile
	// Access:    public
	// Returns:   void
	// Qualifier: Set the log file location to write to.
	// Parameter: std::string filename
	//************************************
	void SetLogFile(std::string filename);
	/*
	Set the logger write priority.
	@param priority: The logger priority to change to
	*/
	void SetLogPriority(Logger::LogType priority);
	/*
	Write a message to the file currently set with the given priority.
	@param priority: The logger priority to change to
	@param message: The message to write to the file
	@returns true if the log was written to the file else false.
	*/
	bool LogMessage(Logger::LogType priority, std::string message);
	static bool Log(Logger::LogType priority, const std::string& message);
	struct LogEntry
	{
		LogType Type = LogType::None;
		std::string Message;
	};

	static std::vector<LogEntry> Messages;

private:

	std::ofstream mLogFile;
	std::string mLogFileLocation;
	LogType mPriority;
	Logger() = default;

	ME_SINGLETON_DEFINITION(Logger)
};
