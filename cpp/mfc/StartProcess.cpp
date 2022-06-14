// 서비스에서 UI 프로그램을 실행 시키기 위한 도전 소스

BOOL StartProcess(const std::wstring & runApp, const std::wstring & runPath)
{
	// UI 실행 프로그램 추가
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;
	bool bResult = FALSE;
	DWORD dwSessionID(NULL), winLoginPID(NULL);
	HANDLE hUserToken(nullptr), hUserTokenDup(nullptr), hPToken(nullptr), hProcess(nullptr);
	DWORD dwCreationFlags;

	dwSessionID = WTSGetActiveConsoleSessionId();

	PROCESSENTRY32W procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) {

	}

	if (!Process32FirstW(hSnap, &procEntry)) {
		DWORD dwErr = GetLastError();
		return FALSE;
	}

	do {
		//if (_wcsicmp(procEntry.szExeFile, L"winlogon.exe") == 0) {
		if (_wcsicmp(procEntry.szExeFile, L"TCCInsCommonCtl.exe") == 0) {
			DWORD winlogonSessID = 0;
			/*
			if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessID) && winlogonSessID == dwSessionID) {
				winLoginPID = procEntry.th32ProcessID;
				break;
			}
			*/
			winLoginPID = procEntry.th32ProcessID;
			break;
		}
	} while (Process32NextW(hSnap, &procEntry));

	if (winLoginPID == NULL) {
		return FALSE;
	}

	////////////////////////////////////////////////////////////////////

	WTSQueryUserToken(dwSessionID, &hUserToken);
	dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
	ZeroMemory(&si, sizeof(STARTUPINFOW));
	si.cb = sizeof(STARTUPINFOW);
	si.lpDesktop = L"winsta0\\default";
	ZeroMemory(&pi, sizeof(pi));
	TOKEN_PRIVILEGES tp;
	LUID luid;
	hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, winLoginPID);

	if (!::OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, &hPToken)) {
		char pTemp[121];
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		char pTemp[121];
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hUserTokenDup);
	int dup = GetLastError();

	SetTokenInformation(hUserTokenDup, TokenSessionId, (void *)dwSessionID, sizeof(DWORD));

	if (!AdjustTokenPrivileges(hUserTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL)) {
		char pTemp[121];
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
	}

	LPVOID pEnv = NULL;

	if (CreateEnvironmentBlock(&pEnv, hUserTokenDup, TRUE)) {
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
	}
	else {
		pEnv = NULL;
	}

	bResult = CreateProcessAsUserW(
		hUserTokenDup,
		(runPath + L"\\" + runApp).c_str(),
		NULL,
		NULL,
		NULL,
		FALSE,
		dwCreationFlags,
		pEnv,
		runPath.c_str(),
		&si,
		&pi
	);

	if (bResult) {
	}
	else {
		long nError = GetLastError();
		char pTemp[256];
		return FALSE;
	}

	int iResultOfCreateProcessAsUser = GetLastError();

	if (pi.hProcess != NULL) {
		DWORD dwWait = ::WaitForInputIdle(pi.hProcess, INFINITE);
		//::CloseHandle(pi.hProcess);	// close 하면 프로세스가 정상적인지 체크 할 수가 없게 된다.
	}

	// 프로그램 최상위로 올리기
	// 트레이에 실행할 프로그램은 크게 의미가 없다.
	::SetForegroundWindow((HWND)pi.hProcess);
	::SetWindowPos((HWND)pi.hProcess, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	::SetWindowPos((HWND)pi.hProcess, HWND_NOTOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	if (hProcess) {
		CloseHandle(hProcess);
		hProcess = (nullptr);
	}

	if (hUserToken) {
		CloseHandle(hUserToken);
		hUserToken = (nullptr);
	}

	if (hUserTokenDup) {
		CloseHandle(hUserTokenDup);
		hUserTokenDup = (nullptr);
	}

	if (hPToken) {
		CloseHandle(hPToken);
		hPToken = (nullptr);
	}

	return bResult;
}
