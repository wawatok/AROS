/*
    Copyright (C) 1995-2005, The AROS Development Team. All rights reserved.
*/

#include <exec/types.h>

#define G(o) ((struct Gadget *)(o))

struct BinaryData
{
    APTR	Pool;
    ULONG	Flags;
    LONG	NumLines;
    WORD	OffsetSize;
    WORD	LineSize;
};
