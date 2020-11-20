#pragma once

struct ClipboardHelper final abstract {
	static bool CopyText(HWND hWnd, PCWSTR text);
};

