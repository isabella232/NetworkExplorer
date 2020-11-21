// ConnectionsView.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ViewBase.h"
#include "SortedFilteredVector.h"
#include "VirtualListView.h"
#include "ActiveConnectionTracker.h"
#include "ProcessManager.h"

class CConnectionsView : 
	public CViewBase<CConnectionsView>,
	public CVirtualListView<CConnectionsView>,
	public CCustomDraw<CConnectionsView> {
public:
	DECLARE_WND_CLASS(NULL)

	CConnectionsView(IMainFrame* frame) : CViewBase(frame) {}

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(HWND, int row) const;

	const CString GetProcessName(Connection*) const;

	void DoSort(const SortInfo* si);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);

	PCWSTR GetHeader() const;
	void AddConnection(SortedFilteredVector<std::shared_ptr<Connection>>& vec, std::shared_ptr<Connection> conn, bool reallyNew = true);

	void DoRefresh();
	void DoUpdate();
	void OnActivate(bool activate);

	static PCWSTR ConnectionTypeToString(ConnectionType type);
	static PCWSTR ConnectionStateToString(MIB_TCP_STATE state);
	static CString IPAddressToString(DWORD ip);
	static CString IPAddressToString(const IN6_ADDR& ip);
	static DWORD SwapBytes(DWORD x);

	BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CConnectionsView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CConnectionsView>)
		CHAIN_MSG_MAP(CCustomDraw<CConnectionsView>)
		CHAIN_MSG_MAP(CViewBase<CConnectionsView>)
	ALT_MSG_MAP(1)
		COMMAND_RANGE_HANDLER(ID_PROTOCOLS_TCP, ID_PROTOCOLS_UDPV6, OnToggleProtocol)
		CHAIN_MSG_MAP_ALT(CViewBase<CConnectionsView>, 1)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnToggleProtocol(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	enum class ItemState { None, New, Deleted, DeletePending = 4};
	struct ItemEx {
		ItemState State{ ItemState::None };
		DWORD64 TargetTime;
		CString ProcessName;
		int Image{ 0 };
	};

	ItemEx* GetItemEx(Connection* conn) const;

	CListViewCtrl m_List;
	SortedFilteredVector<std::shared_ptr<Connection>> m_Items;
	std::vector<std::shared_ptr<Connection>> m_NewConnections, m_OldConnections;
	mutable std::unordered_map<Connection, ItemEx> m_ItemsEx;
	std::unordered_map<HICON, int> m_IconMap;
	ActiveConnectionTracker m_Tracker;
	ProcessManager m_pm;
	mutable CImageList m_Images;
};
