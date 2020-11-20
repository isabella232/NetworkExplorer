#include "pch.h"
#include "SecurityHelper.h"
#include "resource.h"

bool SecurityHelper::IsRunningElevated() {
	static bool runningElevated = false;
	static bool runningElevatedCheck = false;
	if (runningElevatedCheck)
		return runningElevated;

	runningElevatedCheck = true;
	HANDLE hToken;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return false;

	TOKEN_ELEVATION te;
	DWORD len;
	if (::GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &len)) {
		runningElevated = te.TokenIsElevated ? true : false;
	}
	::CloseHandle(hToken);
	return runningElevated;
}

bool SecurityHelper::RunElevated() {
	WCHAR path[MAX_PATH];
	::GetModuleFileName(nullptr, path, _countof(path));
	return (INT_PTR)::ShellExecute(nullptr, L"runas", path, nullptr, nullptr, SW_SHOWDEFAULT) > 31;
}

bool SecurityHelper::IsTokenAdminGroup() {
	DWORD		tokenInfoBufLen;
	PVOID		tokenInfoBuf;
	PTOKEN_GROUPS	tokenGroups;
	SID_IDENTIFIER_AUTHORITY	ntauth = SECURITY_NT_AUTHORITY;
	void* psidAdmin = NULL;
	BOOLEAN		isAdmin = FALSE;
	HANDLE		hToken;
	DWORD		i;

	OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);

	//
	// Allocate admin group SID
	//
	AllocateAndInitializeSid(&ntauth, 2,
		SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0, &psidAdmin);

	//
	// Check the user field first
	//
	GetTokenInformation(hToken,
		TokenGroups,
		NULL,
		0,
		&tokenInfoBufLen);
	tokenInfoBuf = malloc(tokenInfoBufLen);
	if (GetTokenInformation(hToken,
		TokenGroups,
		tokenInfoBuf,
		tokenInfoBufLen,
		&tokenInfoBufLen)) {

		tokenGroups = (PTOKEN_GROUPS)tokenInfoBuf;


		for (i = 0; i < tokenGroups->GroupCount; ++i) {

			if (EqualSid(psidAdmin, tokenGroups->Groups[i].Sid) &&
				tokenGroups->Groups[i].Attributes & SE_GROUP_ENABLED &&
				tokenGroups->Groups[i].Attributes & ~SE_GROUP_USE_FOR_DENY_ONLY) {

				break;
			}
		}
		isAdmin = (i != tokenGroups->GroupCount);
	}
	free(tokenInfoBuf);
	FreeSid(psidAdmin);
	CloseHandle(hToken);
	return isAdmin;
}

void SecurityHelper::ErrorDialog(HWND hParent, PCWSTR message, DWORD error) {
	LPWSTR  lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr, error,	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, nullptr);

	CString text;
	text.Format(L"%s: %s", message, lpMsgBuf);
	if ((error == ERROR_ACCESS_DENIED || error == ERROR_ELEVATION_REQUIRED) &&
		// Unmanifested applications on Windows 8 and higher will return the
		// Windows 8 version (6.2). Since this test is for >=6, this is acceptable.
		::IsWindowsVistaOrGreater() && !IsTokenAdminGroup()) {

		AtlMessageBox(hParent, (PCWSTR)text, IDS_ERRORTITLE, MB_OK | MB_ICONERROR);

		//DialogBoxParam(GetModuleHandle(NULL), "AccessDenied", hParent, (DLGPROC)About,
		//	(LPARAM)_strdup(errmsg));

	}
	else if (error == ERROR_INVALID_HANDLE) {
		AtlMessageBox(hParent, L"This object type cannot be opened", IDS_ERRORTITLE, MB_OK | MB_ICONERROR);
	}
	else {
		AtlMessageBox(hParent, (PCWSTR)text, IDS_ERRORTITLE, MB_OK | MB_ICONERROR);
	}
	::LocalFree(lpMsgBuf);
}
