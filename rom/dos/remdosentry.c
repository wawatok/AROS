/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include <dos/dosextens.h>
#include <proto/utility.h>

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH1(LONG, RemDosEntry,

/*  SYNOPSIS */
	AROS_LHA(struct DosList *, dlist, D1),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 112, Dos)

/*  FUNCTION
	Removes a given dos list entry from the dos list. Automatically
	locks the list for writing.

    INPUTS
	dlist - pointer to dos list entry.

    RESULT
	!=0 if all went well, 0 otherwise.

    NOTES
	Since anybody who wants to use a device or volume node in the
	dos list has to lock the list, filesystems may be called with
	the dos list locked. So if you want to add a dos list entry
	out of a filesystem don't just wait on the lock but serve all
	incoming requests until the dos list is free instead.

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY
	29-10-95    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *,DOSBase)
    struct DosList *dl;

    if(dlist == NULL)
	return 0;

    dl = LockDosList(LDF_ALL | LDF_WRITE);

    while(TRUE)
    {
        if(dl->dol_Next == dlist)
	{
	    dl->dol_Next = dlist->dol_Next;
	    break;
	}

	dl = dl->dol_Next;
    }

    UnLockDosList(LDF_ALL | LDF_WRITE);
    
    return 1;

    AROS_LIBFUNC_EXIT
} /* RemDosEntry */
