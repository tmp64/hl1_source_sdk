set( TIER2_HEADER_FILES
	${SOURCE_SDK_ROOT}/public/tier2/tier2.h
	KeyValuesCompat.h
)

set( TIER2_SOURCE_FILES
	tier2.cpp
	KeyValuesCompat.cpp
)

add_library( tier2 STATIC
	${TIER2_HEADER_FILES}
	${TIER2_SOURCE_FILES}
)

target_include_directories( tier2 PRIVATE
	${SOURCE_SDK_INCLUDE_PATHS}
)

target_compile_definitions( tier2 PRIVATE
	${SOURCE_SDK_DEFINES}
)

set_target_properties( tier2 PROPERTIES
	COMPILE_FLAGS "${SOURCE_SDK_WARNING_LEVEL}" 
)
