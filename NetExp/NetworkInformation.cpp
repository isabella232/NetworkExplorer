#include "pch.h"
#include "NetworkInformation.h"

std::vector<AdapterInfo> NetworkInformation::EnumAdapters() {
    DWORD size = 1 << 20;
    std::vector<AdapterInfo> adapters;

    auto buffer = ::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (buffer) {
        auto info = (PIP_ADAPTER_ADDRESSES)buffer;
        if (ERROR_SUCCESS == ::GetAdaptersAddresses(AF_UNSPEC, 0xfff, nullptr, info, &size)) {
            adapters.reserve(32);
            while (info) {
                AdapterInfo ai;
                ::memcpy(&ai, info, sizeof(*info));
                ai.Name = info->AdapterName;
                ai.FriendlyName = info->FriendlyName;
                ai.Description = info->Description;
                adapters.push_back(std::move(ai));
                info = info->Next;
            }
        }
        ::VirtualFree(buffer, 0, MEM_RELEASE);
    }
    return adapters;
}

std::vector<InterfaceInfo> NetworkInformation::EnumInterfaces() {
    std::vector<InterfaceInfo> interfaces;
    DWORD count = 0;
    ::GetNumberOfInterfaces(&count);
    if (count == 0)
        return interfaces;

    PMIB_IF_TABLE2 table;
    if (::GetIfTable2(&table) == ERROR_SUCCESS) {
        auto count = table->NumEntries;
        interfaces.reserve(count);
        for (DWORD i = 0; i < count; i++) {
            auto& iface = table->Table[i];
            InterfaceInfo info = *(InterfaceInfo*)&iface;
            interfaces.push_back(info);
        }
        ::FreeMibTable(table);
    }
    return interfaces;
}

std::vector<MIB_IPADDRROW> NetworkInformation::EnumIPAddressTable() {
    DWORD size = 1 << 17;
    std::vector<MIB_IPADDRROW> addresses;

    auto buffer = ::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (buffer) {
        auto table = (PMIB_IPADDRTABLE)buffer;
        if (ERROR_SUCCESS == ::GetIpAddrTable(table, &size, FALSE)) {
            auto count = table->dwNumEntries;
            addresses.reserve(count);
            for (DWORD i = 0; i < count; i++) {
                addresses.push_back(table->table[i]);
            }
        }
    }
    return addresses;
}

MIB_TCPSTATS NetworkInformation::GetTcpStats(NetFamily family) {
    MIB_TCPSTATS stats{};
    ::GetTcpStatisticsEx(&stats, static_cast<ULONG>(family));
    return stats;
}
