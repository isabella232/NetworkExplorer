#include "pch.h"
#include "NetworkInfoView.h"
#include "FormatHelper.h"
#include "SortHelper.h"
#include "ToStringHelper.h"
#include <VersionHelpers.h>

CString CNetworkInfoView::GetColumnText(HWND, int row, int col) {
	if (m_IsGenericNode) {
		auto& item = m_Items[row];
		switch (col) {
			case 0: return item.Property;
			case 1: return item.ValueFn ? item.ValueFn() : item.Value;
		}
	}
	else if (m_SelectedNode == m_IPTableNode) {
		auto& item = m_IPTable[row];
		CString text;
		switch (col) {
			case 0: text.Format(L"%3d", item.dwIndex); return text;
			case 1: return FormatHelper::IPv4AddressToString(item.dwAddr);
			case 2: return FormatHelper::IPv4AddressToString(item.dwMask);
			case 3: return FormatHelper::IPv4AddressToString(item.dwBCastAddr);
			case 4: return FormatHelper::Format(item.dwReasmSize);
			case 5: return ToStringHelper::AddressTypeToString(item.wType);
		}
	}
	else if (m_SelectedNode == m_AdaptersNode) {
		auto& item = m_Adapters[row];
		switch (col) {
			case 0: return item.FriendlyName.c_str();
			case 1: return FormatHelper::Format(item.IfIndex);
			case 2: return ToStringHelper::InterfaceTypeToString(item.IfType);
			case 3: return item.Description.c_str();
		}
	}

	return CString();
}

bool CNetworkInfoView::IsSortable(int col) const {
	if (m_IsGenericNode)
		return col == 0;

	return true;
}

void CNetworkInfoView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	if (m_IsGenericNode) {
		std::sort(m_Items.begin(), m_Items.end(), [si](const auto& i1, const auto& i2) {
			return SortHelper::SortStrings(i1.Property, i2.Property, si->SortAscending);
			});
	}
	else if (m_SelectedNode == m_AdaptersNode) {
		std::sort(m_Adapters.begin(), m_Adapters.end(), [si](const auto& a1, const auto& a2) {
			switch (si->SortColumn) {
				case 0: return SortHelper::SortStrings(a1.FriendlyName, a2.FriendlyName, si->SortAscending);
				case 1: return SortHelper::SortNumbers(a1.IfIndex, a2.IfIndex, si->SortAscending);
				case 2: return SortHelper::SortStrings(ToStringHelper::InterfaceTypeToString(a1.IfType), ToStringHelper::InterfaceTypeToString(a2.IfType), si->SortAscending);
				case 3: return SortHelper::SortStrings(a1.Description, a2.Description, si->SortAscending);
			}
			return false;
			});
	}
}

void CNetworkInfoView::DoRefresh() {
	if (m_SelectedNode == m_TcpStatsNode) {
		m_TcpStats = NetworkInformation::GetTcpStats(NetFamily::IPv4);
	}
}

PCWSTR CNetworkInfoView::GetHeader() const {
	return L"Information";
}

void CNetworkInfoView::OnFinalMessage(HWND) {
	delete this;
}

void CNetworkInfoView::SetAdaptersNode() {
	m_IsGenericNode = false;
	auto cm = GetColumnManager(m_List);
	cm->Clear();
	cm->AddColumn(L"Friendly Name", LVCFMT_LEFT, 280);
	cm->AddColumn(L"If Index", LVCFMT_RIGHT, 60);
	cm->AddColumn(L"If Type", LVCFMT_LEFT, 120);
	cm->AddColumn(L"Description", LVCFMT_LEFT, 350);
	m_List.SetItemCount((int)m_Adapters.size());
}

void CNetworkInfoView::SetAdapterNodeItems(const AdapterInfo& adapter) {
	m_Items.clear();
	SetListViewPropertyColumns();
	m_IsGenericNode = true;

	m_Items = {
		GenericItem(AdapterFriendlyName, L"Friendly Name", CString(adapter.FriendlyName.c_str())),
		GenericItem(AdapterIfIndex, L"Interface Index", FormatHelper::Format(adapter.IfIndex)),
		GenericItem(AdapterName, L"Name", CString(adapter.Name.c_str())),
		GenericItem(AdapterDesc, L"Description", CString(adapter.Description.c_str())),
		GenericItem(AdapterIfType, L"Interface Type", ToStringHelper::InterfaceTypeToString(adapter.IfType)),
		GenericItem(AdapterConnectionType, L"Connection Type", ToStringHelper::InterfaceConnectionTypeToString(adapter.ConnectionType)),
	};
	m_List.SetItemCount((int)m_Items.size());
}

