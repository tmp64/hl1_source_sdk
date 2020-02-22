#ifndef ENGINE_IGAMEUIFUNCS_H
#define ENGINE_IGAMEUIFUNCS_H

#include "interface.h"

#include "vgui/IInput.h"

/**
*	Represents a single video mode.
*/
struct vmode_t
{
	int iWidth;
	int iHeight;
	int iBPP;
};

/**
*	Provides a number of GameUI functions.
*/
class IGameUIFuncs : public IBaseInterface
{
public:

	/**
	*	Gets whether a key is down or not.
	*	@param pszKeyName Key whose state should be queried.
	*	@param bIsDown If the key exists, whether the key is down or not.
	*	@return Whether the given key exists or not.
	*/
	virtual bool IsKeyDown( const char* pszKeyName, bool& bIsDown ) = 0;

	/**
	*	@param iKeyNum Key ID.
	*	@return The name of the given key code.
	*/
	virtual const char* Key_NameForKey( vgui2::KeyCode iKeyNum ) = 0;

	/**
	*	@param iKeyNum Key code.
	*	@return String that is executed when the key is pressed, or an empty string if it isn't bound.
	*/
	virtual const char* Key_BindingForKey( vgui2::KeyCode iKeyNum ) = 0;

	/**
	*	@param pszBind Binding to look up the key for.
	*	@return Key code of the key that is bound to the binding.
	*/
	virtual vgui2::KeyCode GetVGUI2KeyCodeForBind( const char* pszBind ) = 0;

	/**
	*	Gets the array of video modes. The array is guaranteed to remain in memory up until shutdown.
	*	@param ppModes Pointer to pointer to an array of modes.
	*	@param piNumModes Number of modes stored in the array.
	*/
	virtual void GetVideoModes( vmode_t** ppModes, int* piNumModes ) = 0;

	/**
	*	Gets the current video mode.
	*	@param piWidth Width of the screen.
	*	@param piHeight Height of the screen.
	*	@param piBPP Bits Per Pixel.
	*/
	virtual void GetCurrentVideoMode( int* piWidth, int* piHeight, int* piBPP ) = 0;

	/**
	*	Gets the current renderer's information.
	*	@param pszName Name of the renderer.
	*	@param iNameLen Size of pszName in bytes.
	*	@param piWindowed Whether the game is currently in windowed mode.
	*	@param piHDModels Whether the game checks the _hd directory.
	*	@param piAddonsFolder Whether the game checks the _addon directory.
	*	@param piVidLevel Video level. Affects window scaling and anti aliasing.
	*/
	virtual void GetCurrentRenderer( char* pszName, int iNameLen, 
									 int* piWindowed, int* piHDModels, 
									 int* piAddonsFolder, int* piVidLevel ) = 0;

	/**
	*	@return Whether the client is currently connected to a VAC secure server.
	*/
	virtual bool IsConnectedToVACSecureServer() = 0;

	/**
	*	@return Key code of a given key name. Returns -1 if the key doesn't exist.
	*/
	virtual vgui2::KeyCode Key_KeyStringToKeyNum( const char* pszKeyName ) = 0;
};

/**
*	GameUI funcs name.
*/
#define IGAMEUIFUNCS_NAME "VENGINE_GAMEUIFUNCS_VERSION001"

#endif //ENGINE_IGAMEUIFUNCS_H
