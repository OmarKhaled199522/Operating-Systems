#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    bool ( *copyStringFromTo ) ( char ** destination, const char * first, const char * last ); // copy the string from first to last into destination
    bool ( *copyString ) ( char ** destination, const char * source ); // create copy string from source to destination
    bool ( *newStringArray ) ( char *** strArray, size_t len ); // create new string array begin form strArray of length len
    bool ( *extendStringArray ) ( char *** strArray, size_t len ); // increase the lenght of strArray to len
    void (*leadingTrim) ( char * str ); // remove leading spaces in str
    int (*splitOnDelim) (char *** tokens, char * str, const char delim[] ); // split the string str into tokens on the delimiters "delim"
    bool (*extendString) ( char ** str, size_t len );
    void (*trailingTrim) ( char * str );
    void (*trim) ( char * str );
} Utility_t;


#endif // UTILITY_H_INCLUDED
