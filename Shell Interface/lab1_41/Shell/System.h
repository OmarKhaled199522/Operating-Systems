#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "Utility.h"
#include "Command.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Expression.h"

typedef struct {
    int (*handleCommand) ();
    bool (*isFileExist) ( const char * filepath );
} System_t;
#endif // SYSTEM_H_INCLUDED
