/*
 * qerror.h -- hader file for qerror() and qabort()
 */

#ifndef _QERROR_H
#define _QERROR_H

void qlog(const char *, ...);
void qerror(const char *, ...);
void qabort(const char *, ...);

#endif
