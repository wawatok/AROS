/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id: preparecontext.c 34764 2010-10-15 15:04:08Z jmcmullan $

    Desc: PrepareContext() - Prepare a task context for dispatch, ARM version.
    Lang: english
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <utility/tagitem.h>
#include <proto/kernel.h>
#include <aros/arm/cpucontext.h>

#include "etask.h"
#include "exec_intern.h"
#include "exec_util.h"

AROS_LH4(BOOL, PrepareContext,
	 AROS_LHA(VOLATILE struct Task *, task,       A0),
	 AROS_LHA(APTR,                   entryPoint, A1),
	 AROS_LHA(APTR,                   fallBack,   A2),
	 AROS_LHA(struct TagItem *,       tagList,    A3),
	 struct ExecBase *, SysBase, 6, Exec)
{
    AROS_LIBFUNC_INIT

    struct ExceptionContext *ctx;

    if (!(task->tc_Flags & TF_ETASK) )
	return FALSE;

    ctx = KrnCreateContext();
    GetIntETask (task)->iet_Context = ctx;
    if (!ctx)
	return FALSE;

    /* Set up function arguments */
    while(tagList)
    {
    	switch(tagList->ti_Tag)
	{
	    case TAG_MORE:
	    	tagList = (struct TagItem *)tagList->ti_Data;
		continue;
		
	    case TAG_SKIP:
	    	tagList += tagList->ti_Data;
		break;
		
	    case TAG_DONE:
	    	tagList = NULL;
    	    	break;

#define HANDLEARG(x)				   \
	    case TASKTAG_ARG ## x:		   \
	        ctx->r[3 + x - 1] = tagList->ti_Data; \
	        break;
		
	    HANDLEARG(1)
	    HANDLEARG(2)
	    HANDLEARG(3)
	    HANDLEARG(4)
	    HANDLEARG(5)
	    HANDLEARG(6)
	    HANDLEARG(7)
	    HANDLEARG(8)
	}
	
	if (tagList) tagList++;
    }

    /* Now prepare return address */
    ctx->lr = (ULONG)fallBack;
    
    /* Then set up the frame to be used by Dispatch() */
    ctx->sp = (ULONG)task->tc_SPReg;
    ctx->pc = (ULONG)entryPoint;

    return TRUE;

    AROS_LIBFUNC_EXIT
} /* PrepareContext() */
