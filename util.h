#ifndef __UTIL_H_JUN__
#define __UTIL_H_JUN__

#include <stdio.h>
#include <stdlib.h>

#ifdef __DEBUG__
        #define DEBUG(format, ...) \
			do { \
				fprintf(stderr, format, ##__VA_ARGS__); \
			}while(0)
#else
        #define DEBUG(format, ...) do { }while(0)
#endif

#define ERROR(format, ...) \
	do { \
		fprintf(stderr, format, ##__VA_ARGS__); \
		exit(-1); \
	}while(0)

#define INFO(format, ...) \
	do { \
		fprintf(stdout, format, ##__VA_ARGS__); \
	}while(0)

#define CUR_INFO() INFO("%s : %d\n", __FILE__, __LINE__)

#define CUR_ERR() ERROR("%s : %d\n", __FILE__, __LINE__)

#endif
