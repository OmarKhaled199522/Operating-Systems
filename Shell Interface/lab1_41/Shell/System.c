#include "System.h"

static int handleCommand();
static int executeCommand();
static bool isFileExist( const char * filePath );
static bool setCommandPath( char * filePath );
static bool searchInEnviroment( char * filePath );
static void setFullPath( const char * folder, const char * file );
static void setRunningMode();
static int handle_Cd_Command();
static int handle_Echo_Command();
static int handle_Expression_Command();
static int handle_history_Command();

extern Utility_t Utility;
extern Command_t Command;
extern Expression_t Expression;
extern FILE * historyFile;
extern const int TOO_MANY_ARGUMENTS, SUCCESSFUL, DIRECTORY_NOT_FOUND, VARIABLE_NOT_FOUND,
                 EXECUTION_FAILED, COMMAND_NOT_FOUND, EXIT_COMMAND, INVALID_EXPRESSION;

char ** envPaths;  // enviroment paths
int envPathsNum;
char fullPath[100];
bool executeMode;
const bool FOREGROUND_MODE = 1, BACKGROUND_MODE = 0;

System_t System = { handleCommand, isFileExist };

static int executeCommand()
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid == 0)   // Child process
    {
        if ( execv( Command.args[0], Command.args ) == -1 ) return EXECUTION_FAILED;
    }
    else if ( pid < 0 ) return EXECUTION_FAILED;
    else // Parent process
    {
        if( executeMode == FOREGROUND_MODE )
        {
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while ( !WIFEXITED(status) && !WIFSIGNALED(status) );
        }
    }

    return SUCCESSFUL;
}

static int handleCommand()
{
    setRunningMode();
    
    if( strcmp( Command.args[0], "exit" ) == 0 ) return EXIT_COMMAND;
    if( strcmp( Command.args[0], "cd" ) == 0 )  return handle_Cd_Command();
    if( strcmp( Command.args[0], "echo" ) == 0 ) return handle_Echo_Command();
    if( strcmp( Command.args[0], "history" ) == 0 ) return handle_history_Command();

    if( !setCommandPath( Command.args[0] ) )
    {
        return handle_Expression_Command();
    }
    else
    {
        return executeCommand();
    }

}

static int handle_history_Command()
{
    fseek( historyFile, 0, SEEK_SET );
     char * line;
     int lineNum = 0;
     size_t len = Command.maxAllowedLength + 10;
     while( getline( &line, &len, historyFile ) != -1 )
     {
        printf( "%d %s", lineNum++, line );
     }

     return SUCCESSFUL;
}
static int handle_Expression_Command()
{
    if( Command.argsNum > 2 ) return TOO_MANY_ARGUMENTS;
    if( Command.argsNum == 2 )
    {
        size_t firstArgLen = strlen( Command.args[0] );
        if( firstArgLen >= 2 && Command.args[0][firstArgLen - 1] == '=' )
        {
            Command.args[0][firstArgLen - 1] = '\0';
            if( !Expression.addVariable( Command.args[0], Command.args[1] ) ) return EXECUTION_FAILED;
        }
	else
	{
	    return INVALID_EXPRESSION;
	}
        return SUCCESSFUL;
    }
    else
    {
        char ** operands;
        int operandsNum;
        if( (operandsNum = Utility.splitOnDelim( &operands, Command.args[0], "=" ) ) == -1 ) return EXECUTION_FAILED;
        if( operandsNum != 2 ) return COMMAND_NOT_FOUND;
        if( !Expression.addVariable( operands[0], operands[1] ) ) return EXECUTION_FAILED;
        free( operands );
        return SUCCESSFUL;
    }

    return COMMAND_NOT_FOUND;
}

static int handle_Echo_Command()
{
    for( int i = 0; i < Command.argsNum; i++ )
    {
        if( Command.args[i][0] == '$' )
        {
            char * value = Expression.getVarValue( Command.args[i] + 1 );
            if( value == NULL )
            {
                return VARIABLE_NOT_FOUND;
            }
            char * tmp;
            if( !Utility.copyString( &tmp, value ) ) return EXECUTION_FAILED;
            Command.args[i] = tmp;
        }
    }

    setCommandPath( Command.args[0] );
    return executeCommand();
}

static int handle_Cd_Command()
{
    if( Command.argsNum == 1 ) return SUCCESSFUL;
    if( Command.argsNum > 2 ) return TOO_MANY_ARGUMENTS;

    char * currDir = Command.args[1];

    if ( strcmp( currDir, "~" ) == 0 )
    {
        currDir = getenv( "HOME" );
    }

    if( !isFileExist( currDir ) ) return DIRECTORY_NOT_FOUND;

    chdir( currDir );

    return SUCCESSFUL;
}

static void setRunningMode()
{
    char * lastArgument = Command.args[ Command.argsNum - 1 ];
    size_t len = strlen( lastArgument );

    if( strcmp( lastArgument, "&" ) == 0 )
    {
        executeMode = BACKGROUND_MODE;
        Command.args[ Command.argsNum - 1 ] = NULL;
    }
    else if( lastArgument[ len - 1 ] == '&' )
    {
        executeMode = BACKGROUND_MODE;
        lastArgument[ len - 1 ] = '\0';
    }
    else
    {
        executeMode = FOREGROUND_MODE;
    }
}

static bool setCommandPath( char * filePath )
{
    if( isFileExist( filePath ) ) // check if file is exist so execute it immediately
    {
        return true;
    }

    return searchInEnviroment( filePath );
}

static bool isFileExist( const char * filePath )
{
    return access( filePath, F_OK ) == 0;
}

static bool searchInEnviroment( char * filePath )
{
    char * envPath;
    bool found = false;

    if( !Utility.copyString( &envPath, getenv( "PATH" ) ) ) return false;
    if( ( envPathsNum = Utility.splitOnDelim( &envPaths, envPath, ":" ) ) == -1 ) return false;

    for( int i = 0; i < envPathsNum; i++ )
    {
        setFullPath( envPaths[i], filePath );

        if( isFileExist( fullPath ) )
        {
            Command.args[0] = fullPath;
            found = true;
            break;
        }
    }

    free( envPath );

    return found;
}

static void setFullPath( const char * folder, const char * file )
{
    sprintf( fullPath, "%s/%s", folder, file );
}
