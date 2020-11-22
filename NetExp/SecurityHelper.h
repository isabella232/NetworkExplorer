#pragma once

struct SecurityHelper abstract final {
	static bool IsRunningElevated();
	static bool RunElevated();
	static HICON GetShieldIcon();
};


