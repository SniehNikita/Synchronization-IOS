// error.c
// Řešení IJC-DU1, příklad A), 17.3.2022
// Autor: Nikita Sniehovskyi, FIT
// Přeloženo: gcc 11

#ifndef _ERROR_H
#define _ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//  warnings to stderr
void warning_msg(const char *fmt, ...);

//  not critical errors
void error_msg(const char *fmt, ...);

//  fatal errors, stops program
void fatal_error_msg(const char *fmt, ...);

#endif