void CNetworkInfoView::SetInterfaceNodeItems(const InterfaceInfo& iface) {
	m_Items.clear();
	SetListViewPropertyColumns();
	m_IsGenericNode = true;

	m_Items = {
		GenericItem(IfDesc, L"Description", CString(iface.Description)),
		GenericItem(IfIndex, L"Index", FormatHelper::Format(iface.InterfaceIndex)),
		GenericItem(IfAlias, L"Alias", CString(iface.Alias)),
		GenericItem(IfType, L"Type", ToStringHelper::InterfaceTypeToString(iface.Type)),
		GenericItem(IfGuid, L"GUID", FormatHelper::GuidToString(iface.InterfaceGuid)),
	};
	m_List.SetItemCount((int)m_Items.size());
}

void CNetworkInfoView::SetGeneralNodeItems() {
	m_IsGenericNode = true;
	ULONG size = sizeof(m_FixedInfoBuffer);
	m_FixedInfo = (PFIXED_INFO)m_FixedInfoBuffer;
	::GetNetworkParams(m_FixedInfo, &size);

	SetListViewPropertyColumns();
	m_Items.clear();
	m_Items = {
		GenericItem(GenericItemType::HostName, L"Host Name", CString(m_FixedInfo->HostName)),
		GenericItem(GenericItemType::DomainName, L"Domain Name", CString(m_FixedInfo->DomainName)),
		GenericItem(GenericItemType::NodeType, L"Node Type", ToStringHelper::NodeTypeToString(m_FixedInfo->NodeType)),
		GenericItem(GenericItemType::DnsServerList, L"DNS Server List", [this]() {
			auto& addr = m_FixedInfo->DnsServerList;
			CString result;
			while (true) {
				result += addr.IpAddress.String;
				result += CString(L" (") + addr.IpMask.String + L")";
				if (!addr.Next)
					break;
				addr = *addr.Next;
				result += L", ";
			}
			return result;
			}),
		GenericItem(GenericItemType::ScopeId, L"Scope Name", CString(m_FixedInfo->ScopeId)),
		GenericItem(GenericItemType::EnableRouting, L"Enable Routing", m_FixedInfo->EnableRouting ? L"True" : L"False"),
		GenericItem(GenericItemType::EnableProxy, L"Enable Proxy", m_FixedInfo->EnableProxy ? L"True" : L"False"),
		GenericItem(GenericItemType::EnableDns, L"Enable DNS", m_FixedInfo->EnableDns ? L"True" : L"False"),
	};
	m_List.SetItemCount((int)m_Items.size());
}

void CNetworkInfoView::SetListViewPropertyColumns() {
	auto cm = GetColumnManager(m_List);
	cm->Clear();
	cm->AddColumn(L"Property", LVCFMT_LEFT, 150);
	cm->AddColumn(L"Value", LVCFMT_LEFT, 350);
	cm->UpdateColumns();
}

void CNetworkInfoView::BuildTree() {
	m_Tree.LockWindowUpdate();
	m_Tree.DeleteAllItems();

	auto item = m_GeneralNode = m_Tree.InsertItem(L"General Information", 0, 0, TVI_ROOT, TVI_LAST);
	m_TcpStatsNode = m_Tree.InsertItem(L"TCP IPv4 Statistics", 4, 4, TVI_ROOT, TVI_LAST);
	m_TcpStatsNode6 = m_Tree.InsertItem(L"TCP IPv6 Statistics", 4, 4, TVI_ROOT, TVI_LAST);
	m_UdpStatsNode = m_Tree.InsertItem(L"UDP Statistics", 4, 4, TVI_ROOT, TVI_LAST);
	item = m_AdaptersNode = m_Tree.InsertItem(L"Adapters", 1, 1, TVI_ROOT, TVI_LAST);
	AddAdapters(item);
	item.SortChildren(FALSE);
	item = m_InterfacesNode = m_Tree.InsertItem(L"Interfaces", 2, 2, TVI_ROOT, TVI_LAST);
	AddInterfaces(item);
	item.SortChildren(FALSE);
	m_IPTableNode = m_Tree.InsertItem(L"Address Table", 3, 3, TVI_ROOT, TVI_LAST);

	m_Tree.LockWindowUpdate(FALSE);
}

