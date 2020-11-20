#pragma once

struct SecurityHelper abstract final {
	static bool IsRunningElevated();
	static bool RunElevated();
	static void ErrorDialog(HWND hParent, PCWSTR message, DWORD Error);
	static bool IsTokenAdminGroup();
};

