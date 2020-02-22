#ifndef ENGINE_IBASEUI_H
#define ENGINE_IBASEUI_H

#include "interface.h"

struct cl_enginefunc_t;

/**
*	Interface to the engine's game UI controls.
*	Most of these are handled by the engine itself.
*/
class IBaseUI : public IBaseInterface
{
public:

	/**
	*	Called when this interface is first loaded. Initializes the game UI.
	*	@param pFactories Array of factories to use.
	*	@param iNumFactories Number of factories.
	*/
	virtual void Initialize( CreateInterfaceFn* pFactories, int iNumFactories ) = 0;

	/**
	*	Sets up the game UI.
	*/
	virtual void Start( cl_enginefunc_t* pEngfuncs, int iVersion ) = 0;

	/**
	*	Shuts down the UI. Unloads libraries used by the UI.
	*/
	virtual void Shutdown() = 0;

	/**
	*	Called when a key event occurs.
	*	@param down Whether the key was up or down.
	*	@param keynum Key number that was pressed or released.
	*	@param pszCurrentBinding The current binding for the key.
	*	@return Whether the event was handled.
	*/
	virtual int Key_Event( int down, int keynum, const char* pszCurrentBinding ) = 0;

	/**
	*	Calls the engine's surface app handler to handle the given SDL event.
	*	@param pEvent Event.
	*	@param pUserData User data associated with the event. Ignored.
	*/
	virtual void CallEngineSurfaceAppHandler( void* pEvent, void* pUserData ) = 0;

	/**
	*	Draw the section of the VGUI2 UI that is contained in the given rectangle.
	*/
	virtual void Paint( int x, int y, int right, int bottom ) = 0;

	/**
	*	Hides the game UI and activates the client UI.
	*/
	virtual void HideGameUI() = 0;

	/**
	*	Activates the game UI and hides the client UI.
	*/
	virtual void ActivateGameUI() = 0;

	/**
	*	Hides the game console.
	*/
	virtual void HideConsole() = 0;

	/**
	*	Shows the game console.
	*/
	virtual void ShowConsole() = 0;
};

/**
*	Interface version for IBaseUI.
*/
#define IBASEUI_NAME "BaseUI001"

#endif //ENGINE_IBASEUI_H
