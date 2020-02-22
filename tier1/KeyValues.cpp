//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifdef _XBOX
#include "xbox/xbox_platform.h"
#include "xbox/xbox_win32stubs.h"
#endif
#if defined(_WIN32) && !defined(_XBOX)
#define NOMINMAX
#include <windows.h>		// for WideCharToMultiByte and MultiByteToWideChar
#elif defined(POSIX)
#include <wchar.h> // wcslen()
#endif

#include <KeyValues.h>
#include "FileSystem.h"
#include <vstdlib/IKeyValuesSystem.h>

#include <Color.h>
#include <stdlib.h>
#include "tier0/dbg.h"
#include "tier0/mem.h"
#include "UtlVector.h"
#include "UtlBuffer.h"
//#include "common/MinMax.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//GCC complains about deleting KeyValues due to it having no virtual destructor - Solokiller
#ifdef POSIX
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif

static const char * s_LastFileLoadingFrom = "unknown"; // just needed for error messages

#define KEYVALUES_TOKEN_SIZE	1024
static char s_pTokenBuf[KEYVALUES_TOKEN_SIZE];


#define INTERNALWRITE( pData, len ) InternalWrite( filesystem, f, pBuf, pData, len )


// a simple class to keep track of a stack of valid parsed symbols
const int MAX_ERROR_STACK = 64;
class CKeyValuesErrorStack
{
public:
	CKeyValuesErrorStack() : m_pFilename("NULL"), m_errorIndex(0), m_maxErrorIndex(0) {}

	void SetFilename( const char *pFilename )
	{
		m_pFilename = pFilename;
		m_maxErrorIndex = 0;
	}

	// entering a new keyvalues block, save state for errors
	// Not save symbols instead of pointers because the pointers can move!
	int Push( int symName )
	{
		if ( m_errorIndex < MAX_ERROR_STACK )
		{
			m_errorStack[m_errorIndex] = symName;
		}
		m_errorIndex++;
		m_maxErrorIndex = max( m_maxErrorIndex, (m_errorIndex-1) );
		return m_errorIndex-1;
	}

	// exiting block, error isn't in this block, remove.
	void Pop()
	{
		m_errorIndex--;
		Assert(m_errorIndex>=0);
	}

	// Allows you to keep the same stack level, but change the name as you parse peers
	void Reset( int stackLevel, int symName )
	{
		Assert( stackLevel >= 0 && stackLevel < m_errorIndex );
		m_errorStack[stackLevel] = symName;
	}

	// Hit an error, report it and the parsing stack for context
	void ReportError( const char *pError )
	{
		Msg( "KeyValues Error: %s in file %s\n", pError, m_pFilename );
		for ( int i = 0; i < m_maxErrorIndex; i++ )
		{
			if ( m_errorStack[i] != INVALID_KEY_SYMBOL )
			{
				if ( i < m_errorIndex )
				{
					Msg( "%s, ", keyvalues()->GetStringForSymbol(m_errorStack[i]) );
				}
				else
				{
					Msg( "(*%s*), ", keyvalues()->GetStringForSymbol(m_errorStack[i]) );
				}
			}
		}
		Msg( "\n" );
	}

private:
	int		m_errorStack[MAX_ERROR_STACK];
	const char *m_pFilename;
	int		m_errorIndex;
	int		m_maxErrorIndex;
} g_KeyValuesErrorStack;


// a simple helper that creates stack entries as it goes in & out of scope
class CKeyErrorContext
{
public:
	CKeyErrorContext( KeyValues *pKv )
	{
		Init( pKv->GetNameSymbol() );
	}

	~CKeyErrorContext()
	{
		g_KeyValuesErrorStack.Pop();
	}
	CKeyErrorContext( int symName )
	{
		Init( symName );
	}
	void Reset( int symName )
	{
		g_KeyValuesErrorStack.Reset( m_stackLevel, symName );
	}
private:
	void Init( int symName )
	{
		m_stackLevel = g_KeyValuesErrorStack.Push( symName );
	}

	int m_stackLevel;
};

// Uncomment this line to hit the ~CLeakTrack assert to see what's looking like it's leaking
// #define LEAKTRACK

#ifdef LEAKTRACK

class CLeakTrack
{
public:
	CLeakTrack()
	{
	}
	~CLeakTrack()
	{
		if ( keys.Count() != 0 )
		{
			Assert( 0 );
		}
	}

	struct kve
	{
		KeyValues *kv;
		char		name[ 256 ];
	};

	void AddKv( KeyValues *kv, char const *name )
	{
		kve k;
		Q_strncpy( k.name, name ? name : "NULL", sizeof( k.name ) );
		k.kv = kv;

		keys.AddToTail( k );
	}

	void RemoveKv( KeyValues *kv )
	{
		int c = keys.Count();
		for ( int i = 0; i < c; i++ )
		{
			if ( keys[i].kv == kv )
			{
				keys.Remove( i );
				break;
			}
		}
	}

	CUtlVector< kve > keys;
};

static CLeakTrack track;

#define TRACK_KV_ADD( ptr, name )	track.AddKv( ptr, name )
#define TRACK_KV_REMOVE( ptr )		track.RemoveKv( ptr )

#else

