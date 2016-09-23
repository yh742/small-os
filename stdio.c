#include "stdio.h"
#include "stdargs.h"
#include "fb.h"

void printf(char *s, ...)
{
	va_list ap;
	char *p;
	unsigned int uival;
	char *sval;

	va_start(ap, s);
	for (p = s; *p != '\0'; p++)
	{
		if (*p != '%')
		{
			fb_write_byte(p);
			continue;
		}

		switch(*(++p))
		{
			case '%':
				fb_write_byte(p);
				break;
			case 'c':
				uival = va_arg(ap, unsigned int);
				fb_write_byte((char *)&uival);
				break;
			case 'u':
				uival = va_arg(ap, unsigned int);
				fb_write_ui(uival);
				break;
			case 'X':
				uival = va_arg(ap, unsigned int);
				fb_write_hex(uival);
				break;
			case 's':
				sval = va_arg(ap, char*);
				fb_write_s(sval);
				break;	
		}
	}

	va_end(ap);
}
