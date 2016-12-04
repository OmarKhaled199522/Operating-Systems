#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Command.h"
#include "Utility.h"

typedef struct {
    int (*parse) ( char const * command );
} Parser_t;

#endif // PARSER_H_INCLUDED
