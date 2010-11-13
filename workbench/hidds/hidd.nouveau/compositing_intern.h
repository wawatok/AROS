#ifndef _COMPOSITING_INTERN_H
#define _COMPOSITING_INTERN_H
/*
    Copyright � 2010, The AROS Development Team. All rights reserved.
    $Id$
*/

#include "compositing.h"

#include <exec/lists.h>

struct _Rectangle
{
    WORD MinX;
    WORD MinY;
    WORD MaxX;
    WORD MaxY;
};

struct StackBitMapNode
{
    struct Node         n;
    OOP_Object *        bm;
    struct _Rectangle   screenvisiblerect;
    BOOL                isscreenvisible;
};

struct HIDDCompositingData
{
    OOP_Object          *screenbitmap;
    HIDDT_ModeID        screenmodeid;
    struct _Rectangle   screenrect;

    struct List         bitmapstack;
};

#define METHOD(base, id, name) \
  base ## __ ## id ## __ ## name (OOP_Class *cl, OOP_Object *o, struct p ## id ## _ ## name *msg)

#define BASE(lib)                   ((LIBBASETYPEPTR)(lib))

#define SD(cl)                      (&BASE(cl->UserData)->sd)


#endif /* _COMPOSITING_INTERN_H */
