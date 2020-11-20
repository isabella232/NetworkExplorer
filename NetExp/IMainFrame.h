#pragma once

const UINT OM_ACTIVATE_PAGE = WM_APP + 1;

struct IMainFrame {
	virtual CUpdateUIBase* GetUpdateUI() = 0;
	virtual UINT TrackPopupMenu(HMENU hMenu, DWORD flags, int x, int y) = 0;
	virtual void SetStatusBarText(int pane, const CString& text) = 0;
};
