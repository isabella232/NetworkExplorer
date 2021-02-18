#pragma once

struct ToStringHelper final abstract {
	static PCWSTR NodeTypeToString(UINT type);
	static PCWSTR InterfaceTypeToString(IFTYPE type);
	static CString AddressTypeToString(WORD type);
	static PCWSTR InterfaceConnectionTypeToString(NET_IF_CONNECTION_TYPE type);
};
