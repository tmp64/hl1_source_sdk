#define VERSION_SAFE_STEAM_API_INTERFACES

#ifdef PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <tier0/dbg.h>
#include <tier1/interface.h>
#include <tier2/tier2.h>
#include <steam/steam_api.h>

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

	// steamclient.dll should be loaded by steam_api
#ifdef PLATFORM_WINDOWS
	CSysModule *steamClientModule = Sys_LoadModule("steamclient" DLL_EXT_STRING);
#else
	CSysModule *steamClientModule = reinterpret_cast<CSysModule *>(dlopen("steamclient" DLL_EXT_STRING, RTLD_NOW));
#endif

	if (!steamClientModule)
	{
#ifndef PLATFORM_WINDOWS
		Warning("Failed to load steamclient" DLL_EXT_STRING ": %s\n", dlerror());
#endif
		return;
	}

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
