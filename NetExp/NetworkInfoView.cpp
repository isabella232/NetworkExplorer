#include "pch.h"
#include "NetworkInfoView.h"
#include "FormatHelper.h"

CString CNetworkInfoView::GetColumnText(HWND, int row, int col) const {
	auto& item = m_Items[row];
	if (m_IsGenericNode) {
		switch (col) {
			case 0: return item.Property;
			case 1: return item.ValueFn ? item.ValueFn() : item.Value;
		}
	}
	return CString();
}

PCWSTR CNetworkInfoView::GetHeader() const {
	return L"Information";
}

void CNetworkInfoView::OnFinalMessage(HWND) {
	delete this;
}

PCWSTR CNetworkInfoView::NodeTypeToString(UINT type) {
	switch (type) {
		case BROADCAST_NODETYPE: return L"Broadcast";
		case PEER_TO_PEER_NODETYPE: return L"Peer to peer";
		case MIXED_NODETYPE: return L"Mixed";
		case HYBRID_NODETYPE: return L"Hybrid";
	}
	return L"(Unknown)";
}

PCWSTR CNetworkInfoView::InterfaceTypeToString(IFTYPE type) {
	PCWSTR names[] = {
		L"OTHER",
		L"REGULAR_1822",
		L"HDH_1822",
		L"DDN_X25",
		L"RFC877_X25",
		L"ETHERNET_CSMACD",
		L"IS088023_CSMACD",
		L"ISO88024_TOKENBUS",
		L"ISO88025_TOKENRING",
		L"ISO88026_MAN",
		L"STARLAN",
		L"PROTEON_10MBIT",
		L"PROTEON_80MBIT",
		L"HYPERCHANNEL",
		L"FDDI",
		L"LAP_B",
		L"SDLC",
		L"DS1",
		L"E1",
		L"BASIC_ISDN",
		L"PRIMARY_ISDN",
		L"PROP_POINT2POINT_SERIAL",
		L"PPP",
		L"SOFTWARE_LOOPBACK",
		L"EON",
		L"ETHERNET_3MBIT",
		L"NSIP",
		L"SLIP",
		L"ULTRA",
		L"DS3",
		L"SIP",
		L"FRAMERELAY",
		L"RS232",
		L"PARA",
		L"ARCNET",
		L"ARCNET_PLUS",
		L"ATM",
		L"MIO_X25",
		L"SONET",
		L"X25_PLE",
		L"ISO88022_LLC",
		L"LOCALTALK",
		L"SMDS_DXI",
		L"FRAMERELAY_SERVICE",
		L"V35",
		L"HSSI",
		L"HIPPI",
		L"MODEM",
		L"AAL5",
		L"SONET_PATH",
		L"SONET_VT",
		L"SMDS_ICIP",
		L"PROP_VIRTUAL",
		L"PROP_MULTIPLEXOR",
		L"IEEE802.12",
		L"FIBRECHANNEL",
		L"HIPPIINTERFACE",
		L"FRAMERELAY_INTERCONNECT",
		L"AFLANE_8023",
		L"AFLANE_8025",
		L"CCTEMUL",
		L"FASTETHER",
		L"ISDN",
		L"V11",
		L"V36",
		L"G703_64K",
		L"G703_2MB",
		L"QLLC",
		L"FASTETHER_FX",
		L"CHANNEL",
		L"IEEE 802.11",
		L"IBM370PARCHAN",
		L"ESCON",
		L"DLSW",
		L"ISDN_S",
		L"ISDN_U",
		L"LAP_D",
		L"IPSWITCH",
		L"RSRB",
		L"ATM_LOGICAL",
		L"DS0",
		L"DS0_BUNDLE",
		L"BSC",
		L"ASYNC",
		L"CNR",
		L"ISO88025R_DTR",
		L"EPLRS",
		L"ARAP",
		L"PROP_CNLS",
		L"HOSTPAD",
		L"TERMPAD",
		L"FRAMERELAY_MPI",
		L"X213",
		L"ADSL",
		L"RADSL",
		L"SDSL",
		L"VDSL",
		L"ISO88025_CRFPRINT",
		L"MYRINET",
		L"VOICE_EM",
		L"VOICE_FXO",
		L"VOICE_FXS",
		L"VOICE_ENCAP",
		L"VOICE_OVERIP",
		L"ATM_DXI",
		L"ATM_FUNI",
		L"ATM_IMA",
		L"PPPMULTILINKBUNDLE",
		L"IPOVER_CDLC",
		L"IPOVER_CLAW",
		L"STACKTOSTACK",
		L"VIRTUALIPADDRESS",
		L"MPC",
		L"IPOVER_ATM",
		L"ISO88025_FIBER",
		L"TDLC",
		L"GIGABITETHERNET",
		L"HDLC",
		L"LAP_F",
		L"V37",
		L"X25_MLP",
		L"X25_HUNTGROUP",
		L"TRANSPHDLC",
		L"INTERLEAVE",
		L"FAST",
		L"IP",
		L"DOCSCABLE_MACLAYER",
		L"DOCSCABLE_DOWNSTREAM",
		L"DOCSCABLE_UPSTREAM",
		L"A12MPPSWITCH",
		L"TUNNEL",
		L"COFFEE",
		L"CES",
		L"ATM_SUBINTERFACE",
		L"L2_VLAN",
		L"L3_IPVLAN",
		L"L3_IPXVLAN",
		L"DIGITALPOWERLINE",
		L"MEDIAMAILOVERIP",
		L"DTM",
		L"DCN",
		L"IPFORWARD",
		L"MSDSL",
		L"IEEE 1394 (Firewire)",
		L"IF_GSN",
		L"DVBRCC_MACLAYER",
		L"DVBRCC_DOWNSTREAM",
		L"DVBRCC_UPSTREAM",
		L"ATM_VIRTUAL",
		L"MPLS_TUNNEL",
		L"SRP",
		L"VOICEOVERATM",
		L"VOICEOVERFRAMERELAY",
		L"IDSL",
		L"COMPOSITELINK",
		L"SS7_SIGLINK",
		L"PROP_WIRELESS_P2P",
		L"FR_FORWARD",
		L"RFC1483",
		L"USB",
		L"IEEE8023AD_LAG",
		L"BGP_POLICY_ACCOUNTING",
		L"FRF16_MFR_BUNDLE",
		L"H323_GATEKEEPER",
		L"H323_PROXY",
		L"MPLS",
		L"MF_SIGLINK",
		L"HDSL2",
		L"SHDSL",
		L"DS1_FDL",
		L"POS",
		L"DVB_ASI_IN",
		L"DVB_ASI_OUT",
		L"PLC",
		L"NFAS",
		L"TR008",
		L"GR303_RDT",
		L"GR303_IDT",
		L"ISUP",
		L"PROP_DOCS_WIRELESS_MACLAYER",
		L"PROP_DOCS_WIRELESS_DOWNSTREAM",
		L"PROP_DOCS_WIRELESS_UPSTREAM",
		L"HIPERLAN2",
		L"PROP_BWA_P2MP",
		L"SONET_OVERHEAD_CHANNEL",
		L"DIGITAL_WRAPPER_OVERHEAD_CHANNEL",
		L"AAL2",
		L"RADIO_MAC",
		L"ATM_RADIO",
		L"IMT",
		L"MVL",
		L"REACH_DSL",
		L"FR_DLCI_ENDPT",
		L"ATM_VCI_ENDPT",
		L"OPTICAL_CHANNEL",
		L"OPTICAL_TRANSPORT",
	};

	ATLASSERT(type > 0 && type <= _countof(names));
	if (type < 1 || type > _countof(names))
		return L"(Unknown)";

	return names[type - 1];
}

