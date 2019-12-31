#ifndef _ISSHE_STDIO_H_
#define _ISSHE_STDIO_H_

#include "isshe_common.h"

char *isshe_fgets(char *ptr, int n, FILE *stream);

void isshe_fputs(const char *ptr, FILE *stream);

#endif