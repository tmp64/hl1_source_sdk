//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdarg.h>
#include <stdio.h>

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <KeyValues.h>

#include <vgui_controls/Image.h>
#include <vgui_controls/CheckButton.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui2;

void CheckImage::Paint()
{
#ifdef _WIN32
	DrawSetTextFont(GetFont());

	// draw background
	if (_CheckButton->IsEnabled() && _CheckButton->IsCheckButtonCheckable() )
	{
		DrawSetTextColor(_bgColor);
	}
	else
	{
		DrawSetTextColor(_CheckButton->GetDisabledBgColor());
	}
	DrawPrintChar(0, 1, 'g');

	// draw border box
	DrawSetTextColor(_borderColor1);
	DrawPrintChar(0, 1, 'e');
	DrawSetTextColor(_borderColor2);
	DrawPrintChar(0, 1, 'f');

	// draw selected check
	if (_CheckButton->IsSelected())
	{
		if ( !_CheckButton->IsEnabled() )
		{
			DrawSetTextColor( _CheckButton->GetDisabledFgColor() );
		}
		else
		{
			DrawSetTextColor(_checkColor);
		}

		DrawPrintChar(0, 2, 'b');
	}
#else
	// GoldSrc: On Linux "Marlett" symbol font isn't rendered at all.
	// GoldSrc: Draw the checkbox using primitives
	auto fnGetPoportional = [&](int x) {
		if (_CheckButton->IsProportional())
			return scheme()->GetProportionalScaledValue(x);
		else
			return x;
	};

	int X_OFF = fnGetPoportional(0);
	int Y_OFF = fnGetPoportional(2);
	int BORDER_SIZE = fnGetPoportional(12);
	int BORDER_THICK = fnGetPoportional(1);
	int CHECK_SIZE = fnGetPoportional(6);

	// draw background
	if (_CheckButton->IsEnabled() && _CheckButton->IsCheckButtonCheckable() )
	{
		DrawSetColor(_bgColor);
	}
	else
	{
		DrawSetColor(_CheckButton->GetDisabledBgColor());
	}

	DrawFilledRect(X_OFF + BORDER_THICK, Y_OFF + BORDER_THICK, X_OFF + BORDER_SIZE - BORDER_THICK, Y_OFF + BORDER_SIZE - BORDER_THICK);

	// draw border box
	DrawSetColor(_borderColor1);
	DrawFilledRect(X_OFF, Y_OFF, X_OFF + BORDER_SIZE - BORDER_THICK, Y_OFF + BORDER_THICK);	// Top line
	DrawFilledRect(X_OFF, Y_OFF + BORDER_THICK, X_OFF + BORDER_THICK, Y_OFF + BORDER_SIZE);	// Left line

	DrawSetColor(_borderColor2);
	DrawFilledRect(X_OFF + BORDER_SIZE - BORDER_THICK, Y_OFF, X_OFF + BORDER_SIZE, Y_OFF + BORDER_SIZE - BORDER_THICK);	// Right line
	DrawFilledRect(X_OFF + BORDER_THICK, Y_OFF + BORDER_SIZE - BORDER_THICK, X_OFF + BORDER_SIZE, Y_OFF + BORDER_SIZE);	// Bottom line

	// draw selected check
	if (_CheckButton->IsSelected())
	{
		if ( !_CheckButton->IsEnabled() )
		{
			DrawSetColor( _CheckButton->GetDisabledFgColor() );
		}
		else
		{
			DrawSetColor(_checkColor);
		}

		int off = (BORDER_SIZE - CHECK_SIZE) / 2;
		DrawFilledRect(X_OFF + off, Y_OFF + off, X_OFF + BORDER_SIZE - off, Y_OFF + BORDER_SIZE - off);
	}
#endif
}

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CheckButton, CheckButton );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CheckButton::CheckButton(Panel *parent, const char *panelName, const char *text) : ToggleButton(parent, panelName, text)
{
 	SetContentAlignment(a_west);
	SetButtonActivationType(ACTIVATE_ONPRESSED);
	m_bCheckButtonCheckable = true;
	m_bStaySelectedOnClick = true;

	// create the image
	_checkBoxImage = new CheckImage(this);

	SetTextImageIndex(1);
	SetImageAtIndex(0, _checkBoxImage, CHECK_INSET);

	_selectedFgColor = Color( 196, 181, 80, 255 );
	_disabledFgColor = Color(130, 130, 130, 255);
	_disabledBgColor = Color(62, 70, 55, 255);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CheckButton::~CheckButton()
{
	delete _checkBoxImage;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CheckButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetDefaultColor( GetSchemeColor("CheckButton.TextColor", pScheme), GetBgColor() );
	_checkBoxImage->_bgColor = GetSchemeColor("CheckButton.BgColor", Color(62, 70, 55, 255), pScheme);
	_checkBoxImage->_borderColor1 = GetSchemeColor("CheckButton.Border1", Color(20, 20, 20, 255), pScheme);
	_checkBoxImage->_borderColor2 = GetSchemeColor("CheckButton.Border2", Color(90, 90, 90, 255), pScheme);
	_checkBoxImage->_checkColor = GetSchemeColor("CheckButton.Check", Color(20, 20, 20, 255), pScheme);
	_selectedFgColor = GetSchemeColor("CheckButton.SelectedTextColor", GetSchemeColor("ControlText", pScheme), pScheme);
	_disabledFgColor = GetSchemeColor("CheckButton.DisabledFgColor", Color(130, 130, 130, 255), pScheme);
	_disabledBgColor = GetSchemeColor("CheckButton.DisabledBgColor", Color(62, 70, 55, 255), pScheme);

	Color bgArmedColor = GetSchemeColor( "CheckButton.ArmedBgColor", Color(62, 70, 55, 255), pScheme); 
	SetArmedColor( GetFgColor(), bgArmedColor );

	Color bgDepressedColor = GetSchemeColor( "CheckButton.DepressedBgColor", Color(62, 70, 55, 255), pScheme); 
	SetDepressedColor( GetFgColor(), bgDepressedColor );

	_highlightFgColor = GetSchemeColor( "CheckButton.HighlightFgColor", Color(62, 70, 55, 255), pScheme); 

	SetContentAlignment(Label::a_west);

	_checkBoxImage->SetFont( pScheme->GetFont("Marlett", IsProportional()) );
	_checkBoxImage->ResizeImageToContent();
	SetImageAtIndex(0, _checkBoxImage, CHECK_INSET);

	// don't draw a background
	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IBorder *CheckButton::GetBorder(bool depressed, bool armed, bool selected, bool keyfocus)
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Check the button
//-----------------------------------------------------------------------------
void CheckButton::SetSelected(bool state )
{
	if (m_bCheckButtonCheckable)
	{
		// send a message saying we've been checked
		KeyValues *msg = new KeyValues("CheckButtonChecked", "state", (int)state);
		PostActionSignal(msg);
		
		BaseClass::SetSelected(state);
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets whether or not the state of the check can be changed
//-----------------------------------------------------------------------------
void CheckButton::SetCheckButtonCheckable(bool state)
{
	m_bCheckButtonCheckable = state;
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Gets a different foreground text color if we are selected
//-----------------------------------------------------------------------------
#ifdef _X360
Color CheckButton::GetButtonFgColor()
{
	if (HasFocus())
	{
		return _selectedFgColor;
	}

	return BaseClass::GetButtonFgColor();
}
#else
Color CheckButton::GetButtonFgColor()
{
	if ( IsArmed() )
	{
		return _highlightFgColor;
	}

	if (IsSelected())
	{
		return _selectedFgColor;
	}

	return BaseClass::GetButtonFgColor();
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CheckButton::OnCheckButtonChecked(Panel *panel)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CheckButton::SetHighlightColor(Color fgColor)
{
	if ( _highlightFgColor != fgColor )
	{
		_highlightFgColor = fgColor;

		InvalidateLayout(false);
	}
}