void CNetworkInfoView::SetAdapterNodeItems(const AdapterInfo& adapter) {
	m_Items.clear();
	SetListViewPropertyColumns();

	m_Items = {
		GenericItem(AdapterFriendlyName, L"Friendly Name", CString(adapter.FriendlyName.c_str())),
		GenericItem(AdapterIfIndex, L"Interface Index", FormatHelper::Format(adapter.IfIndex)),
		GenericItem(AdapterName, L"Name", CString(adapter.Name.c_str())),
		GenericItem(AdapterDesc, L"Description", CString(adapter.Description.c_str())),
		GenericItem(AdapterIfType, L"Interface Type", CString(InterfaceTypeToString(adapter.IfType))),
	};
	m_List.SetItemCount((int)m_Items.size());
}

void CNetworkInfoView::SetInterfaceNodeItems(const InterfaceInfo& iface) {
	m_Items.clear();
	SetListViewPropertyColumns();

	m_Items = {
		GenericItem(IfDesc, L"Description", CString(iface.Description)),
		GenericItem(IfIndex, L"Index", FormatHelper::Format(iface.InterfaceIndex)),
		GenericItem(IfAlias, L"Alias", CString(iface.Alias)),
		GenericItem(IfType, L"Type", CString(InterfaceTypeToString(iface.Type))),
		GenericItem(IfGuid, L"GUID", FormatHelper::GuidToString(iface.InterfaceGuid)),
	};
	m_List.SetItemCount((int)m_Items.size());
}

