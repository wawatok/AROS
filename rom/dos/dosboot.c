/*
	Copyright (C) 1997 AROS - The Amiga Research OS
	$Id$

	Desc: Start up the ol' Dos boot process.
	Lang: english 
*/

#define AROS_ALMOST_COMPATIBLE

# define  DEBUG  1
# include <aros/debug.h>
#include <aros/macros.h>

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/execbase.h>
#include <exec/alerts.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <libraries/expansionbase.h>
#include <aros/asmcall.h>
#include <string.h>
#include <dos/filehandler.h>
#include <dos/filesystem.h>
#include <devices/trackdisk.h>

#include <proto/exec.h>
#include <proto/dos.h>

extern void boot();

BOOL init_hidds(struct ExecBase *, struct DosLibrary *);

AROS_UFH3(void, intBoot,
    AROS_UFHA(APTR, argString, A0),
    AROS_UFHA(ULONG, argSize, D0),
    AROS_UFHA(struct ExecBase *,SysBase, A6)
)
{
	struct ExpansionBase *ExpansionBase = NULL;
	struct Library *DOSBase = NULL;
	BPTR lock;
	struct BootNode *bn;
	STRPTR bootname, s1;
	ULONG len;


	/* HACK: Wait for LDDemon to signal us that it has finished
	   initializing itself
	*/
	Wait(SIGBREAKF_CTRL_F);

	DOSBase = OpenLibrary("dos.library", 0);
	if( DOSBase == NULL)
		Alert(AT_DeadEnd| AG_OpenLib | AN_DOSLib | AO_DOSLib);

	ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0);
	if( ExpansionBase == NULL )
	{
		D(bug("Urk, no expansion.library, something's wrong!\n"));
		Alert(AT_DeadEnd | AG_OpenLib | AN_DOSLib | AO_ExpansionLib);
	}

	/* We have to do all the locking in this because we don't
	   have a Process in DOSBoot yet, and we need a process
	   to create locks etc.
	*/
	bn = (struct BootNode *)ExpansionBase->MountList.lh_Head;
	s1 = ((struct DosList *)bn->bn_DeviceNode)->dol_DevName;
	while (*s1++)
		;

	len = s1 - ((struct DosList *)bn->bn_DeviceNode)->dol_DevName;
	bootname = AllocMem(len + 2, MEMF_ANY);
	if (bootname == NULL)
		Alert( AT_DeadEnd | AG_NoMemory | AO_DOSLib | AN_StartMem );
	CopyMem(((struct DosList *)bn->bn_DeviceNode)->dol_DevName,
		bootname, len);
	bootname[len-1] = ':';
	bootname[len] = '\0';

#if (AROS_FLAVOUR & AROS_FLAVOUR_STANDALONE)
	/* only do this in native/standalone mode
		because in hosted AROS emul_handler is already
		initialized at this point
	*/
	if (((struct DosList *)bn->bn_DeviceNode)->dol_Device==0)
	{
		struct DeviceNode *dn = (struct DeviceNode *)bn->bn_DeviceNode;
		struct FileSysStartupMsg *fssm= BADDR(dn->dn_Startup);
		struct Resident *filesys;
		struct MsgPort *mp=CreateMsgPort();
		struct IOFileSys *iofs;
		filesys = FindResident((STRPTR)((ULONG)BADDR(dn->dn_Handler)+1));
		if (filesys) {
			InitResident(filesys,0);
			if (mp)
			{
				iofs = (struct IOFileSys *)CreateIORequest(mp,sizeof(struct IOFileSys));
				if (iofs)
				{
					struct IOExtTD *iotd=(struct IOExtTD *)CreateIORequest(mp,sizeof(struct IOExtTD));
					ULONG *buf = (ULONG *)AllocMem(512,MEMF_ANY);
					if ((iotd) && (buf))
					{
						if (!OpenDevice(fssm->fssm_Device+1,fssm->fssm_Unit, (struct IORequest *)&iotd->iotd_Req,0))
						{
							kprintf("Insert bootable disk\n");
							for (;;)
							{
								iofs->io_Union.io_OpenDevice.io_DeviceName=fssm->fssm_Device+1;
								iofs->io_Union.io_OpenDevice.io_Unit = fssm->fssm_Unit;
								iofs->io_Union.io_OpenDevice.io_Environ = BADDR(fssm->fssm_Environ);
								if (OpenDevice((STRPTR)((ULONG)BADDR(dn->dn_Handler)+1),fssm->fssm_Unit,&iofs->IOFS,0))
								{
									kprintf("\nFS returned error %ld on this disk!\n"
												"Try another disk.\n"
												"AROS will be rebooted in 10 seconds ...\n",iofs->io_DosError);
									Delay(500);
									ColdReboot();
								}
								else
									break;
							}

							dn->dn_Unit = iofs->IOFS.io_Unit;
							dn->dn_Device = iofs->IOFS.io_Device;
							CloseDevice((struct IORequest *)&iotd->iotd_Req);
						}
					}
					if (iotd) DeleteIORequest((struct IORequest *)&iotd->iotd_Req);
					if (buf) FreeMem(buf,512);
					DeleteIORequest(&iofs->IOFS);
				}
				DeleteMsgPort(mp);
			}
		}
		else
			D(bug("filesys not found!!!\n"));
	}
