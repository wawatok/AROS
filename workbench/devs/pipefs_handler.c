/*
    (C) 1995-98 AROS - The Amiga Research OS
    $Id$
    $Log$
    Revision 1.12  2001/07/16 19:22:41  falemagn
    The FSA_CREATE_DIRECTORY actoon didn't return a valid filehandle: FIXED. Implemented deleting.

    Revision 1.10  2001/07/16 15:21:32  falemagn
    File types weren't reported correctly

    Revision 1.4  2001/07/15 21:12:24  falemagn
    Ooops... forgot to do merge with Stefan changes...

    Revision 1.3  2001/07/15 20:52:23  falemagn
    there was a silly bug. I fixed it, but it looks hacky... I think I'll have to look at it again. One of these days :)

    Revision 1.2  2001/07/15 20:16:38  falemagn
    Implemented named pipes. Actually there are ONLY named pipes. The standard AmigaOS PIPE: can be implemented assigning PIPE: to PIPEFS:namedpipe. pipe() support is about to come


    Desc: A PIPE filesystems, in which named and unnamed pipes can be created.
    Lang: English

    History:

    2001/07/14 falemagn created

*/
#define AROS_ALMOST_COMPATIBLE

#include <exec/errors.h>
#include <exec/resident.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <dos/dosextens.h>
#include <dos/filesystem.h>
#include <proto/dos.h>
#include <aros/libcall.h>
#include <aros/asmcall.h>
#ifdef __GNUC__
#include "pipefs_handler_gcc.h"
#endif

#include <string.h>
#include <stddef.h>

//#define kprintf(x...)

static const char name[];
static const char version[];
static const APTR inittabl[4];
static void *const functable[];
struct pipefsbase *AROS_SLIB_ENTRY(init,pipefs_handler)();
void AROS_SLIB_ENTRY(open,pipefs_handler)();
BPTR AROS_SLIB_ENTRY(close,pipefs_handler)();
BPTR AROS_SLIB_ENTRY(expunge,pipefs_handler)();
int AROS_SLIB_ENTRY(null,pipefs_handler)();
void AROS_SLIB_ENTRY(beginio,pipefs_handler)();
LONG AROS_SLIB_ENTRY(abortio,pipefs_handler)();
static const char end;

AROS_UFP3(LONG, pipefsproc,
    AROS_UFPA(char *,argstr,A0),
    AROS_UFPA(ULONG,argsize,D0),
    AROS_UFPA(struct ExecBase *,SysBase,A6));

struct pipefsmessage
{
    struct Message    msg;
    struct IOFileSys *iofs;
    LONG              curlen;
};

struct dirnode
{
    struct MinNode   node;
    struct dirnode  *parent;     /* Parent directory */
    STRPTR           name;
    LONG             type;
    struct DateStamp datestamp;
    ULONG            numusers;
    struct List      files;
};

struct filenode
{
    struct MinNode   node;
    struct dirnode  *parent;
    STRPTR           name;
    LONG             type;
    struct DateStamp datestamp;
    ULONG            numusers;           /* Number of actual users of this pipe */
    ULONG            numwriters;         /* Num of actual writers */
    ULONG            numreaders;         /* Num of actual readers */
    struct List      pendingwrites;      /* List of pending write requestes */
    struct List      pendingreads;       /* List of pending read requestes */
};
/*
   Abuse of pendingreads so that it's used as waiting list either for
   readers or writers before respectively a writer or a reader becomes
   available
*/
#define waitinglist pendingreads

struct usernode
{
    struct Node      node;
    struct filenode *fn;
    ULONG            mode;
};

static STRPTR           SkipColon    (STRPTR str);
static size_t           LenFirstPart (STRPTR path);
static struct filenode *FindFile     (struct dirnode   **dn_ptr,      STRPTR path);
static struct filenode *GetFile      (struct pipefsbase  *pipefsbase, STRPTR filename, struct dirnode *dn, ULONG mode, ULONG *err);
static ULONG            SendRequest  (struct pipefsbase  *pipefsbase, struct IOFileSys *iofs);
static STRPTR           StrDup       (struct pipefsbase  *pipefsbase, STRPTR str);


int entry(void)
{
    /* If the handler was executed by accident return error code. */
    return -1;
}