void CNetworkInfoView::SetGeneralNodeItems() {
	ULONG size = sizeof(m_FixedInfoBuffer);
	m_FixedInfo = (PFIXED_INFO)m_FixedInfoBuffer;
	::GetNetworkParams(m_FixedInfo, &size);

	SetListViewPropertyColumns();
	m_Items.clear();
	m_Items = {
		GenericItem(GenericItemType::HostName, L"Host Name", CString(m_FixedInfo->HostName)),
		GenericItem(GenericItemType::DomainName, L"Domain Name", CString(m_FixedInfo->DomainName)),
		GenericItem(GenericItemType::NodeType, L"Node Type", NodeTypeToString(m_FixedInfo->NodeType)),
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
	while (m_List.DeleteColumn(0))
		;

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
	item = m_AdaptersNode = m_Tree.InsertItem(L"Adapters", 1, 1, TVI_ROOT, TVI_LAST);
	AddAdapters(item);
	item.SortChildren(FALSE);
	item = m_InterfacesNode = m_Tree.InsertItem(L"Interfaces", 2, 2, TVI_ROOT, TVI_LAST);
	AddInterfaces(item);
	item.SortChildren(FALSE);
	item = m_Tree.InsertItem(L"Address Table", 3, 3, TVI_ROOT, TVI_LAST);

	m_Tree.LockWindowUpdate(FALSE);
}

void CNetworkInfoView::UpdateList() {
	if (m_SelectedNode == m_GeneralNode) {
		SetGeneralNodeItems();
		m_IsGenericNode = true;
	}
	else if (m_SelectedNode.GetParent() == m_AdaptersNode) {
		SetAdapterNodeItems(m_Adapters[m_SelectedNode.GetData()]);
	}
	else if (m_SelectedNode.GetParent() == m_InterfacesNode) {
		SetInterfaceNodeItems(m_Interfaces[m_SelectedNode.GetData()]);
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

LRESULT CNetworkInfoView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT);
	m_Tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 8, 4);
	UINT icons[] = { IDI_NETWORK_INFO, IDI_NIC, IDI_INTERFACE, IDI_TABLE };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_Tree.SetImageList(images, TVSIL_NORMAL);

	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	//m_Splitter.SetSplitterExtendedStyle(SPLIT_FLATBAR | SPLIT_PROPORTIONAL);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);
	m_Splitter.SetSplitterPosPct(35);

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