void CNetworkInfoView::UpdateList() {
	ClearSort(m_List);
	if (m_SelectedNode == m_GeneralNode) {
		SetGeneralNodeItems();
	}
	else if (m_SelectedNode == m_IPTableNode) {
		SetIPTableNodeItems();
	}
	else if (m_SelectedNode == m_AdaptersNode) {
		SetAdaptersNode();
	}
	else if (m_SelectedNode.GetParent() == m_AdaptersNode) {
		SetAdapterNodeItems(m_Adapters[m_SelectedNode.GetData()]);
	}
	else if (m_SelectedNode.GetParent() == m_InterfacesNode) {
		SetInterfaceNodeItems(m_Interfaces[m_SelectedNode.GetData()]);
	}
	else if (m_SelectedNode == m_TcpStatsNode) {
		SetTcpStatsNodeItems();
	}
}

void CNetworkInfoView::AddAdapters(CTreeItem parent) {
	auto adapters = NetworkInformation::EnumAdapters();
	int i = 0;
	for (auto& ai : adapters) {
		auto item = parent.AddTail(ai.FriendlyName.c_str(), 1);
		item.SetData(i++);
	}
	m_Adapters = std::move(adapters);
}

void CNetworkInfoView::AddInterfaces(CTreeItem parent) {
	auto interfaces = NetworkInformation::EnumInterfaces();
	int i = 0;
	for (auto& ii : interfaces) {
		auto item = parent.AddTail(ii.Description, 2);
		item.SetData(i++);
	}
	m_Interfaces = std::move(interfaces);
}

void CNetworkInfoView::SetIPTableNodeItems() {
	m_IsGenericNode = false;
	auto cm = GetColumnManager(m_List);
	cm->Clear();
	cm->AddColumn(L"If Index", LVCFMT_LEFT, 70);
	cm->AddColumn(L"Address", LVCFMT_RIGHT, 120);
	cm->AddColumn(L"Mask", LVCFMT_RIGHT, 120);
	cm->AddColumn(L"Broadcast Addr", LVCFMT_RIGHT, 120);
	cm->AddColumn(L"Reasm Size", LVCFMT_RIGHT, 120);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 200);
	cm->UpdateColumns();

	m_IPTable = NetworkInformation::EnumIPAddressTable();
	m_List.SetItemCount((int)m_IPTable.size());
}

void CNetworkInfoView::SetTcpStatsNodeItems() {
	//static auto pStats2 = (decltype(GetTcpStatisticsEx2)*)::GetProcAddress(::GetModuleHandle(L"iphlpapi.dll"), "GetTcpStatisticsEx2");
	SetListViewPropertyColumns();
	m_IsGenericNode = true;

	DoRefresh();
}

LRESULT CNetworkInfoView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT, WS_EX_CLIENTEDGE);
	m_Tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 8, 4);
	UINT icons[] = { IDI_NETWORK_INFO, IDI_NIC, IDI_INTERFACE, IDI_TABLE, IDI_ACTIVITY };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_Tree.SetImageList(images, TVSIL_NORMAL);

	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL, WS_EX_CLIENTEDGE);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	//m_Splitter.SetSplitterExtendedStyle(SPLIT_FLATBAR | SPLIT_PROPORTIONAL);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);
	m_Splitter.SetSplitterPosPct(30);

	BuildTree();
	m_GeneralNode.Select();

	return 0;
}

LRESULT CNetworkInfoView::OnTreeItemChanged(int, LPNMHDR, BOOL&) {
	m_SelectedNode = m_Tree.GetSelectedItem();
	m_List.SetItemCount(0);
	UpdateList();
	return 0;
}
