#include "Utility.h"

static bool copyStringFromTo( char ** destination, const char * first, const char * last );
static bool copyString( char ** destination, const char * source );
static bool newStringArray( char *** strArray, size_t len );
static bool extendStringArray ( char *** strArray, size_t len );
static void leadingTrim( char * str );
static int splitOnDelim( char *** tokens, char * str, const char delim[]  );
static bool extendString ( char ** str, size_t len );
static void trailingTrim( char * str );
static void trim( char * str );

Utility_t Utility = {copyStringFromTo, copyString, newStringArray, extendStringArray, leadingTrim,
                     splitOnDelim, extendString, trailingTrim, trim };

static bool copyStringFromTo( char ** destination, const char * first, const char * last )
{
    char * newStr = (char *) malloc( (last - first + 2) * sizeof( char ) );
    if( newStr == NULL ) // Allocation failed
    {
        return false;
    }

    size_t i = 0;
    while( first <= last )
    {
        newStr[i++] = *first;
        ++first;
    }
    newStr[i] = '\0';

    *destination = newStr;

    return true;
}

static bool copyString( char ** destination, const char * source )
{
    return copyStringFromTo( destination, source, source + strlen( source ) - 1 );
}

static bool newStringArray( char *** strArray, size_t len )
{
    char ** tmp = (char **) malloc( len * sizeof( char* ) );
    if( tmp == NULL ) // Allocation failed
    {
        return false;
    }

    *strArray = tmp;

    return true;
}

static bool extendStringArray ( char *** strArray, size_t len )
{
    char ** tmp = (char **) realloc( *strArray, len * sizeof( char* ) );
    if( tmp == NULL ) // Reallocation failed
    {
        return false;
    }

    *strArray = tmp;

    return true;
}

static void leadingTrim( char * str )
{
    size_t i = 0;
    while( isspace( str[i] ) )
    {
        i++;
    }
    if( i != 0 )
    {
        memmove( str, str + i, strlen( str ) - i + 1 );
    }
}

static void trailingTrim( char * str )
{
    size_t i = strlen( str ) - 1;

    while( i >= 0 && isspace( str[i] ) )
    {
        --i;
    }

    if( i >= 0 )
    {
        str[ i + 1 ] = '\0';
    }
}
static void trim( char * str )
{
    trailingTrim( str );
    leadingTrim( str );
}
static int splitOnDelim( char *** tokens, char * str, const char delim[]  )
{
    size_t tokenNum = 16;
    char ** localTokens;

    if( !Utility.newStringArray( &localTokens, tokenNum ) )  return -1;

    int tokenIdx = 0;
    char * token = strtok( str, delim );

    while( token != NULL )
    {
        if( tokenIdx >= tokenNum )
        {
            tokenNum <<= 1; // double the length;
            if( !Utility.extendStringArray( &localTokens, tokenNum ) )
            {
                return -1;
            }
        }

        localTokens[ tokenIdx++ ] = token;

        token = strtok( NULL, delim );
    }

    *tokens = localTokens;

    return tokenIdx;
}

static bool extendString ( char ** str, size_t len )
{
    char * tmp = (char *) realloc( *str, len * sizeof( char ) );
    if( tmp == NULL ) // Reallocation failed
    {
        return false;
    }

    *str = tmp;

    return true;
}
