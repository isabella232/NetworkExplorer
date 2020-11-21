#pragma once

struct SecurityHelper abstract final {
	static bool IsRunningElevated();
	static bool RunElevated();
	static bool IsTokenAdminGroup();
	static HICON GetShieldIcon();
};


