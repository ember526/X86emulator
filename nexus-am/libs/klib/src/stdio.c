#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
    char out[512];
	  va_list arg;
    int done;
    va_start (arg, fmt);
    done = vsprintf (out, fmt, arg);
    va_end (arg);
    for (int i = 0; out[i] != '\0'; ++i)
    {
      _putc(out[i]);
    }
    return done;
}

 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 void myitoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }

 void myhtoa(int n, char s[])
 {
     int i, sign, tmp;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         tmp = n % 16;
         s[i++] = tmp > 9 ? 'a' + tmp - 10 : '0' + tmp;   /* get next digit */
     } while ((n /= 16) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }

int vsprintf(char *out, const char *fmt, va_list ap) {

    int int_temp;
    char char_temp;
    char *string_temp;
    //double double_temp;

    char ch;
    int length = 0;

    char buffer[512];
    *out = '\0';
    while ( (ch = *fmt++ )) {
        if ( '%' == ch ) {
            switch (ch = *fmt++) {
                /* %% - print out a single %    */
                case '%':
                    strcat(out, "%");
                    length++;
                    break;

                /* %c: print out a character    */
                case 'c':
                    char_temp = va_arg(ap, int);
                    buffer[0] = char_temp;
                    buffer[1] = '\0';
                    strcat(out, buffer);
                    length++;
                    break;

                /* %s: print out a string       */
                case 's':
                    string_temp = va_arg(ap, char *);
                    strcat(out, string_temp);
                    length += strlen(string_temp);
                    break;

                /* %d: print out an int         */
                case 'd':
                    int_temp = va_arg(ap, int);
                    myitoa(int_temp, buffer);
                    strcat(out, buffer);
                    length += strlen(buffer);
                    break;

                /* %x: print out an int in hex  */
                case 'x':
                    int_temp = va_arg(ap, int);
                    myhtoa(int_temp, buffer);
                    strcat(out, buffer);
                    length += strlen(buffer);
                    break;
//
                //case 'f':
                //    double_temp = va_arg(ap, double);
                //    ftoa_fixed(buffer, double_temp);
                //    strcat(out, buffer);
                //    //length += strlen(buffer);
                //    break;
				//
                //case 'e':
                //    double_temp = va_arg(ap, double);
                //    ftoa_sci(buffer, double_temp);
                //    strcat(out, buffer);
                //    //length += strlen(buffer);
                //    break;
            }//
        }
        else {
            buffer[0] = ch;
            buffer[1] = '\0';
            strcat(out, buffer);
            length++;
        }
    }
    return strlen(out);
}

int sprintf(char *out, const char *fmt, ...) {
	va_list arg;
  	int done;
  	va_start (arg, fmt);
  	done = vsprintf (out, fmt, arg);
  	va_end (arg);
  	return done;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
	assert(0);
  return 0;
}

#endif
