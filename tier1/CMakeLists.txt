set( TIER1_HEADER_FILES
	${SOURCE_SDK_ROOT}/public/tier1/bitbuf.h
	${SOURCE_SDK_ROOT}/public/tier1/byteswap.h
	${SOURCE_SDK_ROOT}/public/tier1/callqueue.h
	${SOURCE_SDK_ROOT}/public/tier1/characterset.h
	${SOURCE_SDK_ROOT}/public/tier1/checksum_crc.h
	${SOURCE_SDK_ROOT}/public/tier1/checksum_md5.h
	${SOURCE_SDK_ROOT}/public/tier1/checksum_sha1.h
	#${SOURCE_SDK_ROOT}/public/tier1/CommandBuffer.h
	#${SOURCE_SDK_ROOT}/public/tier1/convar.h
	${SOURCE_SDK_ROOT}/public/tier1/datamanager.h
	${SOURCE_SDK_ROOT}/public/datamap.h
	${SOURCE_SDK_ROOT}/public/tier1/delegates.h
	${SOURCE_SDK_ROOT}/public/tier1/diff.h
	${SOURCE_SDK_ROOT}/public/tier1/fmtstr.h
	${SOURCE_SDK_ROOT}/public/tier1/functors.h
	${SOURCE_SDK_ROOT}/public/tier1/generichash.h
	#${SOURCE_SDK_ROOT}/public/tier1/iconvar.h
	${SOURCE_SDK_ROOT}/public/tier1/interface.h
	${SOURCE_SDK_ROOT}/public/tier1/KeyValues.h
	${SOURCE_SDK_ROOT}/public/tier1/kvpacker.h
	${SOURCE_SDK_ROOT}/public/tier1/lzmaDecoder.h
	${SOURCE_SDK_ROOT}/public/tier1/lzss.h
	${SOURCE_SDK_ROOT}/public/tier1/mempool.h
	${SOURCE_SDK_ROOT}/public/tier1/memstack.h
	${SOURCE_SDK_ROOT}/public/tier1/netadr.h
	${SOURCE_SDK_ROOT}/public/tier1/processor_detect.h
	${SOURCE_SDK_ROOT}/public/tier1/rangecheckedvar.h
	${SOURCE_SDK_ROOT}/public/tier1/refcount.h
	${SOURCE_SDK_ROOT}/public/tier1/smartptr.h
	${SOURCE_SDK_ROOT}/public/tier1/snappy.h
	${SOURCE_SDK_ROOT}/public/tier1/snappy-sinksource.h
	${SOURCE_SDK_ROOT}/public/tier1/stringpool.h
	${SOURCE_SDK_ROOT}/public/tier1/strtools.h
	${SOURCE_SDK_ROOT}/public/tier1/tier1.h
	${SOURCE_SDK_ROOT}/public/tier1/tokenreader.h
	${SOURCE_SDK_ROOT}/public/tier1/utlbidirectionalset.h
	${SOURCE_SDK_ROOT}/public/tier1/utlblockmemory.h
	${SOURCE_SDK_ROOT}/public/tier1/utlbuffer.h
	${SOURCE_SDK_ROOT}/public/tier1/utlbufferutil.h
	${SOURCE_SDK_ROOT}/public/tier1/utlcommon.h
	${SOURCE_SDK_ROOT}/public/tier1/utldict.h
	${SOURCE_SDK_ROOT}/public/tier1/utlenvelope.h
	${SOURCE_SDK_ROOT}/public/tier1/utlfixedmemory.h
	${SOURCE_SDK_ROOT}/public/tier1/utlhandletable.h
	${SOURCE_SDK_ROOT}/public/tier1/utlhash.h
	${SOURCE_SDK_ROOT}/public/tier1/utlhashtable.h
	${SOURCE_SDK_ROOT}/public/tier1/utllinkedlist.h
	${SOURCE_SDK_ROOT}/public/tier1/utlmap.h
	${SOURCE_SDK_ROOT}/public/tier1/utlmemory.h
	${SOURCE_SDK_ROOT}/public/tier1/utlmultilist.h
	${SOURCE_SDK_ROOT}/public/tier1/utlpriorityqueue.h
	${SOURCE_SDK_ROOT}/public/tier1/utlqueue.h
	${SOURCE_SDK_ROOT}/public/tier1/utlrbtree.h
	${SOURCE_SDK_ROOT}/public/tier1/UtlSortVector.h
	${SOURCE_SDK_ROOT}/public/tier1/utlstack.h
	${SOURCE_SDK_ROOT}/public/tier1/utlstring.h
	${SOURCE_SDK_ROOT}/public/tier1/UtlStringMap.h
	${SOURCE_SDK_ROOT}/public/tier1/utlsymbol.h
	${SOURCE_SDK_ROOT}/public/tier1/utlsymbollarge.h
	${SOURCE_SDK_ROOT}/public/tier1/utlvector.h
	${SOURCE_SDK_ROOT}/public/tier1/utlbinaryblock.h
)

set( TIER1_SOURCE_FILES
	bitbuf.cpp
	newbitbuf.cpp
	byteswap.cpp
	characterset.cpp
	checksum_crc.cpp
	checksum_md5.cpp
	checksum_sha1.cpp
	#commandbuffer.cpp	Requires ICVar
	#convar.cpp			Requires ICVar
	datamanager.cpp
	diff.cpp
	generichash.cpp
	#ilocalize.cpp		Requires ILocalize
	interface.cpp
	KeyValues.cpp
	kvpacker.cpp
	mempool.cpp
	memstack.cpp
	NetAdr.cpp
	splitstring.cpp
	rangecheckedvar.cpp
	reliabletimer.cpp
	stringpool.cpp
	strtools.cpp
	strtools_unicode.cpp
	tier1.cpp
	tokenreader.cpp
	sparsematrix.cpp
	uniqueid.cpp
	utlbuffer.cpp
	utlbufferutil.cpp
	utlstring.cpp
	utlsymbol.cpp
	utlbinaryblock.cpp
)

if( WIN32 )
	set( TIER1_HEADER_FILES
		${TIER1_HEADER_FILES}
		${SOURCE_SDK_ROOT}/common/xbox/xboxstubs.h
		${SOURCE_SDK_ROOT}/public/tier1/uniqueid.h
	)
	
	set( TIER1_SOURCE_FILES
		${TIER1_SOURCE_FILES}
		processor_detect.cpp
	)
else()
	set( TIER1_SOURCE_FILES
		${TIER1_SOURCE_FILES}
		processor_detect_linux.cpp
		pathmatch.cpp
		qsort_s.cpp
	)
endif()

add_library( tier1 STATIC
	${TIER1_HEADER_FILES}
	${TIER1_SOURCE_FILES}
)

source_sdk_check_winxp_support( tier1 )

target_include_directories( tier1 PRIVATE
	${SOURCE_SDK_INCLUDE_PATHS}
)

target_compile_definitions( tier1 PRIVATE
	${SOURCE_SDK_DEFINES}
	TIER1_STATIC_LIB
)

set_target_properties( tier1 PROPERTIES
	COMPILE_FLAGS "${SOURCE_SDK_WARNING_LEVEL}" 
)
