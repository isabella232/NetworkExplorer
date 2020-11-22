#include "pch.h"
#include "FormatHelper.h"

CString FormatHelper::Format(int value, bool hex) {
    CString text;
    text.Format(hex ? L"0x%X" : L"%d", value);
    return text;
}

CString FormatHelper::Format(DWORD value, bool hex) {
    CString text;
    text.Format(hex ? L"0x%X" : L"%u", value);
    return text;
}

CString FormatHelper::GuidToString(const GUID& guid) {
    WCHAR text[64];
    ::StringFromGUID2(guid, text, _countof(text));
    return text;
}
