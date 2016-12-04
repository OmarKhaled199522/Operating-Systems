#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Command.h"
#include "Parser.h"
#include "System.h"

extern const int UNBALANCE_QUOTES, EXECUTION_FAILED, SUCCESSFUL, EMPTY_COMMAND, COMMENT_COMMAND, EXIT_COMMAND,
                 TOO_MANY_ARGUMENTS, DIRECTORY_NOT_FOUND, VARIABLE_NOT_FOUND, TOO_LONG_COMMAND, COMMAND_NOT_FOUND,
                INVALID_EXPRESSION, NO_HISTORY, NOT_ENTERED_YET;

extern Command_t Command;
extern Parser_t Parser;
extern System_t System;

void initialize( int argc, char * argv[] );
bool testEnviromentArgs( int argc, char ** argv );
bool handleMessage( int message );
void run();
void terminate();
void printInLogFile( int signalNum );

FILE * inputStream;
bool runMode;
const bool INTERACTIVE_MODE = 1, BATCH_MODE = 0;
FILE * historyFile, * logFile;

int main( int argc, char * argv[] )
{
    initialize( argc, argv );
    run();
    terminate();
    return EXIT_SUCCESS;
}

void initialize( int argc, char * argv[] )
{
    if( !testEnviromentArgs( argc, argv ) )
    {
        runMode = INTERACTIVE_MODE;
        inputStream = stdin;
    }

    char filePath[100];
    sprintf( filePath, "%s/ShellFiles/history.txt", getenv( "HOME" ) );
    historyFile = fopen( filePath, "r+" );
    sprintf( filePath, "%s/ShellFiles/log.txt", getenv( "HOME" ) );
    logFile = fopen( filePath, "w" );
}

void terminate()
{
    fclose( historyFile );
    fclose( logFile );
    if( inputStream != stdin )  fclose( inputStream );
}

bool testEnviromentArgs( int argc, char ** argv )

{
    if( argc > 2 )
    {
        fprintf( stderr, "Too many Arguments\n" );
        return false;
    }
    else if( argc < 2 )
    {
        return false;
    }

    if( access( argv[1], F_OK  ) == -1 ) // check existance of the batch file
    {
        fprintf( stderr, "File not found!\nProgram begins \"in Interactive Mode\"\n" );
        return false;
    }
    else if( access( argv[1], R_OK ) == -1 ) // check if the file is accessible to read
    {
        fprintf( stderr, "File not accessible for reading!\n" );
        return false;
    }

    inputStream = fopen( argv[1], "r" );
    if( inputStream == NULL )
    {
        fprintf( stderr, "Failed to open the file!\n" );
        return false;
    }
    runMode = BATCH_MODE;

    return true;
}

bool handleMessage( int message )
{
    int exit = false;

    if( message == EXIT_COMMAND ) exit = true;
    else if( message == TOO_LONG_COMMAND ) fprintf( stderr,"Too long command\n" );
    else if( message == UNBALANCE_QUOTES ) fprintf( stderr, "There 's unbalanced quotes!\n" );
    else if( message == EXECUTION_FAILED ) fprintf( stderr, "Failed in execution!\n" );
    else if( message == TOO_MANY_ARGUMENTS ) fprintf( stderr, "Too many arguments\n" );
    else if( message == DIRECTORY_NOT_FOUND ) fprintf( stderr, "cd: %s: No such file or directory!\n", Command.args[1] );
    else if( message == VARIABLE_NOT_FOUND ) fprintf( stderr, "No such variable!\n" );
    else if( message == COMMAND_NOT_FOUND ) fprintf( stderr, "%s : No such command!\n", Command.body );
    else if( message == INVALID_EXPRESSION ) fprintf( stderr, "Invalid Expression!\n" );
    else if(message == NO_HISTORY) fprintf( stderr, "No commands in history\n" );
    else if(message == NOT_ENTERED_YET) fprintf( stderr, "No such command in history\n");
    return exit;
}

void run()
{
    bool finish;
    int message;
    do
    {
        printf( "Shell> " );

        if( ( message = Command.read() ) != SUCCESSFUL )
        {
            finish = handleMessage( message );
        }
        else if( ( message = Command.buildArguments() ) != SUCCESSFUL )
        {
            finish = handleMessage( message );
        }
        else if( ( message = System.handleCommand() ) != SUCCESSFUL )
        {
            finish = handleMessage( message );
        }
        else
        {
            finish = false;
        }

        Command.reset();

    }while( !finish );
}

void printInLogFile( int signalNum )
{
    fprintf( logFile, "Child process was terminated\n" );
}

