#define VERSION_SAFE_STEAM_API_INTERFACES
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <fstream>
#include <string>
#include <tier0/dbg.h>
#include <tier1/interface.h>
#include <tier2/tier2.h>
#include <steam/steam_api.h>

#ifdef OSX
#include <vector>
#include <libproc.h>
#endif

namespace
{
bool g_bIsInited = false;
ISteamClient017 *g_pSteamClient017 = nullptr;

#ifdef SOURCE_SDK_MIN_STEAM_API
typedef void (S_CALLTYPE *RegisterCallResultFn)(class CCallbackBase *pCallback, SteamAPICall_t hAPICall);
typedef void (S_CALLTYPE *UnregisterCallResultFn)(class CCallbackBase *pCallback, SteamAPICall_t hAPICall);

RegisterCallResultFn g_pfnRegisterCallResult = nullptr;
UnregisterCallResultFn g_pfnUnregisterCallResult = nullptr;

bool g_bIsAvailable = false;
#endif

//------------------------------------------------------------------

#ifdef SOURCE_SDK_MIN_STEAM_API
void LoadSteamAPIFuncsInRuntime()
{
	HMODULE hModule = nullptr;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, "steam_api.dll", &hModule);

	if (!hModule)
	{
		Error("LoadSteamAPIFuncsInRuntime: steam_api.dll not found\n");
		return;
	}

	g_pfnRegisterCallResult = reinterpret_cast<RegisterCallResultFn>(GetProcAddress(hModule, "SteamAPI_RegisterCallResult"));
	g_pfnUnregisterCallResult = reinterpret_cast<UnregisterCallResultFn>(GetProcAddress(hModule, "SteamAPI_UnregisterCallResult"));

	g_bIsAvailable = g_pfnRegisterCallResult && g_pfnUnregisterCallResult;
}
#endif

void LoadSteamClient017()
{
	if (g_pSteamClient017)
		return;

	CSysModule *steamClientModule = nullptr;
#if defined(_WIN32)
	// steamclient.dll should be loaded by steam_api
	steamClientModule = Sys_LoadModule("steamclient" DLL_EXT_STRING);
	if (!steamClientModule)
	{
		Warning("Failed to load steamclient.dll\n");
		return;
	}
#elif defined(LINUX)
	// steamclient.so should be loaded by steam_api
	std::string libpath;
	try
	{
		// Check /proc/self/maps to find full path to steamclient.so.
		std::ifstream file;
		file.exceptions(std::ios::failbit | std::ios::badbit);
		file.open("/proc/self/maps");

		std::string line;
		std::string libname = "steamclient.so";
		while (std::getline(file, line))
		{
			if (line.size() >= libname.size() &&
			    std::string_view(line).substr(line.size() - libname.size()) == libname)
			{
				size_t idx = line.find_last_of(' ');
				libpath = line.substr(idx + 1);
				break;
			}
		}
	}
	catch (const std::exception &e)
	{
		Warning("Failed to read /proc/self/maps: %s\n", e.what());
		return;
	}

	if (libpath.empty())
	{
		Warning("Failed to get full path to steamclient.so\n");
		return;
	}

	steamClientModule = Sys_LoadModule(libpath.c_str());
#elif defined(OSX)
	std::string target_process("steam_osx");
	
	// To get a list of mapped libraries in any given executable we need to first access its
	// task port with task_for_pid-allow, this requires a special entitlement and code signing
	// which is overkill for this narrow purpose. The quick and easy solution to get around
	// that and find this library dynamically regardless of installation path is to find an
	// instance of steam_osx, strip the name, and substitute steamclient.dylib in its place.
	// This method will always work because steam_osx is required for any game to function and
	// they're both contained within the same application bundle. Sys_LoadModule will always be
	// passed the correct path to steamclient.dylib or somehow even in the event of a fail condition,
	// the path to an unknown process is given, failing silently.
	int size = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
	size_t count = size / sizeof(pid_t);
	
	std::vector<pid_t> pids(count);
	
	proc_listpids(PROC_ALL_PIDS, 0, pids.data(), size);
	
	char buffer[PROC_PIDPATHINFO_MAXSIZE] = {};
	
	for (auto &pid : pids) {
		if (!pid) {
			continue;
		}
		
		if (proc_pidpath(pid, buffer, sizeof(buffer)) < 0) {
			continue;
		}
		
		if (strstr(buffer, target_process.c_str()) != nullptr) {
			break;
		}
	}
	
	std::string path = buffer;
	size_t found = path.find(target_process);
	
	if (found != std::string::npos) {
		path.replace(found, target_process.length(), "steamclient.dylib");
	}
	
	steamClientModule = Sys_LoadModule(path.c_str());
#endif

	CreateInterfaceFn factory = Sys_GetFactory(steamClientModule);
	if (!factory)
		return;

	g_pSteamClient017 = (ISteamClient017 *)factory(STEAMCLIENT017_INTERFACE_VERSION, nullptr);
}
}

//------------------------------------------------------------------

/**
 * Initializes SteamAPI interfaces for GoldSrc. Called from ConnectTier2Libraries.
 */
void SteamAPI_InitForGoldSrc()
{
	if (g_bIsInited)
		return;

	g_bIsInited = true;

#ifdef SOURCE_SDK_MIN_STEAM_API
	LoadSteamAPIFuncsInRuntime();
#endif

	if (!SteamAPI_IsAvailable())
		return;

	LoadSteamClient017();
}

//------------------------------------------------------------------
// steam_api.h
//------------------------------------------------------------------
ISteamClient017 *SteamClient017()
{
    return g_pSteamClient017;
}

bool SteamAPI_IsAvailable()
{
	if (!g_bIsInited)
		SteamAPI_InitForGoldSrc();

#ifdef SOURCE_SDK_MIN_STEAM_API
	return g_bIsAvailable;
#else
	return true;
#endif
}

#ifdef SOURCE_SDK_MIN_STEAM_API
void S_CALLTYPE SteamAPI_RegisterCallResult(class CCallbackBase *pCallback, SteamAPICall_t hAPICall)
{
	Assert(SteamAPI_IsAvailable());
	g_pfnRegisterCallResult(pCallback, hAPICall);
}

void S_CALLTYPE SteamAPI_UnregisterCallResult(class CCallbackBase *pCallback, SteamAPICall_t hAPICall)
{
	Assert(SteamAPI_IsAvailable());
	g_pfnUnregisterCallResult(pCallback, hAPICall);
}
#endif
