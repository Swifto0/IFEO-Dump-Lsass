#pragma once
#include <Windows.h>
#include <string>

#define IFEO_REG_KEY "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\"
#define SILENT_PROCESS_EXIT_REG_KEY "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SilentProcessExit\\"
#define LOCAL_DUMP 0x2
#define FLG_MONITOR_SILENT_PROCESS_EXIT 0x200
#define DUMP_FOLDER "C:\\temp"
#define MiniDumpWithFullMemory 0x2

class RegSet {

private:
	BOOL bIsValid;
	HKEY hIFEORegKey;
	HKEY hSPERegKey;

public:
	RegSet(std::string processName);
	~RegSet();
	BOOL isValid();
};