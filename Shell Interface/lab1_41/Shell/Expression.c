#include "Expression.h"

extern Utility_t Utility;
static char * getVarValue( const char * varName );
static bool addVariable( const char * varName, const char * varValue );
static Variable_t * getLocalVar( const char * varName );

Expression_t Expression = { getVarValue, addVariable };
size_t varsSize = 0;
Variable_t * variables;
size_t varsNum = 0;

static char * getVarValue( const char * varName )
{
    Variable_t * var = getLocalVar( varName );
    if( var != NULL )
    {
        return var->value;
    }

    return getenv( varName ); // Search in enviroment variables
}

static Variable_t * getLocalVar( const char * varName )
{
    for( int i = 0; i < varsNum; i++ )
    {
        if( strcmp( varName, variables[i].name ) == 0 )
        {
            return &variables[i];
        }
    }

    return NULL;
}

static bool addVariable( const char * varName, const char * varValue )
{
    Variable_t * var = getLocalVar( varName );
    if( var != NULL )
    {
        strcpy( var->value, varValue );
        return true;
    }
    if( varsNum >= varsSize )
    {
        varsSize += 16;
        Variable_t * tmp = malloc( varsSize * sizeof( Variable_t ) );
        if( tmp == NULL ) return false;
        variables = tmp;
    }

    strcpy( variables[ varsNum ].name, varName );
    strcpy( variables[ varsNum ].value, varValue );
    ++varsNum;

    return true;
}
