// error.c
// Řešení IJC-DU1, příklad A), 17.3.2022
// Autor: Nikita Sniehovskyi, FIT
// Přeloženo: gcc 11

#ifndef _ERROR_C
#define _ERROR_C

#include <stdarg.h>
#include "error.h"


//  warnings to stderr
void warning_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}


//  not critical errors
void error_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}


//  fatal errors
void fatal_error_msg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "FATAL ERROR: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    fprintf(stderr, "Program was stopped.\n");

    va_end(args);
    exit(EXIT_FAILURE);
}

#endif