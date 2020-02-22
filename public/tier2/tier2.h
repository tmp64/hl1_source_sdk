//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A link library for GoldSrc.
//
//===========================================================================//


#ifndef TIER2_H
#define TIER2_H

#if defined( _WIN32 )
#pragma once
#endif

#include "tier1/tier1.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IFileSystem;
class IBaseUI;
class IEngineVGui;
class IGameConsole;
class IGameUI;
class IGameUIFuncs;

namespace vgui2
{
	class ISurface;
	class IVGui;
	class IInputInternal;
	class IPanel;
	class ILocalize;
	class ISchemeManager;
	class ISystem;
}


//-----------------------------------------------------------------------------
// These tier2 libraries must be set by any users of this library.
// They can be set by calling ConnectTier2Libraries or InitDefaultFileSystem.
// It is hoped that setting this, and using this library will be the common mechanism for
// allowing link libraries to access tier2 library interfaces
//-----------------------------------------------------------------------------
extern IFileSystem *g_pFullFileSystem;
extern IBaseUI *g_pBaseUI;
extern IEngineVGui *g_pEngineVGui;
extern IGameUIFuncs *g_pGameUIFuncs;
extern vgui2::ISurface *g_pVGuiSurface;
extern vgui2::IInputInternal *g_pVGuiInput;
extern vgui2::IVGui *g_pVGui;
extern vgui2::IPanel *g_pVGuiPanel;
extern vgui2::ILocalize *g_pVGuiLocalize;
extern vgui2::ISchemeManager *g_pVGuiSchemeManager;
extern vgui2::ISystem *g_pVGuiSystem;

// These interfaces are exposed by GameUI.dll for the engine (hw.dll/sw.dll)
// They are not supposed to be called directly.
// Define TIER2_GAMEUI_INTERNALS if you really need them.
#ifdef TIER2_GAMEUI_INTERNALS
extern IGameConsole *g_pGameConsole;
extern IGameUI *g_pGameUI;
#endif


//-----------------------------------------------------------------------------
// Call this to connect to/disconnect from all tier 2 libraries.
// It's up to the caller to check the globals it cares about to see if ones are missing
//-----------------------------------------------------------------------------
void ConnectTier2Libraries( CreateInterfaceFn *pFactoryList, int nFactoryCount );
void DisconnectTier2Libraries();


#endif // TIER2_H

