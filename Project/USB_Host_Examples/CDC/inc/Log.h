#ifndef	LOG_H
#define	LOG_H

typedef	struct	Struct_Log{
	int (*d)(const char * __restrict /*format*/, ...)	;
	int (*e)(const char * __restrict /*format*/, ...)	;
} TLog;

extern		TLog	Log	;
#endif
