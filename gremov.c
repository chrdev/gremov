
#include <Windows.h>
#include <cfgmgr32.h>
#include <SetupAPI.h>
#include <shlwapi.h> // wvnsprintfW()

#include <stdbool.h>
#include <stdarg.h>


#define MY_VER "1.0.1"


#define stdout GetStdHandle(STD_OUTPUT_HANDLE)
#define stderr GetStdHandle(STD_ERROR_HANDLE)


static const wchar_t kHelp_[] =
	L"\r\nRedirect to save the .reg file.\r\n"
	L"E.g., gremov.exe > removal_overrides.reg\r\n"
	L"See README to learn how to schedule.\r\n";

static const wchar_t kEject_[] = 
	L"\r\nNo removable device.\r\n"
	L"If your device is showing in the eject menu, eject but don't unplug it. Then re-run me.\r\n";

static const wchar_t kSchd_[] =
	L"\r\n.reg file saved.\r\n"
	L"Scheduled task example:\r\n"
	L"Command: \"regedit.exe /s removal_overrides.reg\"\r\n"
	L"Run At:  system startup\r\n"
	L"User:    SYSTEM\r\n";

static __forceinline bool
isCmdOk(void) {
	int argc = 0;
	wchar_t** argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if (!argv) return false;
	LocalFree(argv);
	return argc <= 1;
}

static int __cdecl
hprintf(HANDLE h, const wchar_t* fmt, ...) {
	wchar_t buffer[1024];
	va_list args;
	va_start(args, fmt);
	int cch = wvnsprintfW(buffer, ARRAYSIZE(buffer), fmt, args);
	va_end(args);
	if (cch <= 0) return cch;

	// On Windows 10 and later, WriteFile() writes console right.
	// On Windows Xp and earlier, WriteFile() writes spaced letters.
	// So we try WriteConsoleW() first.
	DWORD sz = 0;
	if (!WriteConsoleW(h, buffer, cch, &sz, NULL)) {
		WriteFile(h, buffer, sizeof(wchar_t) * cch, &sz, NULL);
	}
	return cch;
}

static __forceinline bool
isatty(void) {
	return GetFileType(stdout) == FILE_TYPE_CHAR;
}

static __forceinline void
writeRegHeader(void) {
	static const wchar_t kBom = L'\xFEFF';
	static const wchar_t kHeader[41] = L"Windows Registry Editor Version 5.00\r\n\r\n";

	if (!isatty()) hprintf(stdout, L"%lc", kBom);
	hprintf(stdout, L"%ls", kHeader);
}

static __forceinline void
writeRegItem(const wchar_t* instId, DWORD cap) {
	static bool hasCalled = false;
	static const wchar_t kPat[] = L"[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Enum\\%ls]\r\n\"Capabilities\"=dword:%.8x\r\n\r\n";

	if (!hasCalled) {
		hasCalled = true;
		writeRegHeader();
	}
	hprintf(stdout, kPat, instId, cap);
}

void entry(void)
{
	if (!isCmdOk()) {
		hprintf(stderr, L"%ls", kHelp_);
		ExitProcess(1);
	}

	HDEVINFO infoSet = SetupDiGetClassDevsW(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (infoSet == INVALID_HANDLE_VALUE) ExitProcess(2);

	int rlt = 1;

	size_t total = 0;
	size_t count = 0;
	SP_DEVINFO_DATA info = { sizeof(info) };
	for (DWORD idx = 0; SetupDiEnumDeviceInfo(infoSet, idx, &info); ++idx) {
		DWORD cap;
		CONFIGRET cret;

		if (!SetupDiGetDeviceRegistryPropertyW(infoSet, &info, SPDRP_CAPABILITIES, NULL, (PBYTE)&cap, sizeof(cap), NULL)) continue;
		if (!(cap & CM_DEVCAP_REMOVABLE)) continue;
		if (cap & CM_DEVCAP_SURPRISEREMOVALOK) {
			ULONG status = 0;
			ULONG problemCode = 0;
			cret = CM_Get_DevNode_Status(&status, &problemCode, info.DevInst, 0);
			if (cret != CR_SUCCESS) continue;
			if (problemCode != CM_PROB_HELD_FOR_EJECT) continue;
		}
		++total;

		wchar_t instId[MAX_DEVICE_ID_LEN];
		cret = CM_Get_Device_IDW(info.DevInst, instId, ARRAYSIZE(instId), 0);
		if (cret != CR_SUCCESS) {
			hprintf(stderr, L"Can't get device instance id: 0x%X.\r\n", cret);
			break;
		};

		cap &= ~CM_DEVCAP_REMOVABLE;
		writeRegItem(instId, cap);
		++count;
	};

	if (total != count) goto fin;

	if (isatty()) {
		if (!total) hprintf(stderr, L"%ls", kEject_);
		hprintf(stderr, L"%ls", kHelp_);
	}
	else if (total) {
		hprintf(stderr, L"%ls", kSchd_);
	}

	rlt = 0;
fin:
	SetupDiDestroyDeviceInfoList(infoSet);
	ExitProcess((UINT)rlt);
}
