/*
    Copyright � 1995-2002, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Bullet function ObtainInfoA()
    Lang: English
*/

/*****************************************************************************

    NAME */

	AROS_LH2(ULONG, ObtainInfoA,

/*  SYNOPSIS */
    	AROS_LHA(struct GlyphEngine *, glyphEngine, A0),
	AROS_LHA(struct TagItem *, tagList, A1),
	
/*  LOCATION */
	struct Library *, BulletBase, 8, Bullet)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,BulletBase)

    AROS_LIBFUNC_EXIT

} /* ObtainInfoA */
