#pragma once

struct FormatHelper {
	static CString Format(int value, bool hex = false);
	static CString Format(DWORD value, bool hex = false);
	static CString GuidToString(const GUID& guid);
	static CString IPv4AddressToString(DWORD addr);
};

