/*
 * qerror/qabort - Standard error handling
 * Copyright (c) 1990-2003 - D J Koopman, all rights reserved
 *
 * $Id: qerror.c,v 1.19 2007/02/22 17:34:39 djk Exp $
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "qerror.h"


/*
 * qlog() - general purpose log a message routine, which returns
 */
void qlog(const char *fmt, ...)
{
        va_list vargs;

        va_start(vargs, fmt);
        vfprintf(stdout, fmt, vargs);
        va_end(vargs);
        fflush(stdout);
}


/*
 * qerror() - the general purpose error routine that exits with -1
 */
void qerror(const char *fmt, ...)
{
        va_list vargs;

        va_start(vargs, fmt);
        vfprintf(stdout, fmt, vargs);
        va_end(vargs);
        fflush(stdout);
        exit(-1);
}


/*
 * qabort() - the general purpose error routines with abort()
 */
void qabort(const char *fmt, ...)
{
        va_list vargs;

        va_start(vargs, fmt);
        vfprintf(stdout, fmt, vargs);
        va_end(vargs);
        fflush(stdout);
        abort();
}
