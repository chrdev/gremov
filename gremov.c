#define UNICODE

#include <sdkddkver.h>
#include <Windows.h>
#include <cfgmgr32.h>
#include <SetupAPI.h>

#include <stdbool.h>


// If t is NUL-terminated, len can be -1
static void
errout(const wchar_t* t, int len) {
	if (!t) return;
	if (len < 0) len = lstrlenW(t);
	HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
	DWORD dum;
	if (GetConsoleMode(h, &dum)) {
		WriteConsole(h, t, (DWORD)len, &dum, NULL);
	}
	else {
		WriteFile(h, t, (DWORD)(sizeof(*t) * len), &dum, NULL);
	}
}

static inline wchar_t*
getOutputPath(void) {
	int argc;
	wchar_t** argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if (argc != 2) goto err;

	switch (argv[1][0]) {
	case L'/':
	case L'-':
		goto err;
	}
	// We don't care argv never LocalFree()ed, let it die with the process.
	return argv[1];

err:;
	LocalFree(argv);
	return NULL;
}

static void
help(void) {
	static const wchar_t t[] =
		L"E.g., gremov.exe removal_overrides.reg\r\n"
		L"See README for usage.\r\n";
	errout(t, ARRAYSIZE(t) - 1);
}

static HANDLE
createRegFile(const wchar_t* path, const wchar_t** errmsg) {
	static const wchar_t* kErrExists = L"File exists.\r\n";
	static const wchar_t* kErrWriting = L"Can\'t write file.\r\n";

	static const wchar_t kHeader[41] = L"\xFEFFWindows Registry Editor Version 5.00\r\n\r\n";

	HANDLE h = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		if (errmsg) *errmsg = kErrExists;
		return INVALID_HANDLE_VALUE;
	}

	DWORD cb;
	if (!WriteFile(h, kHeader, sizeof(kHeader), &cb, NULL)) {
		CloseHandle(h);
		DeleteFile(path);
		if (errmsg) *errmsg = kErrWriting;
		return INVALID_HANDLE_VALUE;
	}

	return h;
}

// Return fixed length string of 8 characters, not NUL-terminated.
static const wchar_t*
hexFromDword(DWORD n) {
	static const wchar_t kTxt[16] = L"0123456789abcdef";
	static const wchar_t kZero[8] = L"00000000";

	static wchar_t buf[8];
	CopyMemory(buf, kZero, sizeof(kZero));

	for (int i = 7; n; n >>= 4) {
		buf[i--] = kTxt[n & 0xF];
	}
	
	return buf;
}

static bool
writeCapabilities(HANDLE h, const wchar_t* instPath, DWORD instPathLen, DWORD cap, const wchar_t** errmsg) {
	static const wchar_t* kErrWriting = L"Can\'t write file.\r\n";

	static const wchar_t kP0[50] = L"[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\";
	static const wchar_t kP1[24] = L"]\r\n\"Capabilities\"=dword:";
	static const wchar_t kP2[4] = L"\r\n\r\n";

	DWORD cb;
	if (!WriteFile(h, kP0, sizeof(kP0), &cb, NULL)) goto err;
	if (!WriteFile(h, instPath, sizeof(*instPath) * instPathLen, &cb, NULL)) goto err;
	if (!WriteFile(h, kP1, sizeof(kP1), &cb, NULL)) goto err;
	if (!WriteFile(h, hexFromDword(cap), sizeof(wchar_t) * 8, &cb, NULL)) goto err;
	if (!WriteFile(h, kP2, sizeof(kP2), &cb, NULL)) goto err;
	return true;

err:;
	if (errmsg) *errmsg = kErrWriting;
	return false;
}

void
entry(void)
{
	wchar_t* fpath = getOutputPath();
	if (!fpath) {
		help();
		ExitProcess(1);
	}

	HDEVINFO infoSet = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (infoSet == INVALID_HANDLE_VALUE) ExitProcess(2);

	int rlt = 0;
	const wchar_t* errmsg;

	HANDLE h = createRegFile(fpath, &errmsg);
	if (h == INVALID_HANDLE_VALUE) {
		errout(errmsg, -1);
		rlt = 3;
		goto fin;
	}

	SP_DEVINFO_DATA infoData = { sizeof(infoData) };
	for (DWORD idx = 0;; ++idx) {
		if (!SetupDiEnumDeviceInfo(infoSet, idx, &infoData)) break;
		DWORD cap;
		if (!SetupDiGetDeviceRegistryProperty(infoSet, &infoData, SPDRP_CAPABILITIES, NULL, (PBYTE)&cap, sizeof(cap), NULL)) continue;
		if (!(cap & CM_DEVCAP_REMOVABLE)) continue;
		if (cap & CM_DEVCAP_SURPRISEREMOVALOK) continue;

		wchar_t instPath[MAX_DEVICE_ID_LEN];
		DWORD cch;
		if (!SetupDiGetDeviceInstanceId(infoSet, &infoData, instPath, ARRAYSIZE(instPath), &cch)) {
			continue;
		}

		cap &= ~CM_DEVCAP_REMOVABLE;
		if (!writeCapabilities(h, instPath, cch - 1, cap, &errmsg)) {
			errout(errmsg, -1);
			rlt = 4;
			break;
		}
	};

	CloseHandle(h);

fin:
	SetupDiDestroyDeviceInfoList(infoSet);
	ExitProcess(rlt);
}
