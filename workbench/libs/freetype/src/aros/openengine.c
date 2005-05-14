/*
 * Based on the code from the ft2.library from MorphOS, the ttf.library by
 * Richard Griffith and the type1.library by Amish S. Dave
 */
#include "ftglyphengine.h"

#include <aros/debug.h>
#include <aros/symbolsets.h>
#include <aros/libcall.h>

#include <proto/exec.h>

#include LC_LIBDEFS_FILE

static struct Library *mylib;

AROS_LH0(struct GlyphEngine *, OpenEngine,
	 LIBBASETYPEPTR, LIBBASE, 5, FreeType2
)
{
    AROS_LIBFUNC_INIT

    static UBYTE *EngineName = "freetype2";
    FT_GlyphEngine *ge=NULL;

    D(bug("OpenEngine libbase = 0x%lx\n", mylib));

    if((ge = AllocGE()))
    {
	ge->gle_Library = mylib;
	ge->gle_Name = EngineName;

	D(bug(" returning FT_GlyphEngine 0x%lx\n",ge));

	return (struct GlyphEngine *)ge;
    }

    D(bug(" return NULL\n"));
    return NULL;
    
    AROS_LIBFUNC_EXIT
}

static AROS_SET_LIBFUNC(getbase, struct Library *, FTBase)
{
    AROS_SET_LIBFUNC_INIT;
    mylib = FTBase;
    
    return TRUE;
    AROS_SET_LIBFUNC_EXIT;
}

ADD2INITLIB(getbase, 0);
