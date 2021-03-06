/*
    Copyright (C) 1995-2015, The AROS Development Team. All rights reserved.
*/

#include <proto/alib.h>
#include <dos/rdargs.h>
#include <proto/dos.h>


int __nocommandline = 1;

/* No newline in the end! Important! */
static char text[] = "WORD1 WORD2 WORD3";
static char buf[256];

int main(void)
{
    struct CSource cs;
    int i;
    LONG result = RETURN_OK, res;

    cs.CS_Buffer = text;
    cs.CS_Length = sizeof(text) - 1;
    cs.CS_CurChr = 0;

    i = 1;
    do
    {
        res = ReadItem(buf, sizeof(buf), &cs);

        Printf("Step %ld, result %ld, buffer %s, CurChr %ld\n", i++, res,
            buf, cs.CS_CurChr);
        if (i == 10)
        {
            Printf("ERROR: Unrecoverable loop detected!\n");
            result = RETURN_ERROR;
            break;
        }
    }
    while (res != ITEM_NOTHING);

    return result;
}
