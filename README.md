Source SDK for Half-Life 1
==========================

This repository contains parts of Source SDK 2013 ported to be used with
Half-Life 1 (GoldSource) engine. Specifically:

- *vstdlib* and *tier0* (headers) - platform abstraction layer.
- *tier1* - Valve's template library, KeyValues and DLL interface.
- *tier2* - GoldSrc engine linking library. Exports various engine interfaces.
- *vgui_controls* - VGUI2 elements.
- *steam_api* - Steam API. HL1 since SteamPipe update uses API version identical to Source 2013.


You can find engine interfaces in *tier2/tier2.h*:

- *IFileSystem* - file system.
- *IBaseUI* - exported by the engine to the client. Allows to show and hide GameUI and console from client's code.
- *IGameUIFuncs* - provides some info about the game.
- *IEngineVGui* - provides access to root panels (see hierarchy in VGUI2 docs).
- *IClientVGUI* - exported by the client. Used to initialize and control VGUI2 in the client library.

Documentation
-------------

Documentation can be found on [Valve Developer Community](https://developer.valvesoftware.com/):

- [tier1](https://developer.valvesoftware.com/wiki/Tier1)
- [VGUI2 basics](https://developer.valvesoftware.com/wiki/VGUI_Documentation)
- [VGUI2 category on VDC](https://developer.valvesoftware.com/wiki/Category:VGUI)
- [KeyValues](https://developer.valvesoftware.com/wiki/KeyValues)


Credits
-------

- Valve Software (Source SDK)
- SamVanheer (GoldSrc engine reverse engineering).

Integrating into your mod
-------------------------
0. Your mod must use CMake.
1. Add this repo as a **submodule**.
2. In your main CMakeLists.txt:
   ```cmake
   # source_sdk is the directory you cloned this repo to.
   add_subdirectory( source_sdk )
   ```
3. The CMakeLists.txt of this project sets following variables in the parent scope:
   - *SOURCE_SDK_DEFINES* - Add it to client's compile definitions.
   - *SOURCE_SDK_INCLUDE_PATHS* - Add it to client's include paths.
   - *SOURCE_SDK_LIBS* - Link the client library with these libs.
4. Implement IClientVGUI. Basic init method should look like this:
   ```cpp
	#include <tier0/dbg.h>
	#include <tier1/interface.h>
    #include <tier1/tier1.h>
    #include <tier2/tier2.h>
	
	void CClientVGUI::Initialize(CreateInterfaceFn *pFactories, int iNumFactories)
	{
		ConnectTier1Libraries(pFactories, iNumFactories);
		ConnectTier2Libraries(pFactories, iNumFactories);

		if (!vgui2::VGui_InitInterfacesList("CLIENT", pFactories, iNumFactories))
		{
			Error("Failed to intialize VGUI2\n");
			Assert(false);
		}
	}
   ```
   You can now use VGUI2 controls but you should probably create a viewport.
   Look at *source-sdk-2013/mp/src/game/client/game_controls/baseviewport.h*
   and implement something similar.
5. Have fun! :D