#endif
	CloseLibrary((struct Library *)ExpansionBase);

	D(bug("Locking primary boot device %s\n", bootname));

	lock = Lock(bootname, SHARED_LOCK);

	if(lock != NULL)
	{
	    AssignLock("SYS", lock);
	}
	else
	{
	    Alert(AT_DeadEnd | AG_BadParm | AN_DOSLib);
	}

	FreeMem(bootname, len + 2);
	lock = Lock("SYS:", SHARED_LOCK);
	if ( lock )
		CurrentDir(lock);
	else
		Alert( AT_DeadEnd | AG_BadParm | AN_DOSLib );

	lock = Lock("SYS:C", SHARED_LOCK);
	if( lock )
		AssignLock("C", lock);

	lock = Lock("SYS:S", SHARED_LOCK);
	if( lock )
		AssignLock("S", lock);

	lock = Lock("SYS:Libs", SHARED_LOCK);
	if( lock )
		AssignLock("LIBS", lock);

	lock = Lock("SYS:Devs", SHARED_LOCK);
	if( lock )
		AssignLock("DEVS", lock);

	/* Late binding ENVARC: assign, only if used */
	AssignLate("ENVARC", "SYS:Prefs/env-archive");
	
	/* Initialize HIDDs */
	init_hidds(SysBase, (struct DosLibrary *)DOSBase);

	/* We now call the system dependant boot - should never return. */
	AROS_UFC3(void, boot, 
		AROS_UFCA(STRPTR, argString, A0),
		AROS_UFCA(ULONG, argSize, D0),
		AROS_UFCA(struct ExecBase *, SysBase, A6));
}

void DOSBoot(struct ExecBase *SysBase, struct DosLibrary *DOSBase)
{
	struct ExpansionBase *ExpansionBase;
	struct BootNode *bn;

	struct TagItem bootprocess[] = 
	{
		{ NP_Entry,	(IPTR)intBoot },
		{ NP_Name,	(IPTR)"Boot Process" },
		{ NP_Input,	(IPTR)NULL },
		{ NP_Output,	(IPTR)NULL },
		{ NP_CurrentDir,(IPTR)NULL },
		{ NP_Cli,	1 },
		{ TAG_END, }
	};

	ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library",0);
	if( ExpansionBase == NULL )
	{
		D(bug("Urk, no expansion.library, something's wrong!\n"));
		Alert(AT_DeadEnd | AG_OpenLib | AN_DOSLib | AO_ExpansionLib);
	}

	D(bug("Examining MountList: \n"));

	ForeachNode(&ExpansionBase->MountList, bn)
	{
		D(bug("Node: %p, DevNode: %p, Name = %s\n", bn,
			bn->bn_DeviceNode,
			((struct DosList *)bn->bn_DeviceNode)->dol_DevName
			? ((struct DosList *)bn->bn_DeviceNode)->dol_DevName
			: "(null)"
		));
		AddDosEntry((struct DosList *)bn->bn_DeviceNode);
	}

	if(CreateNewProc(bootprocess) == NULL)
	{
		D(bug("CreateNewProc() failed with %ld\n",
			((struct Process *)FindTask(NULL))->pr_Result2));
		Alert( AT_DeadEnd | AN_DOSLib | AG_ProcCreate );
	}
	CloseLibrary((struct Library*)ExpansionBase);
}


