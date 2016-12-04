#ifndef EXPRESSION_H_INCLUDED
#define EXPRESSION_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>
#include "Utility.h"

typedef struct {
    char name[100], value[100];
} Variable_t;

typedef struct {
    char * (*getVarValue) ( const char * varName );
    bool (*addVariable) ( const char * varName, const char * varValue );
} Expression_t;

#endif // EXPRESSION_H_INCLUDED
