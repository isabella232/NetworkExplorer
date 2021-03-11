// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "ConnectionsView.h"
#include "SortHelper.h"
#include <Ip2string.h>
#include "ClipboardHelper.h"

#pragma comment(lib, "ntdll")

CString CConnectionsView::GetColumnText(HWND, int row, int col) const {
	const auto& item = m_Items[row];
	CString text;

	switch (col) {
		case 0: return item->Pid == 0 ? L"" : GetProcessName(item.get());
		case 1:
			if(item->Pid > 0)
				text.Format(L"%u (0x%X)", item->Pid, item->Pid);
			break;

		case 2: return ConnectionTypeToString(item->Type);
		case 3: return ConnectionStateToString(item->State);
		case 4: return item->Type == ConnectionType::TcpV6 || item->Type == ConnectionType::UdpV6 ? IPAddressToString(item->LocalAddressV6) : IPAddressToString(item->LocalAddress);
		case 5:
			text.Format(L"%d", item->LocalPort);
			break;
		case 6:
			if (item->Type == ConnectionType::Udp || item->Type == ConnectionType::UdpV6)
				return L"";
			return item->Type == ConnectionType::TcpV6 ? IPAddressToString(item->RemoteAddressV6) : IPAddressToString(item->RemoteAddress);
		case 7:
			if (item->Type == ConnectionType::Udp || item->Type == ConnectionType::UdpV6)
				return L"";
			text.Format(L"%d", item->RemotePort);
			break;
		case 8: 
			if(item->TimeStamp)
				text.Format(L"%s.%03d", (PCWSTR)CTime(*(FILETIME*)&item->TimeStamp).Format(L"%x %X"), int(item->TimeStamp % 1000));
			break;
		case 9: return item->ModuleName.c_str();
		case 10: return item->ModulePath.c_str();
	}
	return text;
}

int CConnectionsView::GetRowImage(HWND, int row) const {
	const auto& item = m_Items[row].get();
	return item->Pid == 0 ? -1 : GetItemEx(item)->Image;
}

const CString CConnectionsView::GetProcessName(Connection* conn) const {
	auto itemx = GetItemEx(conn);
	if (itemx)
		return itemx->ProcessName;
	ItemEx ix;
	ix.ProcessName = m_pm.GetProcessName(conn->Pid);
	auto hIcon = m_pm.GetProcessIcon(conn->Pid);
	if (hIcon) {
		if (auto it = m_IconMap.find(hIcon); it != m_IconMap.end())
			ix.Image = it->second;
		else
			ix.Image = m_Images.AddIcon(hIcon);
	}
	m_ItemsEx.insert({ *conn, ix });
	return ix.ProcessName;
}

void CConnectionsView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	auto compare = [&](auto& i1, auto& i2) -> bool {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortStrings(GetProcessName(i1.get()), GetProcessName(i2.get()), si->SortAscending);
			case 1: return SortHelper::SortNumbers(i1->Pid, i2->Pid, si->SortAscending);
			case 2: return SortHelper::SortStrings(ConnectionTypeToString(i1->Type), ConnectionTypeToString(i2->Type), si->SortAscending);
			case 3: return SortHelper::SortNumbers(i1->State, i2->State, si->SortAscending);
			case 4: return SortHelper::SortNumbers(SwapBytes(i1->LocalAddress), SwapBytes(i2->LocalAddress), si->SortAscending);
			case 5: return SortHelper::SortNumbers(i1->LocalPort, i2->LocalPort, si->SortAscending);
			case 6: return SortHelper::SortNumbers(SwapBytes(i1->RemoteAddress), SwapBytes(i2->RemoteAddress), si->SortAscending);
			case 7: return SortHelper::SortNumbers(i1->RemotePort, i2->RemotePort, si->SortAscending);
			case 8: return SortHelper::SortNumbers(i1->TimeStamp, i2->TimeStamp, si->SortAscending);
			case 9: return SortHelper::SortStrings(i1->ModuleName, i2->ModuleName, si->SortAscending);
		}
		return false;
	};
	m_Items.Sort(compare);
}

DWORD CConnectionsView::OnPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CConnectionsView::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	int row = (int)cd->dwItemSpec;
	const auto& item = m_Items[row];
	auto itemx = GetItemEx(item.get());
	if (itemx) {
		auto lv = (NMLVCUSTOMDRAW*)cd;
		if (itemx->State == ItemState::New)
			lv->clrTextBk = RGB(0, 240, 0);
		else if (itemx->State == ItemState::Deleted)
			lv->clrTextBk = RGB(255, 64, 0);
	}
	return CDRF_DODEFAULT;
}

PCWSTR CConnectionsView::GetHeader() const {
	return L"Active Connections";
}

