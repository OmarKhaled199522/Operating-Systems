#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include "Utility.h"
#include "Parser.h"

typedef struct {
    char * body; // command line
    char ** args; // arguments after spliting the body
    size_t maxAllowedLength; // max length of command line ( body )
    int len; // lenth of the command line
    size_t argsNum; // arguments number
    int (*read) (); // read the command from the file "IN"
    void (*reset) (); // reset the variable of Command
    int (*buildArguments) (); // build the arguments from the command line (body)
} Command_t;


#endif // COMMAND_H_INCLUDED
