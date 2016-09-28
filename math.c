#include "math.h"

unsigned int div_ceil(unsigned int num, unsigned int den)
{
	return (num - 1) / den + 1;
}

unsigned int minu(unsigned int a, unsigned int b)
{
	return a < b ? a : b;
}

unsigned int maxu(unsigned int a, unsigned int b)
{
	return a > b ? a : b;
}