const struct Resident pipefs_handler_resident=
{
    RTC_MATCHWORD,
    (struct Resident *)&pipefs_handler_resident,
    (APTR)&end,
    RTF_AUTOINIT,
    41,
    NT_DEVICE,
    0,
    (char *)name,
    (char *)&version[6],
    (ULONG *)inittabl
};

static const char name[]="pipefs.handler";

static const char version[]="$VER: pipefs-handler 41.1 (8.6.96)\r\n";

static const APTR inittabl[4]=
{
    (APTR)sizeof(struct pipefsbase),
    (APTR)functable,
    NULL,
    &AROS_SLIB_ENTRY(init,pipefs_handler)
};

static void *const functable[]=
{
    &AROS_SLIB_ENTRY(open,pipefs_handler),
    &AROS_SLIB_ENTRY(close,pipefs_handler),
    &AROS_SLIB_ENTRY(expunge,pipefs_handler),
    &AROS_SLIB_ENTRY(null,pipefs_handler),
    &AROS_SLIB_ENTRY(beginio,pipefs_handler),
    &AROS_SLIB_ENTRY(abortio,pipefs_handler),
    (void *)-1
};

AROS_LH2(struct pipefsbase *, init,
AROS_LHA(struct pipefsbase *, pipefsbase, D0),
AROS_LHA(BPTR,              segList,  A0),
	 struct ExecBase *, sysBase, 0, pipefs_handler)
{
    AROS_LIBFUNC_INIT

    /* Store arguments */
    SysBase =  sysBase;
    DOSBase =  (struct DosLibrary *)OpenLibrary("dos.library",39);
    pipefsbase->seglist=segList;

    if(DOSBase)
    {
	struct TagItem taglist[]=
	{
	 {NP_Entry,              (IPTR)pipefsproc},
	 {NP_Name, (IPTR)"pipefs.handler process"},
	 {NP_UserData,           (IPTR)pipefsbase},
	 {TAG_DONE,                           0}
	};

	pipefsbase->proc = CreateNewProc(taglist);

       	if (pipefsbase->proc)
	    return pipefsbase;

        CloseLibrary((struct Library *)DOSBase);
    }

    return NULL;
    AROS_LIBFUNC_EXIT
}

AROS_LH3(void, open,
 AROS_LHA(struct IOFileSys *, iofs, A1),
 AROS_LHA(ULONG,              unitnum, D0),
 AROS_LHA(ULONG,              flags, D1),
	   struct pipefsbase *, pipefsbase, 1, pipefs_handler)
{
    AROS_LIBFUNC_INIT
    struct usernode *un;
    struct dirnode  *dn;

    /* Get compiler happy */
    unitnum=flags=0;

    /* Mark Message as recently used. */
    iofs->IOFS.io_Message.mn_Node.ln_Type=NT_REPLYMSG;

    /* Build a fake usernode */
    un = AllocVec(sizeof(*un),MEMF_PUBLIC|MEMF_CLEAR);
    if(un)
    {
	dn = AllocVec(sizeof(*dn),MEMF_PUBLIC|MEMF_CLEAR);
	if (dn)
	{
	    dn->type   = ST_ROOT;
	    dn->name   = iofs->io_Union.io_OpenDevice.io_DosName;
	    dn->parent = NULL;

	    NEWLIST(&dn->files);
	    DateStamp(&dn->datestamp);

	    un->fn = (struct filenode *)dn;
	    iofs->IOFS.io_Unit=(struct Unit *)un;
            iofs->IOFS.io_Device=&pipefsbase->device;

	    /* I have one more opener. */
            pipefsbase->device.dd_Library.lib_OpenCnt++;

	    pipefsbase->device.dd_Library.lib_Flags&=~LIBF_DELEXP;
    	    iofs->IOFS.io_Error=0;

    	    return;
        }

	FreeVec(un);
    }

    iofs->io_DosError=ERROR_NO_FREE_STORE;

    iofs->IOFS.io_Error=IOERR_OPENFAIL;

    AROS_LIBFUNC_EXIT
}

