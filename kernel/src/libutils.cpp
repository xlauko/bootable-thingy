#include "libutils.h"

extern "C" {
void clear (void)
{
	for ( int i = 0; i < COLUMNS * LINES * 2; ++i )
		video[ i ] = 0;

	xpos = 0;
   	ypos = 0;
}

void itoa (char *buf, int base, int d)
{
	char *p = buf;
   	char *p1, *p2;
   	unsigned long ud = d;
   	int divisor = 10;

   	if (base == 'd' && d < 0) {
		*p++ = '-';
	   	buf++;
	   	ud = -d;
	} else if (base == 'x')
	 	divisor = 16;

	do {
		int remainder = ud % divisor;
 		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
   	while (ud /= divisor);

   	*p = 0;

   	p1 = buf;
   	p2 = p - 1;
   	while (p1 < p2) {
		char tmp = *p1;
	   	*p1 = *p2;
	   	*p2 = tmp;
	   	p1++;
	   	p2--;
	}
}

void putchar( int c )
{
    const int columns = 80;
    const int lines = 24;
    static int x = 0, y = 0;

    if ( c == '\n' || x >= columns )
        x = 0, y++;

    if ( c == '\n' )
        return;

    if ( y >= lines )
        y = 0;

    int idx = ( x + y * columns ) * 2;
    video[ idx ] = c & 0xff;
    video[ idx + 1 ] = 7; /* vga attribute */

    ++ x;
}

void puts( const char *str )
{
    do putchar( *str ); while ( *str++ );
    putchar( '\n' );
}


void printf (const char *format, ...)
{
	char **arg = (char **) &format;
	int c;
	char buf[20];

	arg++;

	while ((c = *format++) != 0) {
   		if (c != '%')
	 		putchar (c);
   		else {
	   		char *p, *p2;
	   		int pad0 = 0, pad = 0;

	   		c = *format++;
	   		if (c == '0') {
		   		pad0 = 1;
		   		c = *format++;
		 	}

	   		if (c >= '0' && c <= '9') {
		   		pad = c - '0';
		   		c = *format++;
		 	}

	   		switch (c) {
		 		case 'd':
		 		case 'u':
		 		case 'x':
		   			itoa (buf, c, *((int *) arg++));
		   			p = buf;
		   			goto string;
		   			break;

		 		case 's':
		   			p = *arg++;
		 		string:
		   			for (p2 = p; *p2; p2++);
		   			for (; p2 < p + pad; p2++)
			 			putchar (pad0 ? '0' : ' ');
		   			while (*p)
			 			putchar (*p++);
		   			break;

		 		default:
		   			putchar (*((int *) arg++));
		   			break;
		 	}
	 	}
 	}
}

}