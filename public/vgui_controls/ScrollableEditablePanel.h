//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef SCROLLABLEEDITABLEPANEL_H
#define SCROLLABLEEDITABLEPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/EditablePanel.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
namespace vgui2
{
	class ScrollBar;
}

namespace vgui2
{

//-----------------------------------------------------------------------------
// An editable panel that has a scrollbar
//-----------------------------------------------------------------------------
class ScrollableEditablePanel : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE( ScrollableEditablePanel, vgui2::EditablePanel );

public:
	ScrollableEditablePanel( vgui2::Panel *pParent, vgui2::EditablePanel *pChild, const char *pName );
	virtual ~ScrollableEditablePanel() {}

	virtual void ApplySettings( KeyValues *pInResourceData );
	virtual void PerformLayout();

	vgui2::ScrollBar	*GetScrollbar( void ) { return m_pScrollBar; }

	MESSAGE_FUNC( OnScrollBarSliderMoved, "ScrollBarSliderMoved" );
	virtual void OnMouseWheeled(int delta);	// respond to mouse wheel events

private:
	vgui2::ScrollBar *m_pScrollBar;
	vgui2::EditablePanel *m_pChild;
};


} // end namespace vgui2

#endif // SCROLLABLEEDITABLEPANEL_H