//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A higher level link library for general use in the game and tools.
//
//===========================================================================//

#include <tier1/tier1.h>
#include "tier0/dbg.h"


//-----------------------------------------------------------------------------
// These tier1 libraries must be set by any users of this library.
// They can be set by calling ConnectTier1Libraries or InitDefaultFileSystem.
// It is hoped that setting this, and using this library will be the common mechanism for
// allowing link libraries to access tier1 library interfaces
//-----------------------------------------------------------------------------
static bool s_bConnected = false;

// for utlsortvector.h
#ifndef _WIN32
	void *g_pUtlSortVectorQSortContext = NULL;
#endif


//-----------------------------------------------------------------------------
// Call this to connect to all tier 1 libraries.
// It's up to the caller to check the globals it cares about to see if ones are missing
//-----------------------------------------------------------------------------
void ConnectTier1Libraries( CreateInterfaceFn *pFactoryList, int nFactoryCount )
{
	// Don't connect twice..
	if ( s_bConnected )
		return;

	s_bConnected = true;

	for ( int i = 0; i < nFactoryCount; ++i )
	{
		
	}
}

void DisconnectTier1Libraries()
{
	if ( !s_bConnected )
		return;

	s_bConnected = false;
}
