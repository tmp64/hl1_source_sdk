cmake_minimum_required( VERSION 3.5.2 )

project( HL1_SourceSDK )


set( SOURCE_SDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR} )

# Add "cmake" to module path
set( CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${SOURCE_SDK_ROOT}/cmake
)

# DLL extension macro
if( WIN32 )
	set( DLL_EXT_MACRO ".dll" )
else()
	set( DLL_EXT_MACRO ".so" )
endif()

# Steam API
if( SOURCE_SDK_NO_STEAM )
	set( STEAM_API_DEFINES
		NO_STEAM
	)
else()
	find_package( steam_api MODULE REQUIRED )
	set( STEAM_API_LIBRARIES
		steam_api
	)
endif()

set( SOURCE_SDK_DEFINES
	${SOURCE_SDK_DEFINES}
	${STEAM_API_DEFINES}
	NO_MALLOC_OVERRIDE
	_DLL_EXT=${DLL_EXT_MACRO}
)

if( SOURCE_SDK_VGUI_NS_ALIAS )
	# `namespace vgui = vgui2` for compatibility with large parts of Source SDK
	set( SOURCE_SDK_DEFINES
		${SOURCE_SDK_DEFINES}
		SOURCE_SDK_VGUI_NS_ALIAS
	)
endif()

set( SOURCE_SDK_INCLUDE_PATHS
	${SOURCE_SDK_INCLUDE_PATHS}
	${CMAKE_CURRENT_SOURCE_DIR}/common
	${CMAKE_CURRENT_SOURCE_DIR}/public
	${CMAKE_CURRENT_SOURCE_DIR}/public/tier0
	${CMAKE_CURRENT_SOURCE_DIR}/public/tier1
)

set( SOURCE_SDK_WARNING_LEVEL "")

#-----------------------------------------------------------------
# Dynamic libraries from the engine
#-----------------------------------------------------------------
find_package( tier0 MODULE REQUIRED )
find_package( vstdlib MODULE REQUIRED )

#-----------------------------------------------------------------
# Static libraries
#-----------------------------------------------------------------
add_subdirectory( tier1 )
add_subdirectory( tier2 )
add_subdirectory( vgui2/vgui_controls )

#-----------------------------------------------------------------
# Source SDK exported variables
#-----------------------------------------------------------------
set( SOURCE_SDK_DEFINES ${SOURCE_SDK_DEFINES} PARENT_SCOPE )
set( SOURCE_SDK_INCLUDE_PATHS ${SOURCE_SDK_INCLUDE_PATHS} PARENT_SCOPE )

set( SOURCE_SDK_LIBS
	vgui_controls
	${STEAM_API_LIBRARIES}
	tier2
	tier1
	tier0
	vstdlib
	PARENT_SCOPE
)
