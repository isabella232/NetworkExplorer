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

HICON SecurityHelper::GetShieldIcon() {
	SHSTOCKICONINFO ssii = { sizeof(ssii) };
	if (FAILED(::SHGetStockIconInfo(SIID_SHIELD, SHGSI_SMALLICON | SHGSI_ICON, &ssii)))
		return nullptr;

	return ssii.hIcon;
}
