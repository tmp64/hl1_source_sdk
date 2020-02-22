#
# Finds Steam API library
#

find_library( STEAMAPI_LIB NAMES steam_api.lib libsteam_api.so PATHS ${SOURCE_SDK_ROOT}/lib/public NO_DEFAULT_PATH )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( steam_api DEFAULT_MSG STEAMAPI_LIB )

if( STEAMAPI_LIB )
	add_library( steam_api SHARED IMPORTED GLOBAL )

	if( MSVC )
		set_property( TARGET steam_api PROPERTY IMPORTED_IMPLIB ${STEAMAPI_LIB} )
	else()
		set_property( TARGET steam_api PROPERTY IMPORTED_LOCATION ${STEAMAPI_LIB} )
	endif()
endif()

unset( STEAMAPI_LIB CACHE )
