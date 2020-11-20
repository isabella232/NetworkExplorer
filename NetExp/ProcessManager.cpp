#include "pch.h"
#include "ProcessManager.h"
#include <TlHelp32.h>

ProcessManager::ProcessManager() {
    EnumProcesses();
}

ProcessManager::~ProcessManager() {
    for (auto& [_, h] : _icons)
        ::DestroyIcon(h);
}

void ProcessManager::EnumProcesses() {
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return;

    if (_processes.empty()) {
        _processes.reserve(512);
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if (::Process32First(hSnapshot, &pe)) {
        do {
            ProcessInfo pi;
            pi.hIcon = nullptr;
            pi.Id = pe.th32ProcessID;
            pi.Name = pe.szExeFile;
            _processes.insert({ pi.Id, pi });
        } while (::Process32Next(hSnapshot, &pe));
    }

    ::CloseHandle(hSnapshot);
}

CString ProcessManager::GetProcessName(DWORD pid) const {
    auto result = GetFullProcessImageName(pid);
    if (result.IsEmpty()) {
        if (auto it = _processes.find(pid); it != _processes.end())
            return it->second.Name;
    }
    else {
        result = result.Mid(result.ReverseFind(L'\\') + 1);
        _processes.erase(pid);
    }
    return result;
}

HICON ProcessManager::GetProcessIcon(DWORD id) const {
    auto path = GetFullProcessImageName(id);
    if (path.IsEmpty())
        return nullptr;

    HICON hIcon{ nullptr };
    if (auto it = _icons.find((PCWSTR)path); it == _icons.end()) {
        ::ExtractIconEx(path, 0, nullptr, &hIcon, 1);
        if (hIcon)
            _icons.insert({ std::wstring(path), hIcon });
    }
    else {
        hIcon = it->second;
    }
    return hIcon;
}

CString ProcessManager::GetFullProcessImageName(DWORD pid) const {
    HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess) {
        WCHAR path[MAX_PATH];
        DWORD size = _countof(path);
        auto count = ::QueryFullProcessImageName(hProcess, 0, path, &size);
        ::CloseHandle(hProcess);
        if(count)
            return path;
    }
    return CString();
}
