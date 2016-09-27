#include "mem.h"

// Align to the start of next n
unsigned int align_up(unsigned int n, unsigned int a)
{
	unsigned int m = n % a;
	if (m == 0){
		return n;
	}
	return n + (a - m);
}

// Align to the start of this n
unsigned int align_down(unsigned int n, unsigned int a)
{
	return n - (n % a);
}

