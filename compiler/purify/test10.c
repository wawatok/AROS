#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char ** argv)
{
    int len;
    char *str;

    len = strlen (argv[0])-2;
    if (len < 0)
	len = 1;
    str = malloc (len + 1);

    strncpy (str, argv[0], len);
    str[len] = 0;

    printf ("str=%s\n", str);

    return 0;
}
