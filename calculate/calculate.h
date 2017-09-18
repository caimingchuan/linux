
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

#ifndef __CALCULATE_H__
#define __CALAULATE_H__

#ifndef EOK
#define EOK			0
#endif

#define USER		1000
#define	NUM			(USER + 1)
#define SYM			(USER + 2)
#define DOT			(USER + 3)
#define BAR			(USER + 4)

typedef struct
{
	double 	num;
	int 	type;
	char 	data;
} Data;

typedef struct Data_List
{
	struct Data_List *next;
	struct Data_List *prev;
	Data	data;
} DataList, *pDataList;

#endif
