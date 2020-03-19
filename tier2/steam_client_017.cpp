#define VERSION_SAFE_STEAM_API_INTERFACES
#include <tier1/interface.h>
#include <steam/steam_api.h>

static ISteamClient017 *s_SteamClient017 = nullptr;

ISteamClient017 *SteamClient017()
{
    return s_SteamClient017;
}

void InitSteamClient017()
{
    // steamclient.dll should be loaded by steam_api
    CSysModule *steamClientModule = Sys_LoadModule("steamclient" DLL_EXT_STRING);
    if (!steamClientModule)
        return;

    CreateInterfaceFn factory = Sys_GetFactory(steamClientModule);
    if (!factory)
    {
        return;
    }
    s_SteamClient017 = (ISteamClient017 *)factory(STEAMCLIENT017_INTERFACE_VERSION, nullptr);
}