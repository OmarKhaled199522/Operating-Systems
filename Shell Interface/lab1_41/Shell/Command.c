#include "Command.h"

extern const int EMPTY_COMMAND, COMMENT_COMMAND, EXIT_COMMAND, TOO_LONG_COMMAND, SUCCESSFUL, NO_HISTORY, NOT_ENTERED_YET;

extern Utility_t Utility;
extern FILE * inputStream, * historyFile;
extern Parser_t Parser;
extern bool runMode;
extern const bool BATCH_MODE;
char recenetCommand [1000 + 1];

static int read();
static void reset();
static int buildArguments ();
static int commandsNum = 0;
static char commandHistory [5000 + 1][500 + 1];

Command_t Command = { NULL, NULL, 80, 0, 0, read, reset, buildArguments };

void copy_string(char des[], char src[]) {

   int count = 0;

   while (src[count] != '\0') {
      des[count] = src[count];
      count++;
   }
   des[count] = '\0';
}

static int read()
{

    size_t len = Command.maxAllowedLength + 5;
    Command.len = getline( &Command.body, &len, inputStream );
    if( Command.len == -1 )
    {
	printf("\n");
        return EXIT_COMMAND;
    }

    Command.body[ --Command.len ] = '\0';

    int fetchCommand;

    if(strlen(Command.body) == 2){

        if(Command.body[0] == '!' && Command.body[1] >= '0' && Command.body[1] <= '9'){

            fetchCommand = Command.body[1] - '0';

            if(fetchCommand >= commandsNum){

                copy_string(commandHistory[commandsNum++], Command.body);
                return NOT_ENTERED_YET;
            }

            if(commandsNum >= 10){

                fetchCommand += (commandsNum - 10);
            }

            copy_string(Command.body, commandHistory[fetchCommand]);
        }
    }

    copy_string(commandHistory[commandsNum++], Command.body);

    if(strcmp(Command.body , "!!") == 0){

        if(commandsNum == 1){

            return NO_HISTORY;
        }

        copy_string(Command.body, recenetCommand);

    } else {

        copy_string(recenetCommand, Command.body);
    }

    if( runMode == BATCH_MODE )
    {
	printf( "%s\n", Command.body );
    }

    fprintf( historyFile, "%s\n", Command.body );

    if ( Command.len > Command.maxAllowedLength ) // too long command
    {
        return TOO_LONG_COMMAND;
    }
    return SUCCESSFUL;
}

static void reset()
{
   if( Command.body != NULL )
    {
        free( Command.body );
        Command.body = NULL;
    }
    if( Command.args != NULL )
    {
       free( Command.args );
       Command.args = NULL;
    }
    Command.argsNum = 0;
}

static bool isComment()
{
    return Command.body[0] == '#';
}

static int buildArguments()
{
    Utility.leadingTrim( Command.body );
    Command.len = strlen( Command.body );

    if( Command.len == 0 )
    {
        return EMPTY_COMMAND;
    }
    else if( isComment() )
    {
        return COMMENT_COMMAND;
    }

    return Parser.parse( Command.body );
}