#define TRACK_KV_ADD( ptr, name ) 
#define TRACK_KV_REMOVE( ptr )	

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName )
{
	TRACK_KV_ADD( this, setName );

	Init();
	SetName ( setName );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, const char *firstValue )
{
	TRACK_KV_ADD( this, setName );

	Init();
	SetName( setName );
	SetString( firstKey, firstValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, const wchar_t *firstValue )
{
	TRACK_KV_ADD( this, setName );

	Init();
	SetName( setName );
	SetWString( firstKey, firstValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, int firstValue )
{
	TRACK_KV_ADD( this, setName );

	Init();
	SetName( setName );
	SetInt( firstKey, firstValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, const char *firstValue, const char *secondKey, const char *secondValue )
{
	TRACK_KV_ADD( this, setName );

	Init();
	SetName( setName );
	SetString( firstKey, firstValue );
	SetString( secondKey, secondValue );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
KeyValues::KeyValues( const char *setName, const char *firstKey, int firstValue, const char *secondKey, int secondValue )
{
	TRACK_KV_ADD( this, setName );

	Init();
	SetName( setName );
	SetInt( firstKey, firstValue );
	SetInt( secondKey, secondValue );
}

//-----------------------------------------------------------------------------
// Purpose: Initialize member variables
//-----------------------------------------------------------------------------
void KeyValues::Init()
{
	m_iKeyName = INVALID_KEY_SYMBOL;
	m_iDataType = TYPE_NONE;

	m_pSub = NULL;
	m_pPeer = NULL;

	m_pValue = NULL;

	m_iAllocationSize = 0;
	
	//m_bHasEscapeSequences = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
KeyValues::~KeyValues()
{
	TRACK_KV_REMOVE( this );

	RemoveEverything();
}

//-----------------------------------------------------------------------------
// Purpose: remove everything
//-----------------------------------------------------------------------------
void KeyValues::RemoveEverything()
{
	KeyValues *dat;
	KeyValues *datNext = NULL;
	for ( dat = m_pSub; dat != NULL; dat = datNext )
	{
		datNext = dat->m_pPeer;
		dat->m_pPeer = NULL;
		delete dat;
	}

	for ( dat = m_pPeer; dat && dat != this; dat = datNext )
	{
		datNext = dat->m_pPeer;
		dat->m_pPeer = NULL;
		delete dat;
	}

	FreeAllocatedValue();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *f - 
//-----------------------------------------------------------------------------

void KeyValues::RecursiveSaveToFile( CUtlBuffer& buf, int indentLevel )
{
	RecursiveSaveToFile( NULL, FILESYSTEM_INVALID_HANDLE, &buf, indentLevel );
}

//-----------------------------------------------------------------------------
// Purpose: Get the name of the current key section
//-----------------------------------------------------------------------------
const char *KeyValues::GetName( void ) const
{
	return keyvalues()->GetStringForSymbol(m_iKeyName);
}

//-----------------------------------------------------------------------------
// Purpose: Get the symbol name of the current key section
//-----------------------------------------------------------------------------
int KeyValues::GetNameSymbol() const
{
	return m_iKeyName;
}


//-----------------------------------------------------------------------------
// Purpose: Read a single token from buffer (0 terminated)
//-----------------------------------------------------------------------------
const char *KeyValues::ReadToken( CUtlBuffer &buf, bool &wasQuoted, bool &wasConditional)
{
	wasQuoted = false;
	wasConditional = false;

	if ( !buf.IsValid() )
		return NULL; 

	// eating white spaces and remarks loop
	while ( true )
	{
		buf.EatWhiteSpace();
		if ( !buf.IsValid() )
			return NULL;	// file ends after reading whitespaces

		// stop if it's not a comment; a new token starts here
		if ( !buf.EatCPPComment() )
			break;
	}

	const char *c = (const char*)buf.PeekGet( sizeof(char), 0 );
	if ( !c )
		return NULL;

	// read quoted strings specially
	if ( *c == '\"' )
	{
		wasQuoted = true;
		buf.GetDelimitedString( /*m_bHasEscapeSequences ? GetCStringCharConversion() :*/ GetNoEscCharConversion(), 
			s_pTokenBuf, KEYVALUES_TOKEN_SIZE );
		return s_pTokenBuf;
	}

	if ( *c == '{' || *c == '}' )
	{
		// it's a control char, just add this one char and stop reading
		s_pTokenBuf[0] = *c;
		s_pTokenBuf[1] = 0;
		buf.SeekGet( CUtlBuffer::SEEK_CURRENT, 1 );
		return s_pTokenBuf;
	}

	// read in the token until we hit a whitespace or a control character
	bool bReportedError = false;
	bool bConditionalStart = false;
	int nCount = 0;
	while ( c = (const char*)buf.PeekGet( sizeof(char), 0 ) )
	{
		// end of file
		if ( *c == 0 )
			break;

		// break if any control character appears in non quoted tokens
		if ( *c == '"' || *c == '{' || *c == '}' )
			break;

		if ( *c == '[' )
			bConditionalStart = true;

		if ( *c == ']' && bConditionalStart )
		{
			wasConditional = true;
		}

		// break on whitespace
		if ( isspace(*c) )
			break;

		if (nCount < (KEYVALUES_TOKEN_SIZE-1) )
		{
			s_pTokenBuf[nCount++] = *c;	// add char to buffer
		}
		else if ( !bReportedError )
		{
			bReportedError = true;
			g_KeyValuesErrorStack.ReportError(" ReadToken overflow" );
		}

		buf.SeekGet( CUtlBuffer::SEEK_CURRENT, 1 );
	}
	s_pTokenBuf[ nCount ] = 0;
	return s_pTokenBuf;
}

//-----------------------------------------------------------------------------
// Purpose: Load keyValues from disk
//-----------------------------------------------------------------------------
bool KeyValues::LoadFromFile( IBaseFileSystem *filesystem, const char *resourceName, const char *pathID )
{
	Assert(filesystem);
	Assert( IsX360() || ( IsPC() && _heapchk() == _HEAPOK ) );

	FileHandle_t f = filesystem->Open(resourceName, "rb", pathID);
	if (!f)
		return false;

	s_LastFileLoadingFrom = resourceName;

	// load file into a null-terminated buffer
	int fileSize = filesystem->Size(f);

	//New code:
	unsigned bufSize = fileSize + 1;

	char *buffer = new char[ bufSize ];

	Assert( buffer );

	( ( IFileSystem * ) filesystem )->Read( buffer, fileSize, f ); // read into local buffer

	buffer[ fileSize ] = 0; // null terminate file as EOF

	filesystem->Close( f );	// close file after reading

	bool retOK = LoadFromBuffer( resourceName, buffer, filesystem );

	delete[] buffer;

	//Old code:
	/*
	unsigned bufSize = ((IFileSystem *)filesystem)->GetOptimalReadSize( f, fileSize + 1 );

	char *buffer = (char*)((IFileSystem *)filesystem)->AllocOptimalReadBuffer( f, bufSize );
	
	Assert(buffer);
	
	((IFileSystem *)filesystem)->ReadEx(buffer, bufSize, fileSize, f); // read into local buffer

	buffer[fileSize] = 0; // null terminate file as EOF

	filesystem->Close( f );	// close file after reading

	bool retOK = LoadFromBuffer( resourceName, buffer, filesystem );

	((IFileSystem *)filesystem)->FreeOptimalReadBuffer( buffer );
	*/

	return retOK;
}

//-----------------------------------------------------------------------------
// Purpose: Save the keyvalues to disk
//			Creates the path to the file if it doesn't exist 
//-----------------------------------------------------------------------------
bool KeyValues::SaveToFile( IBaseFileSystem *filesystem, const char *resourceName, const char *pathID )
{
	// create a write file
	FileHandle_t f = filesystem->Open(resourceName, "wb", pathID);

	if ( f == FILESYSTEM_INVALID_HANDLE )
	{
		Msg( "KeyValues::SaveToFile: couldn't open file \"%s\" in path \"%s\".\n",
			resourceName?resourceName:"NULL", pathID?pathID:"NULL" );
		return false;
	}

	RecursiveSaveToFile(filesystem, f, NULL, 0);
	filesystem->Close(f);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Write out a set of indenting
//-----------------------------------------------------------------------------
void KeyValues::WriteIndents( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, int indentLevel )
{
	for ( int i = 0; i < indentLevel; i++ )
	{
		INTERNALWRITE( "\t", 1 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Write out a string where we convert the double quotes to backslash double quote
//-----------------------------------------------------------------------------
void KeyValues::WriteConvertedString( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, const char *pszString )
{
	// handle double quote chars within the string
	// the worst possible case is that the whole string is quotes
	int len = Q_strlen(pszString);
	char *convertedString = (char *) stackalloc((len + 1)  * sizeof(char) * 2);
	int j=0;
	for (int i=0; i <= len; i++)
	{
		if (pszString[i] == '\"')
		{
			convertedString[j] = '\\';
			j++;
		}
		convertedString[j] = pszString[i];
		j++;
	}		

	INTERNALWRITE(convertedString, strlen(convertedString));
	stackfree( convertedString );
}


void KeyValues::InternalWrite( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, const void *pData, int len )
{
	if ( filesystem )
	{
		filesystem->Write( pData, len, f );
	}

	if ( pBuf )
	{
		pBuf->Put( pData, len );
	}
} 


//-----------------------------------------------------------------------------
// Purpose: Save keyvalues from disk, if subkey values are detected, calls
//			itself to save those
//-----------------------------------------------------------------------------
void KeyValues::RecursiveSaveToFile( IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, int indentLevel )
{
	// write header
	WriteIndents( filesystem, f, pBuf, indentLevel );
	INTERNALWRITE("\"", 1);
	WriteConvertedString(filesystem, f, pBuf, GetName());	
	INTERNALWRITE("\"\n", 2);
	WriteIndents( filesystem, f, pBuf, indentLevel );
	INTERNALWRITE("{\n", 2);

	// loop through all our keys writing them to disk
	for ( KeyValues *dat = m_pSub; dat != NULL; dat = dat->m_pPeer )
	{
		if ( dat->m_pSub )
		{
			dat->RecursiveSaveToFile( filesystem, f, pBuf, indentLevel + 1 );
		}
		else
		{
			// only write non-empty keys

			switch (dat->m_iDataType)
			{
			case TYPE_STRING:
				{
					if (dat->m_pszValue && *(dat->m_pszValue))
					{
						WriteIndents(filesystem, f, pBuf, indentLevel + 1);
						INTERNALWRITE("\"", 1);
						WriteConvertedString(filesystem, f, pBuf, dat->GetName());	
						INTERNALWRITE("\"\t\t\"", 4);

						WriteConvertedString(filesystem, f, pBuf, dat->m_pszValue);

						INTERNALWRITE("\"\n", 2);
					}
					break;
				}
			case TYPE_WSTRING:
				{
					if ( dat->m_pwszValue )
					{
						static char buf[KEYVALUES_TOKEN_SIZE];
						int result = Q_UnicodeToUTF8( m_pwszValue, buf, sizeof( buf ) );

						if( result > 0 )
						{
							WriteIndents(filesystem, f, pBuf, indentLevel + 1);
							INTERNALWRITE("\"", 1);
							INTERNALWRITE(dat->GetName(), Q_strlen(dat->GetName()));
							INTERNALWRITE("\"\t\t\"", 4);

							WriteConvertedString(filesystem, f, pBuf, buf);

							INTERNALWRITE("\"\n", 2);
						}
					}
					break;
				}

			case TYPE_INT:
				{
					WriteIndents(filesystem, f, pBuf, indentLevel + 1);
					INTERNALWRITE("\"", 1);
					INTERNALWRITE(dat->GetName(), Q_strlen(dat->GetName()));
					INTERNALWRITE("\"\t\t\"", 4);

					char buf[32];
					Q_snprintf(buf, sizeof( buf ), "%d", dat->m_iValue);

					INTERNALWRITE(buf, Q_strlen(buf));
					INTERNALWRITE("\"\n", 2);
					break;
				}

			case TYPE_UINT64:
				{
					WriteIndents(filesystem, f, pBuf, indentLevel + 1);
					INTERNALWRITE("\"", 1);
					INTERNALWRITE(dat->GetName(), Q_strlen(dat->GetName()));
					INTERNALWRITE("\"\t\t\"", 4);

					char buf[32];
					Q_binarytohex((byte *)dat->m_pszValue, sizeof(uint64), buf, sizeof(buf));

					INTERNALWRITE(buf, Q_strlen(buf));
					INTERNALWRITE("\"\n", 2);
					break;
				}

			case TYPE_FLOAT:
				{
					WriteIndents(filesystem, f, pBuf, indentLevel + 1);
					INTERNALWRITE("\"", 1);
					INTERNALWRITE(dat->GetName(), Q_strlen(dat->GetName()));
					INTERNALWRITE("\"\t\t\"", 4);

					char buf[48];
					Q_snprintf(buf, sizeof( buf ), "%f", dat->m_flValue);

					INTERNALWRITE(buf, Q_strlen(buf));
					INTERNALWRITE("\"\n", 2);
					break;
				}
			case TYPE_COLOR:
				Msg( "KeyValues::RecursiveSaveToFile: TODO, missing code for TYPE_COLOR.\n");
				break;

			default:
				break;
			}
		}
	}

	// write tail
	WriteIndents(filesystem, f, pBuf, indentLevel);
	INTERNALWRITE("}\n", 2);
}

//-----------------------------------------------------------------------------
// Purpose: looks up a key by symbol name
//-----------------------------------------------------------------------------
KeyValues *KeyValues::FindKey(int keySymbol) const
{
	for (KeyValues *dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		if (dat->m_iKeyName == keySymbol)
			return dat;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find a keyValue, create it if it is not found.
//			Set bCreate to true to create the key if it doesn't already exist 
//			(which ensures a valid pointer will be returned)
//-----------------------------------------------------------------------------
KeyValues *KeyValues::FindKey(const char *keyName, bool bCreate)
{
	// return the current key if a NULL subkey is asked for
	if (!keyName || !keyName[0])
		return this;

	// look for '/' characters deliminating sub fields
	char szBuf[256];
	const char *subStr = strchr(keyName, '/');
	const char *searchStr = keyName;

	// pull out the substring if it exists
	if (subStr)
	{
		int size = subStr - keyName;
		Q_memcpy( szBuf, keyName, size );
		szBuf[size] = 0;
		searchStr = szBuf;
	}

	// lookup the symbol for the search string
	HKeySymbol iSearchStr = keyvalues()->GetSymbolForString(searchStr);

	KeyValues *lastItem = NULL;
	KeyValues *dat;
	// find the searchStr in the current peer list
	for (dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		lastItem = dat;	// record the last item looked at (for if we need to append to the end of the list)

		// symbol compare
		if (dat->m_iKeyName == iSearchStr)
		{
			break;
		}
	}

	// make sure a key was found
	if (!dat)
	{
		if (bCreate)
		{
			// we need to create a new key
			dat = new KeyValues( searchStr );
//			Assert(dat != NULL);

			// insert new key at end of list
			if (lastItem)
			{
				lastItem->m_pPeer = dat;
			}
			else
			{
				m_pSub = dat;
			}
			dat->m_pPeer = NULL;

			// a key graduates to be a submsg as soon as it's m_pSub is set
			// this should be the only place m_pSub is set
			m_iDataType = TYPE_NONE;
		}
		else
		{
			return NULL;
		}
	}
	
	// if we've still got a subStr we need to keep looking deeper in the tree
	if ( subStr )
	{
		// recursively chain down through the paths in the string
		return dat->FindKey(subStr + 1, bCreate);
	}

	return dat;
}

//-----------------------------------------------------------------------------
// Purpose: Create a new key, with an autogenerated name.  
//			Name is guaranteed to be an integer, of value 1 higher than the highest 
//			other integer key name
//-----------------------------------------------------------------------------
KeyValues *KeyValues::CreateNewKey()
{
	int newID = 1;

	// search for any key with higher values
	for (KeyValues *dat = m_pSub; dat != NULL; dat = dat->m_pPeer)
	{
		// case-insensitive string compare
		int val = atoi(dat->GetName());
		if (newID <= val)
		{
			newID = val + 1;
		}
	}

	char buf[12];
	Q_snprintf( buf, sizeof(buf), "%d", newID );

	return CreateKey( buf );
}


//-----------------------------------------------------------------------------
// Create a key
//-----------------------------------------------------------------------------
KeyValues* KeyValues::CreateKey( const char *keyName )
{
	// key wasn't found so just create a new one
	KeyValues* dat = new KeyValues( keyName );

	//dat->UsesEscapeSequences( m_bHasEscapeSequences != 0 ); // use same format as parent does
	
	// add into subkey list
	AddSubKey( dat );

	return dat;
}


//-----------------------------------------------------------------------------
// Adds a subkey. Make sure the subkey isn't a child of some other keyvalues
//-----------------------------------------------------------------------------
void KeyValues::AddSubKey( KeyValues *pSubkey )
{
	// Make sure the subkey isn't a child of some other keyvalues
	Assert( pSubkey->m_pPeer == NULL );

	// add into subkey list
	if ( m_pSub == NULL )
	{
		m_pSub = pSubkey;
	}
	else
	{
		KeyValues *pTempDat = m_pSub;
		while ( pTempDat->GetNextKey() != NULL )
		{
			pTempDat = pTempDat->GetNextKey();
		}

		pTempDat->SetNextKey( pSubkey );
	}
}


	
//-----------------------------------------------------------------------------
// Purpose: Remove a subkey from the list
//-----------------------------------------------------------------------------
void KeyValues::RemoveSubKey(KeyValues *subKey)
{
	if (!subKey)
		return;

	// check the list pointer
	if (m_pSub == subKey)
	{
		m_pSub = subKey->m_pPeer;
	}
	else
	{
		// look through the list
		KeyValues *kv = m_pSub;
		while (kv->m_pPeer)
		{
			if (kv->m_pPeer == subKey)
			{
				kv->m_pPeer = subKey->m_pPeer;
				break;
			}
			
			kv = kv->m_pPeer;
		}
	}

	subKey->m_pPeer = NULL;
}



//-----------------------------------------------------------------------------
// Purpose: Return the first subkey in the list
//-----------------------------------------------------------------------------
KeyValues *KeyValues::GetFirstSubKey()
{
	return m_pSub;
}

//-----------------------------------------------------------------------------
// Purpose: Return the next subkey
//-----------------------------------------------------------------------------
KeyValues *KeyValues::GetNextKey()
{
	return m_pPeer;
}

//-----------------------------------------------------------------------------
// Purpose: Sets this key's peer to the KeyValues passed in
//-----------------------------------------------------------------------------
void KeyValues::SetNextKey( KeyValues *pDat )
{
	m_pPeer = pDat;
}


KeyValues* KeyValues::GetFirstTrueSubKey()
{
	KeyValues *pRet = m_pSub;
	while ( pRet && pRet->m_iDataType != TYPE_NONE )
		pRet = pRet->m_pPeer;

	return pRet;
}

KeyValues* KeyValues::GetNextTrueSubKey()
{
	KeyValues *pRet = m_pPeer;
	while ( pRet && pRet->m_iDataType != TYPE_NONE )
		pRet = pRet->m_pPeer;

	return pRet;
}

KeyValues* KeyValues::GetFirstValue()
{
	KeyValues *pRet = m_pSub;
	while ( pRet && pRet->m_iDataType == TYPE_NONE )
		pRet = pRet->m_pPeer;

	return pRet;
}

KeyValues* KeyValues::GetNextValue()
{
	KeyValues *pRet = m_pPeer;
	while ( pRet && pRet->m_iDataType == TYPE_NONE )
		pRet = pRet->m_pPeer;

	return pRet;
}


//-----------------------------------------------------------------------------
// Purpose: Get the integer value of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
int KeyValues::GetInt( const char *keyName, int defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		switch ( dat->m_iDataType )
		{
		case TYPE_STRING:
			return atoi(dat->m_pszValue);
		case TYPE_WSTRING:
			return wcstol(dat->m_pwszValue, NULL, 10);
		case TYPE_FLOAT:
			return (int)dat->m_flValue;
		case TYPE_UINT64:
			// can't convert, since it would lose data
			Assert(0);
			return 0;
		case TYPE_INT:
		case TYPE_PTR:
		default:
			return dat->m_iValue;
		};
	}
	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get the integer value of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
uint64 KeyValues::GetUint64( const char *keyName, uint64 defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		switch ( dat->m_iDataType )
		{
		case TYPE_STRING:
			return atoi(dat->m_pszValue);
		case TYPE_WSTRING:
			return wcstoull( dat->m_pwszValue, NULL, 10 );
		case TYPE_FLOAT:
			return (int)dat->m_flValue;
		case TYPE_UINT64:
			return *((uint64 *)dat->m_pszValue);
		case TYPE_INT:
		case TYPE_PTR:
		default:
			return dat->m_iValue;
		};
	}
	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get the pointer value of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
void *KeyValues::GetPtr( const char *keyName, void *defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		switch ( dat->m_iDataType )
		{
		case TYPE_PTR:
			return dat->m_pValue;

		case TYPE_WSTRING:
		case TYPE_STRING:
		case TYPE_FLOAT:
		case TYPE_INT:
		case TYPE_UINT64:
		default:
			return NULL;
		};
	}
	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get the float value of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
float KeyValues::GetFloat( const char *keyName, float defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		switch ( dat->m_iDataType )
		{
		case TYPE_STRING:
			return (float)atof(dat->m_pszValue);
		case TYPE_WSTRING:
			return wcstod( dat->m_pwszValue, NULL );
		case TYPE_FLOAT:
			return dat->m_flValue;
		case TYPE_INT:
			return (float)dat->m_iValue;
		case TYPE_UINT64:
			return (float)(*((uint64 *)dat->m_pszValue));
		case TYPE_PTR:
		default:
			return 0.0f;
		};
	}
	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get the string pointer of a keyName. Default value is returned
//			if the keyName can't be found.
//-----------------------------------------------------------------------------
const char *KeyValues::GetString( const char *keyName, const char *defaultValue )
{
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		// convert the data to string form then return it
		char buf[64];
		switch ( dat->m_iDataType )
		{
		case TYPE_FLOAT:
			Q_snprintf( buf, sizeof( buf ), "%f", dat->m_flValue );
			SetString( keyName, buf );
			break;
		case TYPE_INT:
		case TYPE_PTR:
			Q_snprintf( buf, sizeof( buf ), "%d", dat->m_iValue );
			SetString( keyName, buf );
			break;
		case TYPE_UINT64:
			Q_snprintf( buf, sizeof( buf ), "%I64i", *((uint64 *)(dat->m_pszValue)) );
			SetString( keyName, buf );
			break;

		case TYPE_WSTRING:
		{
			// convert the string to char *, set it for future use, and return it
			static char szBuf[512];
			int result = Q_UnicodeToUTF8( dat->m_pwszValue, szBuf, sizeof( szBuf ) );

			if( result > 0 )
			{
				SetString( keyName, szBuf );
			}
			else
			{
				return defaultValue;
			}
			break;
		}
		case TYPE_STRING:
			break;
		default:
			return defaultValue;
		};
		
		return dat->m_pszValue;
	}
	return defaultValue;
}

const wchar_t *KeyValues::GetWString( const char *keyName, const wchar_t *defaultValue)
{
	KeyValues *dat = FindKey( keyName, false );

	if ( dat )
	{
		wchar_t wbuf[64];
		switch ( dat->m_iDataType )
		{
		case TYPE_FLOAT:
			swprintf(wbuf, ARRAYSIZE( wbuf ), L"%f", dat->m_flValue);
			SetWString( keyName, wbuf);
			break;
		case TYPE_INT:
		case TYPE_PTR:
			swprintf( wbuf, ARRAYSIZE( wbuf ), L"%d", dat->m_iValue );
			SetWString( keyName, wbuf );
			break;
		case TYPE_UINT64:
			{
				swprintf( wbuf, ARRAYSIZE( wbuf ), L"%I64i", *((uint64 *)(dat->m_pszValue)) );
				SetWString( keyName, wbuf );
			}
			break;

		case TYPE_WSTRING:
			break;
		case TYPE_STRING:
		{
			static wchar_t wbuftemp[512]; // convert to wide	
			int result = Q_UTF8ToUnicode( dat->m_pszValue, wbuftemp, sizeof( wbuftemp ) );

			if( result > 0 )
			{
				SetWString( keyName, wbuftemp );
			}
			else
			{
				return defaultValue;
			}
			break;
		}
		default:
			return defaultValue;
		};
		
		return dat->m_pwszValue;
	}

	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Get a bool interpretation of the key.
//-----------------------------------------------------------------------------
bool KeyValues::GetBool( const char *keyName, bool defaultValue, bool* optGotDefault )
{
	if ( FindKey( keyName ) )
    {
        if ( optGotDefault )
            (*optGotDefault) = false;
		return 0 != GetInt( keyName, 0 );
    }
    
    if ( optGotDefault )
        (*optGotDefault) = true;

	return defaultValue;
}

//-----------------------------------------------------------------------------
// Purpose: Gets a color
//-----------------------------------------------------------------------------
Color KeyValues::GetColor( const char *keyName )
{
	Color color(0, 0, 0, 0);
	KeyValues *dat = FindKey( keyName, false );
	if ( dat )
	{
		//TODO: convert to switch. - Solokiller
		if ( dat->m_iDataType == TYPE_COLOR )
		{
			color[0] = dat->m_Color[0];
			color[1] = dat->m_Color[1];
			color[2] = dat->m_Color[2];
			color[3] = dat->m_Color[3];
		}
		else if ( dat->m_iDataType == TYPE_FLOAT )
		{
			color[0] = dat->m_flValue;
		}
		else if ( dat->m_iDataType == TYPE_INT )
		{
			color[0] = dat->m_iValue;
		}
		else if ( dat->m_iDataType == TYPE_STRING )
		{
			// parse the colors out of the string
			float a, b, c, d;
			sscanf(dat->m_pszValue, "%f %f %f %f", &a, &b, &c, &d);
			color[0] = (unsigned char)a;
			color[1] = (unsigned char)b;
			color[2] = (unsigned char)c;
			color[3] = (unsigned char)d;
		}
		else if( dat->m_iDataType == TYPE_WSTRING )
		{
			// parse the colors out of the string
			float a, b, c, d;
			swscanf( dat->m_pwszValue, L"%f %f %f %f", &a, &b, &c, &d );
			color[ 0 ] = ( unsigned char ) a;
			color[ 1 ] = ( unsigned char ) b;
			color[ 2 ] = ( unsigned char ) c;
			color[ 3 ] = ( unsigned char ) d;
		}
	}
	return color;
}

//-----------------------------------------------------------------------------
// Purpose: Sets a color
//-----------------------------------------------------------------------------
void KeyValues::SetColor( const char *keyName, Color value)
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->FreeAllocatedValue();

		dat->m_iDataType = TYPE_COLOR;
		dat->m_Color[0] = value[0];
		dat->m_Color[1] = value[1];
		dat->m_Color[2] = value[2];
		dat->m_Color[3] = value[3];
	}
}

void KeyValues::SetStringValue( char const *strValue )
{
	SetString( NULL, strValue );
}

//-----------------------------------------------------------------------------
// Purpose: Set the string value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetString( const char *keyName, const char *value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->FreeAllocatedValue();

		if (!value)
		{
			// ensure a valid value
			value = "";
		}

		// allocate memory for the new value and copy it in
		int len = Q_strlen( value );
		dat->AllocateValueBlock( len + 1 );
		Q_memcpy( dat->m_pszValue, value, len+1 );

		dat->m_iDataType = TYPE_STRING;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the string value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetWString( const char *keyName, const wchar_t *value )
{
	KeyValues *dat = FindKey( keyName, true );
	if ( dat )
	{
		dat->FreeAllocatedValue();

		if (!value)
		{
			// ensure a valid value
			value = L"";
		}

		// allocate memory for the new value and copy it in
		int len = wcslen( value );
		dat->AllocateValueBlock( ( len + 1 ) * sizeof( wchar_t ) );
		Q_memcpy( dat->m_pwszValue, value, (len+1) * sizeof(wchar_t) );

		dat->m_iDataType = TYPE_WSTRING;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the integer value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetInt( const char *keyName, int value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->FreeAllocatedValue();

		dat->m_iValue = value;
		dat->m_iDataType = TYPE_INT;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the integer value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetUint64( const char *keyName, uint64 value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->FreeAllocatedValue();

		dat->AllocateValueBlock( sizeof( uint64 ) );
		*((uint64 *)dat->m_pszValue) = value;
		dat->m_iDataType = TYPE_UINT64;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the float value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetFloat( const char *keyName, float value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->FreeAllocatedValue();

		dat->m_flValue = value;
		dat->m_iDataType = TYPE_FLOAT;
	}
}

void KeyValues::SetName( const char * setName )
{
	m_iKeyName = keyvalues()->GetSymbolForString( setName );
}

//-----------------------------------------------------------------------------
// Purpose: Set the pointer value of a keyName. 
//-----------------------------------------------------------------------------
void KeyValues::SetPtr( const char *keyName, void *value )
{
	KeyValues *dat = FindKey( keyName, true );

	if ( dat )
	{
		dat->FreeAllocatedValue();

		dat->m_pValue = value;
		dat->m_iDataType = TYPE_PTR;
	}
}

void KeyValues::RecursiveCopyKeyValues( KeyValues& src )
{
	// garymcthack - need to check this code for possible buffer overruns.
	
	m_iKeyName = src.GetNameSymbol();

	if( !src.m_pSub )
	{
		FreeAllocatedValue();
		m_iDataType = src.m_iDataType;
		char buf[256];
		switch( src.m_iDataType )
		{
		case TYPE_NONE:
			break;
		case TYPE_STRING:
			if( src.m_pszValue )
			{
				int len = Q_strlen(src.m_pszValue) + 1;
				AllocateValueBlock( len );
				Q_strncpy( m_pszValue, src.m_pszValue, len );
			}
			break;
		case TYPE_INT:
			{
				m_iValue = src.m_iValue;
				Q_snprintf( buf,sizeof(buf), "%d", m_iValue );
				int len = Q_strlen(buf) + 1;
				AllocateValueBlock( len );
				Q_strncpy( m_pszValue, buf, len  );
			}
			break;
		case TYPE_FLOAT:
			{
				m_flValue = src.m_flValue;
				Q_snprintf( buf,sizeof(buf), "%f", m_flValue );
				int len = Q_strlen(buf) + 1;
				AllocateValueBlock( len );
				Q_strncpy( m_pszValue, buf, len );
			}
			break;
		case TYPE_PTR:
			{
				m_pValue = src.m_pValue;
			}
			break;
		case TYPE_UINT64:
			{
				AllocateValueBlock( sizeof( uint64 ) );
				Q_memcpy( m_pszValue, src.m_pszValue, sizeof(uint64) );
			}
			break;
		case TYPE_COLOR:
			{
				m_Color[0] = src.m_Color[0];
				m_Color[1] = src.m_Color[1];
				m_Color[2] = src.m_Color[2];
				m_Color[3] = src.m_Color[3];
			}
			break;
			
		default:
			{
				// do nothing . .what the heck is this?
				Assert( 0 );
			}
			break;
		}

	}
#if 0
	KeyValues *pDst = this;
	for ( KeyValues *pSrc = src.m_pSub; pSrc; pSrc = pSrc->m_pPeer )
	{
		if ( pSrc->m_pSub )
		{
			pDst->m_pSub = new KeyValues( pSrc->m_pSub->getName() );
			pDst->m_pSub->RecursiveCopyKeyValues( *pSrc->m_pSub );
		}
		else
		{
			// copy non-empty keys
			if ( pSrc->m_sValue && *(pSrc->m_sValue) )
			{
				pDst->m_pPeer = new KeyValues( 
			}
		}
	}
#endif

	// Handle the immediate child
	if( src.m_pSub )
	{
		m_pSub = new KeyValues( NULL );
		m_pSub->RecursiveCopyKeyValues( *src.m_pSub );
	}

	// Handle the immediate peer
	if( src.m_pPeer )
	{
		m_pPeer = new KeyValues( NULL );
		m_pPeer->RecursiveCopyKeyValues( *src.m_pPeer );
	}
}

KeyValues& KeyValues::operator=( KeyValues& src )
{
	RemoveEverything();
	Init();	// reset all values
	RecursiveCopyKeyValues( src );
	return *this;
}


//-----------------------------------------------------------------------------
// Make a new copy of all subkeys, add them all to the passed-in keyvalues
//-----------------------------------------------------------------------------
void KeyValues::CopySubkeys( KeyValues *pParent ) const
{
	// recursively copy subkeys
	// Also maintain ordering....
	KeyValues *pPrev = NULL;
	for ( KeyValues *sub = m_pSub; sub != NULL; sub = sub->m_pPeer )
	{
		// take a copy of the subkey
		KeyValues *dat = sub->MakeCopy();
		 
		// add into subkey list
		if (pPrev)
		{
			pPrev->m_pPeer = dat;
		}
		else
		{
			pParent->m_pSub = dat;
		}
		dat->m_pPeer = NULL;
		pPrev = dat;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Makes a copy of the whole key-value pair set
//-----------------------------------------------------------------------------
KeyValues *KeyValues::MakeCopy( void ) const
{
	KeyValues *newKeyValue = new KeyValues(GetName());

	// copy data
	newKeyValue->m_iDataType = m_iDataType;
	switch ( m_iDataType )
	{
	case TYPE_STRING:
		{
			if ( m_pszValue )
			{
				int len = Q_strlen( m_pszValue );
				Assert( !newKeyValue->m_pszValue );
				newKeyValue->AllocateValueBlock( len + 1 );
				Q_memcpy( newKeyValue->m_pszValue, m_pszValue, len+1 );
			}
		}
		break;
	case TYPE_WSTRING:
		{
			if ( m_pwszValue )
			{
				int len = wcslen( m_pwszValue );
				newKeyValue->AllocateValueBlock( ( len + 1 ) * sizeof( wchar_t ) );
				Q_memcpy( newKeyValue->m_pwszValue, m_pwszValue, (len+1)*sizeof(wchar_t));
			}
		}
		break;

	case TYPE_INT:
		newKeyValue->m_iValue = m_iValue;
		break;

	case TYPE_FLOAT:
		newKeyValue->m_flValue = m_flValue;
		break;

	case TYPE_PTR:
		newKeyValue->m_pValue = m_pValue;
		break;
		
	case TYPE_COLOR:
		newKeyValue->m_Color[0] = m_Color[0];
		newKeyValue->m_Color[1] = m_Color[1];
		newKeyValue->m_Color[2] = m_Color[2];
		newKeyValue->m_Color[3] = m_Color[3];
		break;

	case TYPE_UINT64:
		newKeyValue->AllocateValueBlock( sizeof( uint64 ) );
		Q_memcpy( newKeyValue->m_pszValue, m_pszValue, sizeof(uint64) );
		break;
	};

	// recursively copy subkeys
	CopySubkeys( newKeyValue );
	return newKeyValue;
}


//-----------------------------------------------------------------------------
// Purpose: Check if a keyName has no value assigned to it.
//-----------------------------------------------------------------------------
bool KeyValues::IsEmpty(const char *keyName)
{
	KeyValues *dat = FindKey(keyName, false);
	if (!dat)
		return true;

	if (dat->m_iDataType == TYPE_NONE && dat->m_pSub == NULL)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Clear out all subkeys, and the current value
//-----------------------------------------------------------------------------
void KeyValues::Clear( void )
{
	delete m_pSub;
	m_pSub = NULL;
	m_iDataType = TYPE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Get the data type of the value stored in a keyName
//-----------------------------------------------------------------------------
KeyValues::types_t KeyValues::GetDataType(const char *keyName)
{
	KeyValues *dat = FindKey(keyName, false);
	if (dat)
		return (types_t)dat->m_iDataType;

	return TYPE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: Deletion, ensures object gets deleted from correct heap
//-----------------------------------------------------------------------------
void KeyValues::deleteThis()
{
	delete this;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : includedKeys - 
//-----------------------------------------------------------------------------
void KeyValues::AppendIncludedKeys( CUtlVector< KeyValues * >& includedKeys )
{
	// Append any included keys, too...
	int includeCount = includedKeys.Count();
	int i;
	for ( i = 0; i < includeCount; i++ )
	{
		KeyValues *kv = includedKeys[ i ];
		Assert( kv );

		KeyValues *insertSpot = this;
		while ( insertSpot->GetNextKey() )
		{
			insertSpot = insertSpot->GetNextKey();
		}

		insertSpot->SetNextKey( kv );
	}
}

void KeyValues::ParseIncludedKeys( char const *resourceName, const char *filetoinclude, 
		IBaseFileSystem* pFileSystem, const char *pPathID, CUtlVector< KeyValues * >& includedKeys )
{
	Assert( resourceName );
	Assert( filetoinclude );
	Assert( pFileSystem );
	
	// Load it...
	if ( !pFileSystem )
	{
		return;
	}

	// Get relative subdirectory
	char fullpath[ 512 ];
	Q_strncpy( fullpath, resourceName, sizeof( fullpath ) );

	// Strip off characters back to start or first /
	bool done = false;
	int len = Q_strlen( fullpath );
	while ( !done )
	{
		if ( len <= 0 )
		{
			break;
		}
		
		if ( fullpath[ len - 1 ] == '\\' || 
			 fullpath[ len - 1 ] == '/' )
		{
			break;
		}

		// zero it
		fullpath[ len - 1 ] = 0;
		--len;
	}

	// Append included file
	Q_strncat( fullpath, filetoinclude, sizeof( fullpath ), COPY_ALL_CHARACTERS );

	KeyValues *newKV = new KeyValues( fullpath );

	// CUtlSymbol save = s_CurrentFileSymbol;	// did that had any use ???

	//newKV->UsesEscapeSequences( m_bHasEscapeSequences != 0 );	// use same format as parent

	if ( newKV->LoadFromFile( pFileSystem, fullpath, pPathID ) )
	{
		includedKeys.AddToTail( newKV );
	}
	else
	{
		Msg( "KeyValues::ParseIncludedKeys: Couldn't load included keyvalue file %s\n", fullpath );
		newKV->deleteThis();
	}

	// s_CurrentFileSymbol = save;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : baseKeys - 
//-----------------------------------------------------------------------------
void KeyValues::MergeBaseKeys( CUtlVector< KeyValues * >& baseKeys )
{
	int includeCount = baseKeys.Count();
	int i;
	for ( i = 0; i < includeCount; i++ )
	{
		KeyValues *kv = baseKeys[ i ];
		Assert( kv );

		RecursiveMergeKeyValues( kv );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : baseKV - keyvalues we're basing ourselves on
//-----------------------------------------------------------------------------
void KeyValues::RecursiveMergeKeyValues( KeyValues *baseKV )
{
	// Merge ourselves
	// we always want to keep our value, so nothing to do here

	// Now merge our children
	for ( KeyValues *baseChild = baseKV->m_pSub; baseChild != NULL; baseChild = baseChild->m_pPeer )
	{
		// for each child in base, see if we have a matching kv

		bool bFoundMatch = false;

		// If we have a child by the same name, merge those keys
		for ( KeyValues *newChild = m_pSub; newChild != NULL; newChild = newChild->m_pPeer )
		{
			if ( !Q_strcmp( baseChild->GetName(), newChild->GetName() ) )
			{
				newChild->RecursiveMergeKeyValues( baseChild );
				bFoundMatch = true;
				break;
			}	
		}

		// If not merged, append this key
		if ( !bFoundMatch )
		{
			KeyValues *dat = baseChild->MakeCopy();
			Assert( dat );
			AddSubKey( dat );
		}
	}
}

//-----------------------------------------------------------------------------
// Read from a buffer...
//-----------------------------------------------------------------------------
bool KeyValues::LoadFromBuffer( char const *resourceName, CUtlBuffer &buf, IBaseFileSystem* pFileSystem, const char *pPathID )
{
	KeyValues *pPreviousKey = NULL;
	KeyValues *pCurrentKey = this;
	CUtlVector< KeyValues * > includedKeys;
	CUtlVector< KeyValues * > baseKeys;
	bool wasQuoted;
	bool wasConditional;
	g_KeyValuesErrorStack.SetFilename( resourceName );	
	do 
	{
		bool bAccepted = true;

		// the first thing must be a key
		const char *s = ReadToken( buf, wasQuoted, wasConditional );
		if ( !buf.IsValid() || !s || *s == 0 )
			break;

		if ( !Q_stricmp( s, "#include" ) )	// special include macro (not a key name)
		{
			s = ReadToken( buf, wasQuoted, wasConditional );
			// Name of subfile to load is now in s

			if ( !s || *s == 0 )
			{
				g_KeyValuesErrorStack.ReportError("#include is NULL " );
			}
			else
			{
				ParseIncludedKeys( resourceName, s, pFileSystem, pPathID, includedKeys );
			}

			continue;
		}
		else if ( !Q_stricmp( s, "#base" ) )
		{
			s = ReadToken( buf, wasQuoted, wasConditional );
			// Name of subfile to load is now in s

			if ( !s || *s == 0 )
			{
				g_KeyValuesErrorStack.ReportError("#base is NULL " );
			}
			else
			{
				ParseIncludedKeys( resourceName, s, pFileSystem, pPathID, baseKeys );
			}

			continue;
		}

		if ( !pCurrentKey )
		{
			pCurrentKey = new KeyValues( s );
			Assert( pCurrentKey );

			//pCurrentKey->UsesEscapeSequences( m_bHasEscapeSequences != 0 ); // same format has parent use

			if ( pPreviousKey )
			{
				pPreviousKey->SetNextKey( pCurrentKey );
			}
		}
		else
		{
			pCurrentKey->SetName( s );
		}

		// get the '{'
		s = ReadToken( buf, wasQuoted, wasConditional );

		if ( wasConditional )
		{
			bAccepted = EvaluateConditional( s );

			// Now get the '{'
			s = ReadToken( buf, wasQuoted, wasConditional );
		}

		if ( s && *s == '{' && !wasQuoted )
		{
			// header is valid so load the file
			pCurrentKey->RecursiveLoadFromBuffer( resourceName, buf );
		}
		else
		{
			g_KeyValuesErrorStack.ReportError("LoadFromBuffer: missing {" );
		}

		if ( !bAccepted )
		{
			if ( pPreviousKey )
			{
				pPreviousKey->SetNextKey( NULL );
			}
			pCurrentKey->Clear();
		}
		else
		{
			pPreviousKey = pCurrentKey;
			pCurrentKey = NULL;
		}
	} while ( buf.IsValid() );

	AppendIncludedKeys( includedKeys );
	{
		// delete included keys!
		int i;
		for ( i = includedKeys.Count() - 1; i > 0; i-- )
		{
			KeyValues *kv = includedKeys[ i ];
			kv->deleteThis();
		}
	}

	MergeBaseKeys( baseKeys );
	{
		// delete base keys!
		int i;
		for ( i = baseKeys.Count() - 1; i >= 0; i-- )
		{
			KeyValues *kv = baseKeys[ i ];
			kv->deleteThis();
		}
	}

	g_KeyValuesErrorStack.SetFilename( "" );	

	return true;
}


//-----------------------------------------------------------------------------
// Read from a buffer...
//-----------------------------------------------------------------------------
bool KeyValues::LoadFromBuffer( char const *resourceName, const char *pBuffer, IBaseFileSystem* pFileSystem, const char *pPathID )
{
	if ( !pBuffer )
		return true;

	int nLen = Q_strlen( pBuffer );
	CUtlBuffer buf( pBuffer, nLen, CUtlBuffer::READ_ONLY | CUtlBuffer::TEXT_BUFFER );
	return LoadFromBuffer( resourceName, buf, pFileSystem, pPathID );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void KeyValues::RecursiveLoadFromBuffer( char const *resourceName, CUtlBuffer &buf )
{
	CKeyErrorContext errorReport(this);
	bool wasQuoted;
	bool wasConditional;
	// keep this out of the stack until a key is parsed
	CKeyErrorContext errorKey( INVALID_KEY_SYMBOL );
	while ( 1 )
	{
		bool bAccepted = true;

		// get the key name
		const char * name = ReadToken( buf, wasQuoted, wasConditional );

		if ( !name )	// EOF stop reading
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got EOF instead of keyname" );
			break;
		}

		if ( !*name ) // empty token, maybe "" or EOF
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got empty keyname" );
			break;
		}

		if ( *name == '}' && !wasQuoted )	// top level closed, stop reading
			break;

		// Always create the key; note that this could potentially
		// cause some duplication, but that's what we want sometimes
		KeyValues *dat = CreateKey( name );

		errorKey.Reset( dat->GetNameSymbol() );
		// get the value
		const char * value = ReadToken( buf, wasQuoted, wasConditional );

		if (wasConditional && value)
		{
			bAccepted = EvaluateConditional(value);

			// get the real value
			value = ReadToken(buf, wasQuoted, wasConditional);
		}

		if ( !value )
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got NULL key" );
			break;
		}
		
		if ( *value == '}' && !wasQuoted )
		{
			g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got } in key" );
			break;
		}

		if ( *value == '{' && !wasQuoted )
		{
			// this isn't a key, it's a section
			errorKey.Reset( INVALID_KEY_SYMBOL );
			// sub value list
			dat->RecursiveLoadFromBuffer( resourceName, buf );
		}
		else 
		{
			if (wasConditional)
			{
				g_KeyValuesErrorStack.ReportError("RecursiveLoadFromBuffer:  got conditional between key and value");
				break;
			}

			if (dat->m_pszValue)
			{
				dat->FreeAllocatedValue();
			}

			int len = Q_strlen( value );

			// Here, let's determine if we got a float or an int....
			char* pIEnd;	// pos where int scan ended
			char* pFEnd;	// pos where float scan ended
			const char* pSEnd = value + len ; // pos where token ends

			int ival = strtol( value, &pIEnd, 10 );
			float fval = (float)strtod( value, &pFEnd );

			if ( *value == 0 )
			{
				dat->m_iDataType = TYPE_STRING;	
			}
			else if ( (pFEnd > pIEnd) && (pFEnd == pSEnd) )
			{
				dat->m_flValue = fval; 
				dat->m_iDataType = TYPE_FLOAT;
			}
			else if (pIEnd == pSEnd)
			{
				dat->m_iValue = ival; 
				dat->m_iDataType = TYPE_INT;
			}
			else
			{
				dat->m_iDataType = TYPE_STRING;
			}

			if (dat->m_iDataType == TYPE_STRING)
			{
				// copy in the string information
				dat->AllocateValueBlock( len + 1 );
				Q_memcpy( dat->m_pszValue, value, len+1 );
			}

			// Look ahead one token for a conditional tag
			int prevPos = buf.TellGet();
			const char *peek = ReadToken(buf, wasQuoted, wasConditional);
			if (wasConditional)
			{
				bAccepted = EvaluateConditional(peek);
			}
			else
			{
				buf.SeekGet(CUtlBuffer::SEEK_HEAD, prevPos);
			}
		}

		if (!bAccepted)
		{
			this->RemoveSubKey(dat);
			dat->deleteThis();
			dat = NULL;
		}
	}
}



// writes KeyValue as binary data to buffer
bool KeyValues::WriteAsBinary( CUtlBuffer &buffer )
{
	if ( buffer.IsText() ) // must be a binary buffer
		return false;

	if ( !buffer.IsValid() ) // must be valid, no overflows etc
		return false;

	// Write subkeys:
	
	// loop through all our peers
	for ( KeyValues *dat = this; dat != NULL; dat = dat->m_pPeer )
	{
		// write type
		buffer.PutUnsignedChar( dat->m_iDataType );

		// write name
		buffer.PutString( dat->GetName() );

		// write type
		switch (dat->m_iDataType)
		{
		case TYPE_NONE:
			{
				dat->m_pSub->WriteAsBinary( buffer );
				break;
			}
		case TYPE_STRING:
			{
				if (dat->m_pszValue && *(dat->m_pszValue))
				{
					buffer.PutString( dat->m_pszValue );
				}
				else
				{
					buffer.PutString( "" );
				}
				break;
			}
		case TYPE_WSTRING:
			{
				Assert( !"TYPE_WSTRING" );
				break;
			}

		case TYPE_INT:
			{
				buffer.PutInt( dat->m_iValue );				
				break;
			}

		case TYPE_UINT64:
			{
				buffer.PutDouble( *((double *)dat->m_pszValue) );
				break;
			}

		case TYPE_FLOAT:
			{
				buffer.PutFloat( dat->m_flValue );
				break;
			}
		case TYPE_COLOR:
			{
				buffer.PutUnsignedChar( dat->m_Color[0] );
				buffer.PutUnsignedChar( dat->m_Color[1] );
				buffer.PutUnsignedChar( dat->m_Color[2] );
				buffer.PutUnsignedChar( dat->m_Color[3] );
				break;
			}
		case TYPE_PTR:
			{
				buffer.PutUnsignedInt( (int)dat->m_pValue );
			}

		default:
			break;
		}
	}

	// write tail, marks end of peers
	buffer.PutUnsignedChar( TYPE_NUMTYPES ); 

	return buffer.IsValid();
}

// read KeyValues from binary buffer, returns true if parsing was successful
bool KeyValues::ReadAsBinary( CUtlBuffer &buffer )
{
	if ( buffer.IsText() ) // must be a binary buffer
		return false;

	if ( !buffer.IsValid() ) // must be valid, no overflows etc
		return false;

	RemoveEverything(); // remove current content
	Init();	// reset
	
	char		token[KEYVALUES_TOKEN_SIZE];
	KeyValues	*dat = this;
	types_t		type = (types_t)buffer.GetUnsignedChar();
	
	// loop through all our peers
	while ( true )
	{
		if ( type == TYPE_NUMTYPES )
			break; // no more peers

		dat->m_iDataType = type;

		buffer.GetString( token );
		token[KEYVALUES_TOKEN_SIZE-1] = 0;

		dat->SetName( token );
		
		switch ( type )
		{
		case TYPE_NONE:
			{
				dat->m_pSub = new KeyValues("");
				dat->m_pSub->ReadAsBinary( buffer );
				break;
			}
		case TYPE_STRING:
			{
				buffer.GetString( token );
				token[KEYVALUES_TOKEN_SIZE-1] = 0;

				int len = Q_strlen( token );
				dat->AllocateValueBlock( len + 1 );
				Q_memcpy( dat->m_pszValue, token, len+1 );
								
				break;
			}
		case TYPE_WSTRING:
			{
				Assert( !"TYPE_WSTRING" );
				break;
			}

		case TYPE_INT:
			{
				dat->m_iValue = buffer.GetInt();
				break;
			}

		case TYPE_UINT64:
			{
				dat->AllocateValueBlock( sizeof( uint64 ) );
				*((double *)dat->m_pszValue) = buffer.GetDouble();
			}

		case TYPE_FLOAT:
			{
				dat->m_flValue = buffer.GetFloat();
				break;
			}
		case TYPE_COLOR:
			{
				dat->m_Color[0] = buffer.GetUnsignedChar();
				dat->m_Color[1] = buffer.GetUnsignedChar();
				dat->m_Color[2] = buffer.GetUnsignedChar();
				dat->m_Color[3] = buffer.GetUnsignedChar();
				break;
			}
		case TYPE_PTR:
			{
				dat->m_pValue = (void*)buffer.GetUnsignedInt();
			}

		default:
			break;
		}

		if ( !buffer.IsValid() ) // error occured
			return false;

		type = (types_t)buffer.GetUnsignedChar();

		if ( type == TYPE_NUMTYPES )
			break;

		// new peer follows
		dat->m_pPeer = new KeyValues("");
		dat = dat->m_pPeer;
	}

	return buffer.IsValid();
}

void KeyValues::FreeAllocatedValue()
{
	if( m_iAllocationSize )
	{
		if( m_iAllocationSize <= static_cast<int>( sizeof( KeyValues ) ) )
		{
			keyvalues()->FreeKeyValuesMemory( m_pValue );
		}
		else if( m_pValue )
		{
			delete[] m_pszValue;
		}

		m_pValue = NULL;

		m_iAllocationSize = 0;
	}
}

void KeyValues::AllocateValueBlock( int size )
{
	Assert( m_iAllocationSize == 0 );

	if( size <= static_cast<int>( sizeof( KeyValues ) ) )
	{
		m_pValue = keyvalues()->AllocKeyValuesMemory( size );
	}
	else
	{
		m_pValue = new byte[ size ];
	}

	m_iAllocationSize = size;
}

#include "tier0/memdbgoff.h"

//-----------------------------------------------------------------------------
// Purpose: memory allocator
//-----------------------------------------------------------------------------
void *KeyValues::operator new( size_t iAllocSize )
{
	MEM_ALLOC_CREDIT();
	return keyvalues()->AllocKeyValuesMemory(iAllocSize);
}

void *KeyValues::operator new( size_t iAllocSize, int nBlockUse, const char *pFileName, int nLine )
{
	MemAlloc_PushAllocDbgInfo( pFileName, nLine );
	void *p = keyvalues()->AllocKeyValuesMemory(iAllocSize);
	MemAlloc_PopAllocDbgInfo();
	return p;
}

//-----------------------------------------------------------------------------
// Purpose: deallocator
//-----------------------------------------------------------------------------
void KeyValues::operator delete( void *pMem )
{
	keyvalues()->FreeKeyValuesMemory(pMem);
}

void KeyValues::operator delete( void *pMem, int nBlockUse, const char *pFileName, int nLine )
{
	keyvalues()->FreeKeyValuesMemory(pMem);
}

//-----------------------------------------------------------------------------
// Returns whether a keyvalues conditional evaluates to true or false
// Needs more flexibility with conditionals, checking convars would be nice.
//-----------------------------------------------------------------------------
bool EvaluateConditional(const char *str)
{
	if ( !str )
		return false;

	if ( *str == '[' )
		str++;

	bool bNot = false; // should we negate this command?
	if ( *str == '!' )
		bNot = true;

	if ( Q_stristr( str, "$X360" ) )
		return IsX360() ^ bNot;
	
	if ( Q_stristr( str, "$WIN32" ) )
		return IsPC() ^ bNot; // hack hack - for now WIN32 really means IsPC

	if ( Q_stristr( str, "$WINDOWS" ) )
		return IsWindows() ^ bNot;
	
	if ( Q_stristr( str, "$OSX" ) )
		return IsOSX() ^ bNot;
	
	if ( Q_stristr( str, "$LINUX" ) )
		return IsLinux() ^ bNot;

	if ( Q_stristr( str, "$POSIX" ) )
		return IsPosix() ^ bNot;
	
	return false;
}

#include "tier0/memdbgon.h"

#ifdef POSIX
#pragma GCC diagnostic pop
#endif
