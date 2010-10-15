/*
    Copyright � 1995-2007, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Wait for the Blitter to finish
    Lang: english
*/
#include <aros/debug.h>
#include "graphics_intern.h"

/*****************************************************************************

    NAME */
	#include <clib/graphics_protos.h>

	AROS_LH0(void, WaitBlit,

/*  SYNOPSIS */
	/* void */

/*  LOCATION */
	struct GfxBase *, GfxBase, 38, Graphics)

/*  FUNCTION
	Wait for the blitter to return to finish, ie. the function returns
	when the blitter is idle.

    INPUTS
	None.

    RESULT
	None.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT


    /* TODO: Write graphics/WaitBlit() */
    aros_print_not_implemented ("WaitBlit");

/*    driver_WaitBlit (GfxBase); */

    AROS_LIBFUNC_EXIT
} /* WaitBlit */
