#define VERSION_SAFE_STEAM_API_INTERFACES
#include <tier0/dbg.h>
#include <tier1/interface.h>
#include <steam/steam_api.h>

#ifndef PLATFORM_WINDOWS
#include <dlfcn.h>
#endif

static ISteamClient017 *s_SteamClient017 = nullptr;

ISteamClient017 *SteamClient017()
{
    return s_SteamClient017;
}

void InitSteamClient017()
{
	if (s_SteamClient017)
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
    {
        return;
    }
    s_SteamClient017 = (ISteamClient017 *)factory(STEAMCLIENT017_INTERFACE_VERSION, nullptr);
}