AROS_LH1(BPTR, close,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct pipefsbase *, pipefsbase, 2, pipefs_handler)
{
    AROS_LIBFUNC_INIT
    struct usernode *un;
    struct dirnode  *dn;

    un = (struct usernode *)iofs->IOFS.io_Unit;
    dn = (struct dirnode *)un->fn;

    if(!IsListEmpty(&dn->files))
    {
	iofs->io_DosError=ERROR_OBJECT_IN_USE;
	return 0;
    }

    /* Let any following attemps to use the device crash hard. */
    iofs->IOFS.io_Device=(struct Device *)-1;

    FreeVec(dn);
    FreeVec(un);

    iofs->io_DosError=0;

    /* I have one fewer opener. */
    if(!--pipefsbase->device.dd_Library.lib_OpenCnt)
    {
	/* Delayed expunge pending? */
	if(pipefsbase->device.dd_Library.lib_Flags&LIBF_DELEXP)
	    /* Then expunge the device */
	    return expunge();
    }
    return 0;
    AROS_LIBFUNC_EXIT
}

AROS_LH0(BPTR, expunge, struct pipefsbase *, pipefsbase, 3, pipefs_handler)
{
    AROS_LIBFUNC_INIT

    BPTR ret;
    /*
	This function is single-threaded by exec by calling Forbid.
	Never break the Forbid() or strange things might happen.
    */

    /* Test for openers. */
    if(pipefsbase->device.dd_Library.lib_OpenCnt)
    {
	/* Set the delayed expunge flag and return. */
	pipefsbase->device.dd_Library.lib_Flags|=LIBF_DELEXP;
	return 0;
    }

    SendRequest(pipefsbase, NULL);

    /* Free all resources */
    CloseLibrary((struct Library *)pipefsbase->dosbase);

    /* Get rid of the device. Remove it from the list. */
    Remove(&pipefsbase->device.dd_Library.lib_Node);

    /* Get returncode here - FreeMem() will destroy the field. */
    ret=pipefsbase->seglist;

    /* Free the memory. */
    FreeMem((char *)pipefsbase-pipefsbase->device.dd_Library.lib_NegSize,
	    pipefsbase->device.dd_Library.lib_NegSize+pipefsbase->device.dd_Library.lib_PosSize);

    return ret;
    AROS_LIBFUNC_EXIT
}

AROS_LH0I(int, null, struct pipefsbase *, pipefsbase, 4, pipefs_handler)
{
    AROS_LIBFUNC_INIT
    return 0;
    AROS_LIBFUNC_EXIT
}

AROS_LH1(void, beginio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct pipefsbase *, pipefsbase, 5, pipefs_handler)
{
    AROS_LIBFUNC_INIT
    LONG error=0;
    BOOL enqueued = FALSE;

    kprintf("COMMAND %d\n", iofs->IOFS.io_Command);
    switch(iofs->IOFS.io_Command)
    {
	case FSA_OPEN:
	case FSA_OPEN_FILE:
        case FSA_EXAMINE:
	case FSA_EXAMINE_NEXT:
	case FSA_READ:
	case FSA_WRITE:
	case FSA_CLOSE:
	case FSA_CREATE_DIR:
        case FSA_DELETE_OBJECT:
	    error = SendRequest(pipefsbase, iofs);
	    enqueued = !error;
	    break;

	case FSA_SEEK:
	    error = ERROR_SEEK_ERROR;
	    break;
	case FSA_IS_FILESYSTEM:
	    iofs->io_Union.io_IS_FILESYSTEM.io_IsFilesystem = TRUE;
	    break;
	case FSA_SET_FILE_SIZE:
        case FSA_EXAMINE_ALL:
        case FSA_CREATE_HARDLINK:
        case FSA_CREATE_SOFTLINK:
        case FSA_RENAME:
            error = ERROR_NOT_IMPLEMENTED;
            break;

	default:
	    error = ERROR_ACTION_NOT_KNOWN;
	    break;
    }

    /* Set error code */
    iofs->io_DosError=error;

    /* If the quick bit is not set and the request hasn't been redirected
       send the message to the port
    */
    if(!(iofs->IOFS.io_Flags&IOF_QUICK) && !enqueued)
	ReplyMsg(&iofs->IOFS.io_Message);

    AROS_LIBFUNC_EXIT
}

AROS_LH1(LONG, abortio,
 AROS_LHA(struct IOFileSys *, iofs, A1),
	   struct pipefsbase *, pipefsbase, 6, pipefs_handler)
{
    AROS_LIBFUNC_INIT
    /* Everything already done. */
    return 0;
    AROS_LIBFUNC_EXIT
}

