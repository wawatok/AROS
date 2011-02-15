/*
    Copyright © 2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Code to dynamically load ELF executables under AOS.
    Lang: english
*/

/* Compile for AROS:
 *
 * $ bin/linux-x86_64/tools/m68k-amiga-aros-gcc  -Os -I /path/to/src/AROS \
 *      SetPatchAROS.c -o SetPatchAROS.elf
 * $ bin/linux-x86_64/tools/elf2hunk SetPatchAROS.elf SetPatchAROS
 * 
 * Copy SetPatchAROS to your AOS installation, and add the following
 * to your startup script:
 *
 * Run SetPatchAROS
 *
 * Send a ^C to the SetPatchAROS to unload it.
 */

#define MINSTACK	8192

#include <aros/asmcall.h>
#include <dos/stdio.h>
#include <proto/utility.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <rom/dos/internalloadseg_elf.c>

static AROS_UFH4(LONG, ReadFunc,
	AROS_UFHA(BPTR, file,   D1),
	AROS_UFHA(APTR, buffer, D2),
	AROS_UFHA(LONG, length, D3),
        AROS_UFHA(struct DosLibrary *, DOSBase, A6)
)
{
    AROS_USERFUNC_INIT

    return FRead(file, buffer, 1, length);

    AROS_USERFUNC_EXIT
}

static AROS_UFH3(APTR, AllocFunc,
	AROS_UFHA(ULONG, length, D0),
	AROS_UFHA(ULONG, flags,  D1),
        AROS_UFHA(struct ExecBase *, SysBase, A6)
)
{
    AROS_USERFUNC_INIT

    return AllocMem(length, flags);

    AROS_USERFUNC_EXIT
}

static AROS_UFH3(void, FreeFunc,
	AROS_UFHA(APTR, buffer, A1),
	AROS_UFHA(ULONG, length, D0),
        AROS_UFHA(struct ExecBase *, SysBase, A6)
)
{
    AROS_USERFUNC_INIT

    FreeMem(buffer, length);

    AROS_USERFUNC_EXIT
}

static APTR oldLoadSeg;

static AROS_UFH2(BPTR, myLoadSeg,
	AROS_UFHA(CONST_STRPTR, name, D1),
	AROS_UFHA(struct DosLibrary *, DOSBase, A6))
{
   AROS_USERFUNC_INIT

   SIPTR FunctionArray[3];
   BPTR file, segs;
   ULONG magic;

   FunctionArray[0] = (SIPTR)ReadFunc;
   FunctionArray[1] = (SIPTR)AllocFunc;
   FunctionArray[2] = (SIPTR)FreeFunc;

   file = Open (name, MODE_OLDFILE);
   if (file) {
       LONG len;

       SetVBuf(file, NULL, BUF_FULL, 4096);
       SetIoErr(0);

       /* Is it the ELF magic? */
       len = FRead(file, &magic, 1, sizeof(magic));
       if (len == sizeof(magic) && magic == 0x7f454c46) { /* ELF magic */
           segs = InternalLoadSeg_ELF(file, BNULL, FunctionArray, NULL, DOSBase);
           if (segs) {
               if ((LONG)segs > 0) {
                   Close(file);
               } else {
                   segs = (BPTR)-((LONG)segs);
               }
               SetIoErr(0);
               return segs;
           }
       }
       Close(file);
   }

   return AROS_UFC2(BPTR, oldLoadSeg,
              AROS_UFCA(CONST_STRPTR, name, D1),
              AROS_UFCA(struct DosLibrary *, DOSBase, A6));

   AROS_USERFUNC_EXIT
}

static APTR oldCreateProc;
static AROS_UFH5(APTR, myCreateProc,
	AROS_UFHA(STRPTR *, name, D1),
	AROS_UFHA(LONG, priority, D2),
	AROS_UFHA(BPTR, seglist,  D3),
	AROS_UFHA(LONG, stacksize,D4),
	AROS_UFHA(struct DosLibrary *, DOSBase, A6))
{
    AROS_USERFUNC_INIT

    if (stacksize < MINSTACK)
    	stacksize = MINSTACK;
    else
    	stacksize += 1024;

    return AROS_UFC5(APTR, oldCreateProc,
	AROS_UFCA(STRPTR *, name, D1),
	AROS_UFCA(LONG, priority, D2),
	AROS_UFCA(BPTR, seglist,  D3),
	AROS_UFCA(LONG, stacksize,D4),
	AROS_UFCA(struct DosLibrary *, DOSBase, A6));

   AROS_USERFUNC_EXIT
}

static APTR oldCreateNewProc;
static AROS_UFH2(APTR, myCreateNewProc,
	AROS_UFHA(struct TagItem *, tag, D1),
	AROS_UFHA(struct DosLibrary *, DOSBase, A6))
{
    AROS_USERFUNC_INIT

    struct TagItem *tstate, *tmp, *found, fake[2];
    int tags;

    tstate = tag;
    found = NULL;
    tags = 0;

    while ((tmp = NextTagItem(&tstate)) != NULL) {
    	if (tmp->ti_Tag == NP_StackSize)
    		found = tmp;
    	tags++;
    }

    Printf("Tags:  %ld\n", (LONG)tags);
    if (found == NULL) {
    	Printf("Fake: (0x%lx %ld)\n", (LONG)NP_StackSize, MINSTACK);
	fake[0].ti_Tag = NP_StackSize;
	fake[0].ti_Data = MINSTACK;
	fake[1].ti_Tag = TAG_MORE;
	fake[1].ti_Data = (IPTR)tag;
	tag = &fake[0];
    } else {
    	Printf("Found: 0x%lx (%ld)\n", (LONG)found, found->ti_Data);
    	if (found->ti_Data <= MINSTACK)
    	    found->ti_Data = MINSTACK;
    	else
    	    found->ti_Data += 1024;
    }
    return AROS_UFC2(APTR, oldCreateNewProc,
    	    AROS_UFCA(struct TagItem *, tag, D1),
    	    AROS_UFCA(struct DosLibrary *, DOSBase, A6));

    AROS_USERFUNC_EXIT
}


/* Really stupid way to deal with this,
 * but what we're going to do is make our
 * patches, and wait for a ^C.
 *
 * When we get a ^C, unpatch ourselves from
 * Dos/LoadSeg(), and exit.
 *
 * To use:
 *
 * > Run SetPatchAROS
 */
int main(int argc, char **argv)
{
   struct DOSBase *DOSBase;

   DOSBase = (APTR)OpenLibrary("dos.library", 0);
   if (DOSBase != NULL) {
       oldCreateProc    = SetFunction(DOSBase, -23 * LIB_VECTSIZE, myCreateProc);
       oldCreateNewProc = SetFunction(DOSBase, -83 * LIB_VECTSIZE, myCreateNewProc);
       oldLoadSeg       = SetFunction(DOSBase, -25 * LIB_VECTSIZE, myLoadSeg);
       PutStr("AROS Support active. Press ^C to unload.\n");
       Wait(SIGBREAKF_CTRL_C);
       SetFunction(DOSBase, -25 * LIB_VECTSIZE, oldLoadSeg);
       SetFunction(DOSBase, -83 * LIB_VECTSIZE, oldCreateNewProc);
       SetFunction(DOSBase, -23 * LIB_VECTSIZE, oldCreateProc);
       PutStr("AROS Support unloaded.\n");
       CloseLibrary(DOSBase);
   }

   return RETURN_OK;
}
