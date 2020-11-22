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
        interfaces.reserve(table->NumEntries);
        for (DWORD i = 0; i < table->NumEntries; i++) {
            auto& iface = table->Table[i];
            InterfaceInfo info = *(InterfaceInfo*)&iface;
            interfaces.push_back(info);
        }
        ::FreeMibTable(table);
    }
    return interfaces;
}
