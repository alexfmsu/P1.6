/* copyright @ M. Baricevic 2014 :-) */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define L 10000000
int main(int argc , char const * const * argv)
{
const char * a = "AGCT";
size_t l = 10000000;
register size_t i = 0 , j = 0;
register int r = 0;
srand(0);
for ( i = 0 ; i < l ; i++ )
{
for ( j = 0 ; j < 256 ; j++ )
{
r = rand();
putchar(a[r%4]);
putchar(' ');
}
putchar('\n');
}
return 0;
}
