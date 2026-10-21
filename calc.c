//Giorgi Janikashvili Student ID: 251ADB123
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: <every AI tool, website and book you used; details in comments where you used them>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct{
    const char *src;
    size_t pos;
    size_t len;
    size_t error;
    int failed;
} Parser;

static void fail_at(Parser *p, size_t pos)
{
    if(!p->failed)
    {
        p->failed = 1;
        p->error = pos;
    }
}