static ULONG SendRequest(struct pipefsbase *pipefsbase, struct IOFileSys *iofs)
{
    struct pipefsmessage *msg = AllocVec(sizeof(*msg), MEMF_PUBLIC);

    if (msg)
    {
        msg->msg.mn_Node.ln_Type = NT_MESSAGE;
	msg->msg.mn_Node.ln_Name = "PIPEFSMSG";
        msg->msg.mn_Length       = sizeof(struct pipefsmessage);
	msg->iofs                = iofs;

	if (iofs)
	{
	    iofs->IOFS.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	    iofs->IOFS.io_Flags &= ~IOF_QUICK;
     	}

	PutMsg(&pipefsbase->proc->pr_MsgPort, (struct Message *)msg);

	return 0;
    }

    return ERROR_NO_FREE_STORE;
}

/* The helper process */

#define SendBack(msg, err)                   \
{                                            \
    msg->iofs->io_DosError = err;            \
    ReplyMsg(&(msg)->iofs->IOFS.io_Message); \
    FreeVec(msg);                            \
}

static STRPTR SkipColon(STRPTR str)
{
    STRPTR oldstr = str;

    while(str[0])
        if (str++[0] == ':') return str;

    return oldstr;
}

static STRPTR StrDup(struct pipefsbase *pipefsbase, STRPTR str)
{
    size_t len = strlen(str)+1;
    STRPTR ret = AllocVec(len, MEMF_ANY);

    if (ret)
        CopyMem(str, ret, len);

    return ret;
}

/*
  Return the len of the first part in the path.

  EXAMPLE
    LenFirstPart("yyy/xxx") would return 3
    LenFirstPart("xxxx") would return 4
*/

static size_t LenFirstPart(STRPTR path)
{
    size_t len = 0;

    for (; path[0] && path[0] != '/'; path++, len++);

    return len;
}

static struct filenode *FindFile(struct dirnode **dn_ptr, STRPTR path)
{
    #define dn (*dn_ptr)

    size_t len;
    STRPTR nextpart;
    struct filenode *fn;

    while (path[0] == '/' && dn)
    {
        dn = dn->parent;
	path++;
    }

    if (!dn) return NULL;

    if (!path[0]) return (struct filenode *)dn;

    if (dn->type <= 0)
    {
        kprintf("User wants %S to be a directory, but it's a file.\n", dn->name);
	dn = NULL;
	return NULL;
    }

    len      = LenFirstPart(path);
    nextpart = &path[len];
    fn       = GetHead(&dn->files);

    kprintf("Searching for %.*S.\n", len, path);

    while (fn)
    {
	kprintf("Comparing %S with %.*S.\n", fn->name, len, path);
	if
	(
	    strlen(fn->name) == len               &&
	    strncasecmp(fn->name, path, len) == 0
	)
	{
	    break;
	}
        fn = GetSucc((struct Node *)fn);
    }

    if (fn)
    {
	if (nextpart[0] == '/') nextpart++;

	dn = (struct dirnode *)fn;
	fn = FindFile(&dn, nextpart);
    }

    return fn;

    #undef dn
}

static struct filenode *GetFile(struct pipefsbase *pipefsbase, STRPTR filename, struct dirnode *dn, ULONG mode, ULONG *err)
{
    struct filenode *fn;

    filename = SkipColon(filename);

    kprintf("User wants to open file %S.\n", filename);
    kprintf("Current directory is %S\n", dn->name);

    fn = FindFile(&dn, filename);
    if (!fn)
    {
	kprintf("The file couldn't be found.\n");

	if (dn && mode&FMF_CREATE)
	{
	    kprintf("But the user wants it to be created.\n");

	    fn = AllocVec(sizeof(*fn), MEMF_PUBLIC|MEMF_CLEAR);
	    if (fn)
	    {
		fn->name = StrDup(pipefsbase, FilePart(filename));

		if (fn->name)
		{
		    fn->type = ST_PIPEFILE;

		    DateStamp(&fn->datestamp);
		    NEWLIST(&fn->pendingwrites);
		    NEWLIST(&fn->pendingreads);

		    fn->parent = dn;

		    AddTail(&dn->files, (struct Node *)fn);
		    kprintf("New file created and added to the list\n");

		    return fn;
		}

		FreeVec(fn);
	    }

	    kprintf("AllocVec Failed. No more memory available\n");
	    *err = ERROR_NO_FREE_STORE;
	    return NULL;
	}
	else
	{
            *err = ERROR_OBJECT_NOT_FOUND;
	    return NULL;
	}
    }

