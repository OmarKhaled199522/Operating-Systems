#include "Parser.h"

extern const int UNBALANCE_QUOTES,
                 EXECUTION_FAILED,
                 SUCCESSFUL;

static int parse( char const * );
static bool addArguments( int num , char ** args );

Parser_t Parser = { parse };
extern Command_t Command;
extern Utility_t Utility;
char ** args;
size_t argsNum = 16, argsIdx = 0;

static bool splitOnSpace( const char * begin, const char * end ) // Split the string from begin to end without changing it
{
    char * str;
    if( !Utility.copyStringFromTo( &str, begin, end ) )
    {
        return false;
    }

    const static char delim[] = " \t\n\r\f\v";
    size_t tokenNum = 16;
    char ** tokens;
    if( !Utility.newStringArray( &tokens, tokenNum ) )
    {
        return false;
    }

    size_t tokenIdx = 0;
    char * token = strtok( str, delim );

    while( token != NULL )
    {
        if( tokenIdx >= tokenNum )
        {
            tokenNum <<= 1; // double the length;
            if( !Utility.extendStringArray( &tokens, tokenNum ) )
            {
                return false;
            }
        }

        tokens[ tokenIdx++ ] = token;

        token = strtok( NULL, delim );
    }

    if( !addArguments( tokenIdx, tokens ) )
    {
        return false;
    }

    return true;
}

static bool addArgument( char * arg )
{
    if( argsIdx >= argsNum )
    {
        argsNum <<= 1;
        if( !Utility.extendStringArray( &args, argsNum ) )
        {
            return false;
        }
    }

    args[ argsIdx++ ] = arg;
    return true;
}

static bool addArguments( int num , char ** args )
{
    for( int i = 0; i < num; i++ )
    {
        if( !addArgument( args[i] ) )
        {
            return false;
        }
    }

    return true;
}

static int parse( char const * command )
{
	char leftQuote = ' ';   // no quotes
	char const * quoteStart;
	char const * begin = command;

    argsNum = 16;
    argsIdx = 0;

    if( !Utility.newStringArray( &args, argsNum ) )
    {
        return EXECUTION_FAILED;
    }

    const char * i;
    for( i = command; *i; ++i )
	{
		if( *i != '\'' && *i != '"' ) // Not a Quote
		{
            continue;
		}

		if( leftQuote == ' ' ) // reach at a Quote so check if there is a left quote stored or it is the left
		{
			leftQuote = *i; // left quote
			quoteStart = i;
		}
		else if( *i == leftQuote ) // right quote so check it with the left
		{
             // split the string before the quote on spaces and store tokens

            if( !splitOnSpace( begin, quoteStart - 1 ) )
            {
                return EXECUTION_FAILED;
            }
            // store the string between quotes included the quotes
            char * quoteStr;
            if( i - quoteStart - 1 > 0 && ( !Utility.copyStringFromTo( &quoteStr, quoteStart + 1, i - 1 ) ||
                                             !addArgument( quoteStr ) ) )
            {
               return EXECUTION_FAILED;
            }

            begin = i + 1;
			leftQuote = ' ';
		}
		else // right quote doesn't match the left
		{
			return UNBALANCE_QUOTES;
		}
	}

    if( leftQuote != ' ' ) // there is a quote doesn't closed
    {
        return UNBALANCE_QUOTES;
    }

    if( !splitOnSpace( begin, i ) ) // split the last string after quotes
    {
        return EXECUTION_FAILED;
    }

    Command.argsNum = argsIdx;
    Command.args = args;
    Command.args[ argsIdx ] = NULL;

    return SUCCESSFUL;
}
