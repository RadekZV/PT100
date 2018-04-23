#ifndef __UTILS_INCLUDED__
#define __UTILS_INCLUDED__

#include "stm32f0xx_hal.h"

#ifdef __GNUC__
# define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
# define PUTCHAR_PROTOTYPE int fputc(int ch, FILE * f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE;

void debug(char str[]);


#endif // __UTILS_INCLUDED__