void CConnectionsView::AddConnection(SortedFilteredVector<std::shared_ptr<Connection>>& vec, std::shared_ptr<Connection> conn, bool reallyNew) {
	vec.push_back(conn);
	auto pix = GetItemEx(conn.get());
	ItemEx ix;
	if (pix == nullptr)
		pix = &ix;
	if (pix->ProcessName.IsEmpty()) {
		pix->ProcessName = m_pm.GetProcessName(conn->Pid);
		auto hIcon = m_pm.GetProcessIcon(conn->Pid);
		if (hIcon) {
			if (auto it = m_IconMap.find(hIcon); it == m_IconMap.end()) {
				pix->Image = m_Images.AddIcon(hIcon);
				m_IconMap.insert({ hIcon, pix->Image });
			}
			else {
				pix->Image = it->second;
			}
		}
		m_ItemsEx.insert({ *conn, ix });
	}
}

void CConnectionsView::DoRefresh() {
	if (m_Items.empty()) {
		// first time
		auto count = m_Tracker.EnumConnections();
		for (auto& conn : m_Tracker.GetConnections()) {
			AddConnection(m_Items, conn, false);
		}
		m_List.SetItemCount(count);
		m_ItemsEx.reserve(count + 16);
	}
	else {
		decltype(m_Items) local(m_Items.size());

		auto time = ::GetTickCount64();
		int newCount = (int)m_NewConnections.size();
		for(int i = 0; i < newCount; i++) {
			auto& conn = m_NewConnections[i];
			auto ix = GetItemEx(conn.get());
			if (ix->TargetTime <= time) {
				ix->State = ItemState::None;
				m_NewConnections.erase(m_NewConnections.begin() + i);
				i--;
				newCount--;
			}
		}

		auto oldCount = (int)m_OldConnections.size();
		for(int i = 0; i < oldCount; i++) {
			auto& conn = m_OldConnections[i];
			auto ix = GetItemEx(conn.get());
			if (ix && ix->TargetTime <= time) {
				m_OldConnections.erase(m_OldConnections.begin() + i);
				i--;
				oldCount--;
			}
			else {
				local.push_back(conn);
			}
		}

		m_Tracker.EnumConnections();
		//m_ItemsEx.clear();
		//m_OldConnections.clear();

		for (auto& conn : m_Tracker.GetConnections()) {
			AddConnection(local, conn);
		}

		for (auto& conn : m_Tracker.GetNewConnections()) {
			auto ix = GetItemEx(conn.get());
			ATLASSERT(ix);
			ix->State = ItemState::New;
			ix->TargetTime = time + GetUpdateInterval();
			m_NewConnections.push_back(conn);
		}
		for (auto& conn : m_Tracker.GetClosedConnections()) {
			AddConnection(local, conn);
			auto ix = GetItemEx(conn.get());
			ATLASSERT(ix);
			ix->State = ItemState::Deleted;
			ix->TargetTime = time + GetUpdateInterval();
			m_OldConnections.push_back(conn);
		}


		m_Items = std::move(local);
		m_List.SetItemCountEx((int)m_Items.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
		DoSort(GetSortInfo(m_List));
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}
}


void CConnectionsView::DoUpdate() {
	DoRefresh();
}

void CConnectionsView::OnActivate(bool activate) {
	if (activate) {
		auto ui = GetFrame()->GetUpdateUI();
		const ConnectionType protocols[] = { ConnectionType::Tcp, ConnectionType::TcpV6, ConnectionType::Udp, ConnectionType::UdpV6 };
		auto flags = m_Tracker.GetTrackingFlags();
		for (int i = 0; i < _countof(protocols); i++)
			ui->UISetCheck(ID_PROTOCOLS_TCP + i, (flags & protocols[i]) == protocols[i]);
	}
}

PCWSTR CConnectionsView::ConnectionTypeToString(ConnectionType type) {
	switch (type) {
		case ConnectionType::Tcp: return L"TCP";
		case ConnectionType::TcpV6: return L"TCPv6";
		case ConnectionType::Udp: return L"UDP";
		case ConnectionType::UdpV6: return L"UDPv6";
	}
	ATLASSERT(false);
	return nullptr;
}

PCWSTR CConnectionsView::ConnectionStateToString(MIB_TCP_STATE state) {
	switch (state) {
		case MIB_TCP_STATE_CLOSED: return L"Closed";
		case MIB_TCP_STATE_LISTEN: return L"Listen";
		case MIB_TCP_STATE_SYN_SENT: return L"Syn Sent";
		case MIB_TCP_STATE_SYN_RCVD: return L"Syn Received";
		case MIB_TCP_STATE_ESTAB: return L"Established";
		case MIB_TCP_STATE_FIN_WAIT1: return L"Fin Wait 1";
		case MIB_TCP_STATE_FIN_WAIT2: return L"Fin Wait 2";
		case MIB_TCP_STATE_CLOSE_WAIT: return L"Close Wait";
		case MIB_TCP_STATE_CLOSING: return L"Closing";
		case MIB_TCP_STATE_LAST_ACK: return L"Ack";
		case MIB_TCP_STATE_TIME_WAIT: return L"Time Wait";
		case MIB_TCP_STATE_DELETE_TCB: return L"Delete TCB";
	}
	return L"";
}

CString CConnectionsView::IPAddressToString(DWORD ip) {
	WCHAR buffer[46];
	::RtlIpv4AddressToString((in_addr*)&ip, buffer);
	return buffer;
}

CString CConnectionsView::IPAddressToString(const IN6_ADDR& ip) {
	WCHAR buffer[46];
	::RtlIpv6AddressToString((in6_addr*)&ip, buffer);
	return buffer;
}

DWORD CConnectionsView::SwapBytes(DWORD x) {
	auto p = (BYTE*)&x;
	std::swap(p[0], p[3]);
	std::swap(p[1], p[2]);
	return x;
}

BOOL CConnectionsView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CConnectionsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

CString CConnectionsView::GetListLine(int line) const {
	auto columns = GetColumnManager(m_List)->GetCount();
	CString text;
	for(int i = 0; i < columns; i++) {
		CString item;
		m_List.GetItemText(line, i, item);
		text += item + L",";
	}
	return text.Left(text.GetLength() - 1);
}

LRESULT CConnectionsView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		LVS_SINGLESEL | LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_HEADERINALLVIEWS);

	m_Images.Create(16, 16, ILC_COLOR32, 32, 16);
	m_Images.AddIcon(AtlLoadSysIcon(IDI_APPLICATION));
	m_List.SetImageList(m_Images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 150);
	cm->AddColumn(L"Process ID", LVCFMT_RIGHT, 100);
	cm->AddColumn(L"Protocol", LVCFMT_CENTER, 80);
	cm->AddColumn(L"State", LVCFMT_LEFT, 80);
	cm->AddColumn(L"Local Address", LVCFMT_RIGHT, 150);
	cm->AddColumn(L"Local Port", LVCFMT_RIGHT, 90);
	cm->AddColumn(L"Remote Address", LVCFMT_RIGHT, 150);
	cm->AddColumn(L"Remote Port", LVCFMT_RIGHT, 90);
	cm->AddColumn(L"Create Time", LVCFMT_RIGHT, 140);
	cm->AddColumn(L"Module Name", LVCFMT_LEFT, 180);