    if ((BYTE)fn->type > 0 && mode&(FMF_WRITE|FMF_READ))
    {
	kprintf("The file is a directory, cannot be open for reading/writing\n");
	*err = ERROR_OBJECT_WRONG_TYPE;
	return NULL;
    }

    return fn;
}

#undef SysBase
#ifndef kprintf
     struct ExecBase *SysBase;
#else
#    define SysBase _SysBase
#endif


AROS_UFH3(LONG, pipefsproc,
    AROS_UFHA(char *,argstr,A0),
    AROS_UFHA(ULONG,argsize,D0),
    AROS_UFHA(struct ExecBase *,_SysBase,A6))
{
    AROS_USERFUNC_INIT

    struct Process       *me;
    struct pipefsbase    *pipefsbase;
    struct pipefsmessage *msg;
    struct usernode      *un;
    struct filenode      *fn;
    BOOL cont = TRUE;

    SysBase = _SysBase;

    me         = (struct Process *)FindTask(0);
    pipefsbase = me->pr_Task.tc_UserData;


    do
    {
    	WaitPort(&(me->pr_MsgPort));

	while
	(
	    (msg =(struct pipefsmessage *)GetMsg(&(me->pr_MsgPort))) &&
	    (cont = (msg->iofs != 0))
	)
	{
	    kprintf("Message received.\n");

	    un = (struct usernode *)msg->iofs->IOFS.io_Unit;
	    fn = un->fn;

	    switch (msg->iofs->IOFS.io_Command)
	    {
		case FSA_OPEN:
		    msg->iofs->io_Union.io_OPEN.io_FileMode &= ~(FMF_WRITE|FMF_READ);
		    /* Fall through */
		case FSA_OPEN_FILE:
		{
		    struct usernode *un;
		    BOOL             stillwaiting;
		    ULONG            err;

		    kprintf("Command is OPEN\n");

		    /*
		       I would have liked to put this AFTER GetFile(),
		       but then it would have been really difficult to
		       undo what's done in GetFile() if the following AllocVec()
		       failed...
		    */
		    un = AllocVec(sizeof(*un), MEMF_PUBLIC);
		    if (!un)
		    {
			SendBack(msg, ERROR_NO_FREE_STORE);
			continue;
		    }

		    un->mode = msg->iofs->io_Union.io_OPEN.io_FileMode;

		    fn = GetFile(pipefsbase, msg->iofs->io_Union.io_OPEN.io_Filename, (struct dirnode *)fn, un->mode, &err);
		    if (!fn)
		    {
			FreeVec(un);
			SendBack(msg, err);
			continue;
		    }

		    kprintf("File requested found.\n");
		    kprintf("The requested file is %s.\n",
		            fn->type <= 0  ?
			    "a pipe":
			    "a directory");

		    msg->iofs->IOFS.io_Unit = (struct Unit *)un;
		    fn->numusers++;
		    un->fn = fn;

		    if (fn->type > 0)
		    {
		        SendBack(msg, 0);
			continue;
		    }

                    stillwaiting = !fn->numwriters || !fn->numreaders;

		    if (un->mode == FMF_MODE_OLDFILE) un->mode &= ~FMF_WRITE;
		    if (un->mode == FMF_MODE_NEWFILE) un->mode &= ~FMF_READ;

		    if (un->mode & FMF_READ)
		    {
			kprintf("User wants to read. ");
			fn->numreaders++;
		    }
		    if (un->mode & FMF_WRITE)
		    {
			kprintf("User wants to write. ");
		    	fn->numwriters++;
		    }

		    kprintf("There are %d readers and %d writers at the moment\n", fn->numreaders, fn->numwriters);

		    if (!fn->numwriters || !fn->numreaders)
		    {
			if (un->mode&(FMF_WRITE|FMF_READ))
			{
			    /*
			       If we're lacking of writers or readers
			       then add this message to a waiting list.
			    */
			    kprintf("There are no %s at the moment, so this %s must wait\n",
				     fn->numwriters?"readers":"writers",
				     fn->numwriters?"writer":"reader");

			    AddTail(&fn->waitinglist, (struct Node *)msg);
       			}
			else
			    SendBack(msg, 0);
                    }
		    else
		    {
			if (stillwaiting)
		        {
		            /*
		              Else wake up all the ones that were still waiting
		            */
			    struct pipefsmessage *msg;

			    kprintf("Finally there are enough readers and writers! "
			            "Wake up all of them\n");

			    while ((msg = (struct pipefsmessage *)RemHead(&fn->waitinglist)))
			        SendBack(msg, 0);
           		}
			SendBack(msg, 0);
		    }

		    continue;
		}
		case FSA_CLOSE:
		    kprintf("Command is FSA_CLOSE\n");

		    if (un->mode & FMF_READ)
		    {
			kprintf("User was a reader. ");
			fn->numreaders--;
			kprintf("There are %d readers at the moment\n", fn->numreaders);
   		    }
		    if (un->mode & FMF_WRITE)
		    {
			kprintf("User was a writer. ");
			fn->numwriters--;
			kprintf("There are %d writers at the moment\n", fn->numwriters);
   		    }

		    if (un->mode&FMF_WRITE && !fn->numwriters)
		    {
			struct pipefsmessage *msg;

			kprintf("There are no writers anymore. %s\n",
			        IsListEmpty(&fn->pendingreads) ?
				"There are no pending reads"   :
			        "Reply to all the waiting readers");
			while ((msg = (struct pipefsmessage *)RemHead(&fn->pendingreads)))
			{
			    msg->iofs->io_Union.io_READ_WRITE.io_Length =
			    msg->iofs->io_Union.io_READ_WRITE.io_Length - msg->curlen;
			    SendBack(msg, 0);
			}
		    }
		    if (un->mode&FMF_READ && !fn->numreaders)
		    {
			struct pipefsmessage *msg;

			kprintf("There are no readers anymore. %s\n",
			        IsListEmpty(&fn->pendingwrites) ?
				"There are no pending writes"   :
			        "Reply to all the waiting writers");

			while ((msg = (struct pipefsmessage *)RemHead(&fn->pendingwrites)))
			    SendBack(msg, ERROR_BROKEN_PIPE);
		    }

		    un->fn->numusers--;
		    FreeVec(un);
		    SendBack(msg, 0);

		    continue;
		case FSA_EXAMINE:
   		{
		    struct ExAllData  *ead        = msg->iofs->io_Union.io_EXAMINE.io_ead;
                    const ULONG        type       = msg->iofs->io_Union.io_EXAMINE.io_Mode;
                    const ULONG        size       = msg->iofs->io_Union.io_EXAMINE.io_Size;
                    STRPTR             next, end;

                    static const ULONG sizes[]=
		    {
			0,
			offsetof(struct ExAllData,ed_Type),
    			offsetof(struct ExAllData,ed_Size),
    			offsetof(struct ExAllData,ed_Prot),
    			offsetof(struct ExAllData,ed_Days),
    			offsetof(struct ExAllData,ed_Comment),
    			offsetof(struct ExAllData,ed_OwnerUID),
    			sizeof(struct ExAllData)
    		    };

		    kprintf("Command is EXAMINE\n");
		    kprintf("Examining file %S\n", fn->name);

		    if (type > ED_OWNER)
    		    {
			kprintf("The user requested an invalid type\n");
			SendBack(msg, ERROR_BAD_NUMBER);
			continue;
		    }

    		    next = (STRPTR)ead + sizes[type];
    		    end  = (STRPTR)ead + size;

		    if(next>end)  /* > is correct. Not >= */
		    {
			SendBack(msg, ERROR_BUFFER_OVERFLOW);
			continue;
		    }

		    if (fn->type > 0)
		    {
			msg->iofs->io_DirPos = (LONG)GetHead(&((struct dirnode *)fn)->files);
                    }
		    else
		    {
			msg->iofs->io_DirPos = (LONG)fn;
    		    }

		    switch(type)
		    {
        		case ED_OWNER:
	    		ead->ed_OwnerUID = 0;
	    		ead->ed_OwnerGID = 0;

			/* Fall through */
        		case ED_COMMENT:
	    		    ead->ed_Comment = NULL;

			/* Fall through */
        		case ED_DATE:
	    		    ead->ed_Days  = fn->datestamp.ds_Days;
			    ead->ed_Mins  = fn->datestamp.ds_Minute;
	    		    ead->ed_Ticks = fn->datestamp.ds_Tick;

			/* Fall through */
        		case ED_PROTECTION:
	    		    ead->ed_Prot = 0;

			/* Fall through */
        		case ED_SIZE:
	    		    ead->ed_Size = 0;

			/* Fall through */
        		case ED_TYPE:
	    		    ead->ed_Type = fn->type;

			/* Fall through */
			case ED_NAME:
	  		{
			    STRPTR name = fn->name;
			    ead->ed_Name = next;

			    for (;;)
			    {
	    			if (next >= end)
	    			{
				    SendBack(msg, ERROR_BUFFER_OVERFLOW);
				    continue;
	    			}

	    			if (!(*next++ = *name++))
	    			{
				    break;
	    			}
			    }
			}
    		    }

		    ead->ed_Next = (struct ExAllData *)(((IPTR)next + AROS_PTRALIGN - 1) & ~(AROS_PTRALIGN - 1));

		    SendBack(msg, 0);
		    continue;
		}
		case FSA_EXAMINE_NEXT:
		{
		    struct FileInfoBlock *fib = msg->iofs->io_Union.io_EXAMINE_NEXT.io_fib;
                    struct filenode      *fn  = (struct filenode *)fib->fib_DiskKey;

		    kprintf("Command is EXAMINE_NEXT\n");

		    if (!fn)
    		    {
			kprintf("There are no more entries in this directory\n");
			SendBack(msg, ERROR_NO_MORE_ENTRIES);
			continue;
		    }

    		    kprintf("Current directory is %S. Current file is %S\n", fn->parent->name, fn->name);

		    fib->fib_OwnerUID       = 0;
		    fib->fib_OwnerGID       = 0;
		    fib->fib_Date.ds_Days   = fn->datestamp.ds_Days;
		    fib->fib_Date.ds_Minute = fn->datestamp.ds_Minute;
		    fib->fib_Date.ds_Tick   = fn->datestamp.ds_Tick;
		    fib->fib_Protection	    = 0;
		    fib->fib_Size	    = 0;
		    fib->fib_DirEntryType   = fn->type;

		    strncpy(fib->fib_FileName, fn->name, MAXFILENAMELENGTH - 1);
		    fib->fib_Comment[0] = '\0';

		    fib->fib_DiskKey = (LONG)GetSucc(fn);
    		    SendBack(msg, 0);

		    continue;
		}
		case FSA_CREATE_DIR:
		{
		    STRPTR filename        = SkipColon(msg->iofs->io_Union.io_CREATE_DIR.io_Filename);
		    struct dirnode *parent = (struct dirnode *)fn;
		    struct dirnode *dn;

		    kprintf("Command is FSA_CREATE_DIR\n");
		    kprintf("Current directory is %S\n", parent->name);
		    kprintf("User wants to create directory %S\n", filename);

		    dn = (struct dirnode *)FindFile(&parent, filename);
		    if (dn)
		    {
			kprintf("The object %S already exists\n", filename);
			SendBack(msg, ERROR_OBJECT_EXISTS);
			continue;
		    }
		    else
		    if (!parent)
		    {
			kprintf("The path is not valid.\n");
			SendBack(msg, ERROR_OBJECT_NOT_FOUND);
			continue;
		    }

		    if
		    (
		    	!(un       = AllocVec(sizeof(*un), MEMF_PUBLIC)) ||
                        !(dn       = AllocVec(sizeof(*dn), MEMF_PUBLIC)) ||
			!(dn->name = StrDup(pipefsbase, filename))
		    )
		    {
		        SendBack(msg, ERROR_NO_FREE_STORE);
			continue;
   		    }

		    kprintf("Ok, there's room for this directory.\n");
		    AddTail(&parent->files, (struct Node *)dn);
		    dn->parent   = parent;
		    dn->numusers = 0;
		    dn->type     = ST_USERDIR;
		    NEWLIST(&dn->files);
		    DateStamp(&dn->datestamp);

		    un->fn   = (struct dilenode *)dn;
		    un->mode = 0;
		    msg->iofs->IOFS.io_Unit = (struct Unit *)un;

		    SendBack(msg, 0);
		    continue;
		}
		case FSA_DELETE_OBJECT:
		{
		    STRPTR filename    = SkipColon(msg->iofs->io_Union.io_DELETE_OBJECT.io_Filename);
		    struct dirnode *dn = (struct dirnode *)fn;

		    kprintf("Command is FSA_DELETE_OBJECT\n");
		    kprintf("Current directory is %S\n", fn->name);
		    kprintf("User wants to delete the object %S\n", filename);

		    fn = FindFile(&dn, filename);
		    if (!fn)
		    {
		        kprintf("The object doesn't exist\n");
			SendBack(msg, ERROR_OBJECT_NOT_FOUND);
			continue;
		    }
		    if (fn->type == ST_ROOT)
		    {
		        kprintf("The object is the root directory. Cannot be deleted\n");
			SendBack(msg, ERROR_OBJECT_WRONG_TYPE);
			continue;
		    }
		    if (fn->numusers)
		    {
		        kprintf("The object is in use, cannot be deleted\n");
			SendBack(msg, ERROR_OBJECT_IN_USE);
			continue;
		    }
		    if (fn->type > 0 && !IsListEmpty(&((struct dirnode *)fn)->files))
		    {
		        kprintf("The object is a directory, but is not empty, thus cannot be deleted\n");
			SendBack(msg, ERROR_DIRECTORY_NOT_EMPTY);
			continue;
		    }

		    kprintf("Removing the object from it's parent directory\n");

		    Remove((struct Node *)fn);
		    FreeVec(fn->name);
		    FreeVec(fn);

		    SendBack(msg, 0);
		    continue;
      		}
		case FSA_WRITE:
		    kprintf("Command is FSA_WRITE. ");
		    if (!un->mode & FMF_WRITE)
		    {
		        kprintf("User doesn't have permission to write.\n");
			SendBack(msg, ERROR_BAD_STREAM_NAME);
		        continue;
		    }
		    if (!fn->numreaders)
		    {
			kprintf("There are no more readers: PIPE BROKEN.\n");
			SendBack(msg, ERROR_BROKEN_PIPE);
		        continue;
		    }
		    kprintf("Enqueing the message\n");
		    msg->curlen = msg->iofs->io_Union.io_READ_WRITE.io_Length;
		    AddTail(&fn->pendingwrites, (struct Node *)msg);
		    break;
		case FSA_READ:
		    kprintf("Command is FSA_READ. ");
		    if (!un->mode & FMF_READ)
		    {
		        kprintf("User doesn't have permission to read.\n");
			SendBack(msg, ERROR_BAD_STREAM_NAME);
		        continue;
		    }
		    if (!fn->numwriters)
		    {
			kprintf("There's no data to read: send EOF\n");
			msg->iofs->io_Union.io_READ_WRITE.io_Length = 0;
			SendBack(msg, 0);
		        continue;
		    }
		    kprintf("Enqueing the message\n");
		    msg->curlen = msg->iofs->io_Union.io_READ_WRITE.io_Length;
		    AddTail(&fn->pendingreads, (struct Node *)msg);
		    break;
	    }

	    while (!IsListEmpty(&fn->pendingwrites) && !IsListEmpty(&fn->pendingreads))
	    {
		struct pipefsmessage *rmsg = (struct pipefsmessage *)GetHead(&fn->pendingreads);
		struct pipefsmessage *wmsg = (struct pipefsmessage *)GetHead(&fn->pendingwrites);

		ULONG len = (rmsg->curlen > wmsg->curlen) ?
		             wmsg->curlen : rmsg->curlen;

	    	kprintf("Writer len = %d - Reader len = %d. Copying %d bytes\n",
		         wmsg->curlen, rmsg->curlen, len);

  		CopyMem
		(
        	    wmsg->iofs->io_Union.io_READ_WRITE.io_Buffer +
		    wmsg->iofs->io_Union.io_READ_WRITE.io_Length -
		    wmsg->curlen,

		    rmsg->iofs->io_Union.io_READ_WRITE.io_Buffer +
      		    rmsg->iofs->io_Union.io_READ_WRITE.io_Length -
		    rmsg->curlen,

		    len
		);

		wmsg->curlen -= len;
		rmsg->curlen -= len;

	        kprintf("Writer curlen is now %d - Reader curlen is now %d\n",
		         wmsg->curlen, rmsg->curlen);

		if (!wmsg->curlen)
		{
		    kprintf("Writer: finished its job. Removing it from the list.\n");
		    Remove((struct Node *)wmsg);
		    SendBack(wmsg, 0);
		}

		if (!rmsg->curlen)
		{
		    kprintf("Reader: finished its job. Removing it from the list.\n");
		    Remove((struct Node *)rmsg);
		    SendBack(rmsg, 0);
		}

	    }

	}
	kprintf("Coming back to wait for a new message\n");

    } while (cont);

    return 0;

    AROS_USERFUNC_EXIT
}

static const char end=0;
