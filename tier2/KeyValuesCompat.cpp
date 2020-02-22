#include "interface.h"
#include "vstdlib/IKeyValuesSystem.h"
#include "KeyValues.h"
#include "KeyValuesCompat.h"

//This is the VGUI2 version of the IKeyValuesSystem interface. It has additional methods that makes it incompatible. - Solokiller
class IKeyValues : public IBaseInterface
{
public:
	// registers the size of the KeyValues in the specified instance
	// so it can build a properly sized memory pool for the KeyValues objects
	// the sizes will usually never differ but this is for versioning safety
	virtual void RegisterSizeofKeyValues( int size ) = 0;

	// allocates/frees a KeyValues object from the shared mempool
	virtual void *AllocKeyValuesMemory( int size ) = 0;
	virtual void FreeKeyValuesMemory( void *pMem ) = 0;

	// symbol table access (used for key names)
	virtual HKeySymbol GetSymbolForString( const char *name ) = 0;
	virtual const char *GetStringForSymbol( HKeySymbol symbol ) = 0;

	//Used by GoldSource only. - Solokiller
	virtual void GetLocalizedFromANSI( const char* ansi, wchar_t* outBuf, int unicodeBufferSizeInBytes ) = 0;

	virtual void GetANSIFromLocalized( const wchar_t* wchar, char* outBuf, int ansiBufferSizeInBytes ) = 0;

	// for debugging, adds KeyValues record into global list so we can track memory leaks
	virtual void AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name ) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList( void *pMem ) = 0;
};

#define IKEYVALUES_INTERFACE_VERSION "KeyValues003"

IKeyValues* g_pKeyValuesInterface = NULL;

class CKeyValuesWrapper : public IKeyValuesSystem
{
public:
	void RegisterSizeofKeyValues( int size ) override
	{
		return g_pKeyValuesInterface->RegisterSizeofKeyValues( size );
	}

	void *AllocKeyValuesMemory( int size ) override
	{
		return g_pKeyValuesInterface->AllocKeyValuesMemory( size );
	}

	void FreeKeyValuesMemory( void *pMem ) override
	{
		g_pKeyValuesInterface->FreeKeyValuesMemory( pMem );
	}

	HKeySymbol GetSymbolForString( const char *name ) override
	{
		return g_pKeyValuesInterface->GetSymbolForString( name );
	}

	const char *GetStringForSymbol( HKeySymbol symbol ) override
	{
		return g_pKeyValuesInterface->GetStringForSymbol( symbol );
	}

	void AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name ) override
	{
		g_pKeyValuesInterface->AddKeyValuesToMemoryLeakList( pMem, name );
	}

	void RemoveKeyValuesFromMemoryLeakList( void *pMem ) override
	{
		g_pKeyValuesInterface->RemoveKeyValuesFromMemoryLeakList( pMem );
	}
};

CKeyValuesWrapper g_KeyValuesSystem;

IKeyValuesSystem *keyvalues()
{
	return &g_KeyValuesSystem;
}

bool KV_InitKeyValuesSystem( CreateInterfaceFn* pFactories, int iNumFactories )
{
	for (int i = 0; i < iNumFactories; ++i)
	{
		if (!g_pKeyValuesInterface)
		{
			g_pKeyValuesInterface = (IKeyValues *)pFactories[i](IKEYVALUES_INTERFACE_VERSION, NULL);
		}
	}

	if( !g_pKeyValuesInterface )
		return false;

	g_pKeyValuesInterface->RegisterSizeofKeyValues( sizeof( KeyValues ) );

	return true;
}
