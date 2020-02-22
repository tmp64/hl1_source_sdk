//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined( IENGINEVGUI_H )
#define IENGINEVGUI_H

#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include "vgui/VGUI2.h"

// Forward declarations.
namespace vgui
{
	class Panel;
};

enum VGuiPanel_t
{
	PANEL_ROOT = 0,
	PANEL_CLIENTDLL,
	PANEL_GAMEUIDLL
};

// In-game panels are cropped to the current engine viewport size
enum PaintMode_t
{
	PAINT_UIPANELS		= (1<<0),
	PAINT_INGAMEPANELS  = (1<<1),
};

class IEngineVGui : public IBaseInterface
{
public:
	virtual					~IEngineVGui( void ) { }

	virtual vgui2::VPANEL	GetPanel( VGuiPanel_t type ) = 0;
};

#define VENGINE_VGUI_VERSION	"VEngineVGui001"

#endif // IENGINEVGUI_H
