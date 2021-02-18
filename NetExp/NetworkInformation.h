#pragma once

struct AdapterInfo : IP_ADAPTER_ADDRESSES_LH {
	std::string Name;
	std::wstring DnsSuffix;
	std::wstring Description;
	std::wstring FriendlyName;

private:
	// prevent usage of some base members
	using IP_ADAPTER_ADDRESSES_LH::AdapterName;
	using IP_ADAPTER_ADDRESSES_LH::Next;
};

struct InterfaceInfo : MIB_IF_ROW2 {
};

static_assert(sizeof(InterfaceInfo) == sizeof(MIB_IF_ROW2));

enum class NetFamily {
	IPv4 = AF_INET,
	IPv6 = AF_INET6
};

struct NetworkInformation {
	static std::vector<AdapterInfo> EnumAdapters();
	static std::vector<InterfaceInfo> EnumInterfaces();
	static std::vector<MIB_IPADDRROW> EnumIPAddressTable();
	static std::vector<MIB_IPFORWARDROW> EnumIPForwardTable();
	static std::vector<MIB_IPINTERFACE_ROW> EnumIPInterfaces();
	static MIB_TCPSTATS GetTcpStats(NetFamily family);
};