//	cm->AddColumn(L"Module Path", LVCFMT_LEFT, 300);
	cm->UpdateColumns();

	DoRefresh();

	return 0;
}

LRESULT CConnectionsView::OnToggleProtocol(WORD, WORD id, HWND, BOOL&) {
	const ConnectionType protocols[] = { ConnectionType::Tcp, ConnectionType::TcpV6, ConnectionType::Udp, ConnectionType::UdpV6 };
	int index = id - ID_PROTOCOLS_TCP;
	auto flags = m_Tracker.GetTrackingFlags();
	auto on = (flags & protocols[index]) != ConnectionType::Invalid;
	m_Tracker.SetTrackingFlags(on ? (flags & ~protocols[index]) : (flags | protocols[index]));
	GetFrame()->GetUpdateUI()->UISetCheck(id, on ? FALSE : TRUE);

	return 0;
}

LRESULT CConnectionsView::OnEditCopy(WORD, WORD, HWND, BOOL&) {
	int selected = m_List.GetSelectedIndex();
	if(selected >= 0) {
		ClipboardHelper::CopyText(*this, GetListLine(selected));
	}
	return 0;
}

LRESULT CConnectionsView::OnFileSave(WORD, WORD, HWND, BOOL&) {
	auto paused = IsPaused();
	if(!paused)
		Pause(true);

	CSimpleFileDialog dlg(FALSE, L"csv", nullptr, OFN_OVERWRITEPROMPT | OFN_ENABLESIZING,
		L"CSV Files (*.csv)\0*.csv\0All Files\0*.*\0", *this);
	if(dlg.DoModal() == IDOK) {
		HANDLE hFile = ::CreateFile(dlg.m_szFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
		if(hFile == INVALID_HANDLE_VALUE) {
			AtlMessageBox(*this, L"Failed to create file", IDS_TITLE, MB_ICONERROR);
			return 0;
		}
		DWORD bytes;
		for(int i = 0; i < m_List.GetItemCount(); i++) {
			auto text = GetListLine(i) + L"\n";
			::WriteFile(hFile, text.GetBuffer(), text.GetLength() * sizeof(WCHAR), &bytes, nullptr);
		}
		::CloseHandle(hFile);
	}
	if(!paused)
		Pause(false);

	return 0;
}

CConnectionsView::ItemEx* CConnectionsView::GetItemEx(Connection* conn) const {
	if (auto it = m_ItemsEx.find(*conn); it != m_ItemsEx.end())
		return &it->second;
	return nullptr;
}
