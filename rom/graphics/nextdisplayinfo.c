/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Graphics function NextDisplayInfo()
    Lang: english
*/
#include <graphics/displayinfo.h>
#include <hidd/graphics.h>
#include <proto/oop.h>
#include "graphics_intern.h"
#include "dispinfo.h"

/*****************************************************************************

    NAME */
#include <proto/graphics.h>

        AROS_LH1(ULONG, NextDisplayInfo,

/*  SYNOPSIS */
        AROS_LHA(ULONG, last_ID, D0),

/*  LOCATION */
        struct GfxBase *, GfxBase, 122, Graphics)

/*  FUNCTION
	Go to next entry in the DisplayInfo database.

    INPUTS
        last_ID - previous displayinfo identifier
                  or INVALID_ID if beginning iteration

    RESULT
        next_ID - subsequent displayinfo identifier
                  or INVALID_ID if no more records

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        FindDisplayInfo(), GetDisplayInfoData(), graphics/displayinfo.h

    INTERNALS

    HISTORY


******************************************************************************/
{
    AROS_LIBFUNC_INIT

    OOP_Object *sync, *pixfmt;
    
    HIDDT_ModeID hiddmode;
    ULONG id;
    
    hiddmode = (HIDDT_ModeID)AMIGA_TO_HIDD_MODEID(last_ID);
    
    /* Get the next modeid */
    hiddmode = HIDD_Gfx_NextModeID(SDD(GfxBase)->gfxhidd, hiddmode, &sync, &pixfmt);
    
    id = HIDD_TO_AMIGA_MODEID(hiddmode);
    
    if (id != INVALID_ID) {
        /* Some old software (DirOpus for example) rely on
           getting HIRES_KEY for modes which are >= 640 pixels wide
	   and can have a smaller bitmap - sonic */
	IPTR width, minwidth;
	
        OOP_GetAttr(sync, aHidd_Sync_HDisp, &width);
	OOP_GetAttr(sync, aHidd_Sync_HMin, &minwidth);
        if ((width >= 640) && (minwidth < 640))
            id |= HIRES_KEY;
    }
    
    return id;

    AROS_LIBFUNC_EXIT
} /* NextDisplayInfo */
