#pragma once

#include <string>

struct ProcessInfo {
	DWORD Id;
	CString Name;
	HICON hIcon;
};

class ProcessManager {
public:
	ProcessManager();
	~ProcessManager();

	void EnumProcesses();
	CString GetProcessName(DWORD id) const;
	HICON GetProcessIcon(DWORD id) const;
	CString GetFullProcessImageName(DWORD id) const;

private:
	mutable std::unordered_map<DWORD, ProcessInfo> _processes;
	mutable std::unordered_map<std::wstring, HICON> _icons;
};

