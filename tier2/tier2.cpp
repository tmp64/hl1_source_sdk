#define TIER2_GAMEUI_INTERNALS

#include <tier2/tier2.h>
#include <FileSystem.h>
#include <IBaseUI.h>
#include <IEngineVGui.h>
#include <IGameUIFuncs.h>
#include <IGameConsole.h>
#include <IGameUI.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInputInternal.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include "KeyValuesCompat.h"

IFileSystem *g_pFullFileSystem;
IBaseUI *g_pBaseUI;
IEngineVGui *g_pEngineVGui;
IGameUIFuncs *g_pGameUIFuncs;
IGameConsole *g_pGameConsole;
IGameUI *g_pGameUI;
vgui2::ISurface *g_pVGuiSurface;
vgui2::IInputInternal *g_pVGuiInput;
vgui2::IVGui *g_pVGui;
vgui2::IPanel *g_pVGuiPanel;
vgui2::ILocalize *g_pVGuiLocalize;
vgui2::ISchemeManager *g_pVGuiSchemeManager;
vgui2::ISystem *g_pVGuiSystem;

static bool s_bConnected = false;
void SteamAPI_InitForGoldSrc();

void ConnectTier2Libraries(CreateInterfaceFn *pFactoryList, int nFactoryCount)
{
	if (s_bConnected)
		return;

	s_bConnected = true;

	if (!KV_InitKeyValuesSystem(pFactoryList, nFactoryCount))
	{
		Error("tier2: Failed to initialize KeyValues\n");
		Assert(false);
	}

	SteamAPI_InitForGoldSrc();

	for (int i = 0; i < nFactoryCount; ++i)
	{
		if (!g_pFullFileSystem)
		{
			g_pFullFileSystem = (IFileSystem *)pFactoryList[i](FILESYSTEM_INTERFACE_VERSION, NULL);
		}
		if (!g_pBaseUI)
		{
			g_pBaseUI = (IBaseUI *)pFactoryList[i](IBASEUI_NAME, NULL);
		}
		if (!g_pEngineVGui)
		{
			g_pEngineVGui = (IEngineVGui *)pFactoryList[i](VENGINE_VGUI_VERSION, NULL);
		}
		if (!g_pGameUIFuncs)
		{
			g_pGameUIFuncs = (IGameUIFuncs *)pFactoryList[i](IGAMEUIFUNCS_NAME, NULL);
		}
		if (!g_pGameConsole)
		{
			g_pGameConsole = (IGameConsole *)pFactoryList[i](GAMECONSOLE_INTERFACE_VERSION_GS, NULL);
		}
		if (!g_pGameUI)
		{
			g_pGameUI = (IGameUI *)pFactoryList[i](GAMEUI_INTERFACE_VERSION_GS, NULL);
		}
		if (!g_pVGuiSurface)
		{
			g_pVGuiSurface = (vgui2::ISurface *)pFactoryList[i](VGUI_SURFACE_INTERFACE_VERSION_GS, NULL);
		}
		if (!g_pVGuiInput)
		{
			g_pVGuiInput = (vgui2::IInputInternal *)pFactoryList[i](VGUI_INPUTINTERNAL_INTERFACE_VERSION, NULL);
		}
		if (!g_pVGui)
		{
			g_pVGui = (vgui2::IVGui *)pFactoryList[i](VGUI_IVGUI_INTERFACE_VERSION_GS, NULL);
		}
		if (!g_pVGuiPanel)
		{
			g_pVGuiPanel = (vgui2::IPanel *)pFactoryList[i](VGUI_PANEL_INTERFACE_VERSION_GS, NULL);
		}
		if (!g_pVGuiLocalize)
		{
			g_pVGuiLocalize = (vgui2::ILocalize *)pFactoryList[i](VGUI_LOCALIZE_INTERFACE_VERSION, NULL);
		}
		if (!g_pVGuiSchemeManager)
		{
			g_pVGuiSchemeManager = (vgui2::ISchemeManager *)pFactoryList[i](VGUI_SCHEME_INTERFACE_VERSION_GS, NULL);
		}
		if (!g_pVGuiSystem)
		{
			g_pVGuiSystem = (vgui2::ISystem *)pFactoryList[i](VGUI_SYSTEM_INTERFACE_VERSION_GS, NULL);
		}
	}
}

void DisconnectTier2Libraries()
{
	if (!s_bConnected)
		return;

	s_bConnected = false;
}
