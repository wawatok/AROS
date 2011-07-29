/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Boot AROS
    Lang: english
*/

#include <string.h>
#include <stdlib.h>

#include <aros/debug.h>
#include <exec/alerts.h>
#include <aros/asmcall.h>
#include <aros/bootloader.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/types.h>
#include <libraries/configvars.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <libraries/partition.h>
#include <utility/tagitem.h>
#include <devices/bootblock.h>
#include <devices/timer.h>
#include <dos/dosextens.h>
#include <resources/filesysres.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/partition.h>
#include <proto/bootloader.h>
#include <proto/dos.h>

#include LC_LIBDEFS_FILE

#include "dosboot_intern.h"

#ifdef __mc68000

/* These two functions are implemented in arch/m68k/all/dosboot/bootcode.c */

extern VOID_FUNC CallBootBlockCode(APTR bootcode, struct IOStdReq *io, struct ExpansionBase *ExpansionBase);
extern void dosboot_BootPoint(struct BootNode *bn);

#else

#define CallBootBlockCode(bootcode, io, ExpansionBase) NULL
#define dosboot_BootPoint(bn)

#endif

static BOOL GetBootNodeDeviceUnit(struct BootNode *bn, BPTR *device, IPTR *unit, ULONG *bootblocks)
{
    struct DeviceNode *dn;
    struct FileSysStartupMsg *fssm;
    struct DosEnvec *de;

    if (bn == NULL)
        return FALSE;

    dn = bn->bn_DeviceNode;
    if (dn == NULL)
        return FALSE;

    fssm = BADDR(dn->dn_Startup);
    if (fssm == NULL)
        return FALSE;

    *unit = fssm->fssm_Unit;
    *device = fssm->fssm_Device;

    de = BADDR(fssm->fssm_Environ);
    /* Following check from Guru Book */
    if (de == NULL || (de->de_TableSize & 0xffffff00) != 0 || de->de_TableSize < DE_BOOTBLOCKS)
    	return FALSE;
    *bootblocks = de->de_BootBlocks * de->de_SizeBlock * sizeof(ULONG);
    if (*bootblocks == 0)
    	return FALSE;
    return TRUE;
}
 
static BOOL BootBlockChecksum(UBYTE *bootblock, ULONG bootblock_size)
{
       ULONG crc = 0, crc2 = 0;
       UWORD i;
       for (i = 0; i < bootblock_size; i += 4) {
           ULONG v = (bootblock[i] << 24) | (bootblock[i + 1] << 16) |
(bootblock[i + 2] << 8) | bootblock[i + 3];
           if (i == 4) {
               crc2 = v;
               v = 0;
           }
           if (crc + v < crc)
               crc++;
           crc += v;
       }
       crc ^= 0xffffffff;
       D(bug("[Strap] bootblock checksum %s (%08x %08x)\n", crc == crc2 ? "ok" : "error", crc, crc2));
       return crc == crc2;
}

static inline void SetBootNodeDosType(struct BootNode *bn, ULONG dostype)
{
    struct DeviceNode *dn;
    struct FileSysStartupMsg *fssm;
    struct DosEnvec *de;

    dn = bn->bn_DeviceNode;
    if (dn == NULL)
        return;

    fssm = BADDR(dn->dn_Startup);
    if (fssm == NULL)
        return;

    de = BADDR(fssm->fssm_Environ);
    if (de == NULL || de->de_TableSize < DE_DOSTYPE)
        return;

    de->de_DosType = dostype;
}

static void dosboot_BootBlock(struct BootNode *bn, struct ExpansionBase *ExpansionBase)
{
    ULONG bootblock_size;
    struct MsgPort *msgport;
    struct IOStdReq *io;
    BPTR device;
    IPTR unit;
    VOID_FUNC init = NULL;
    UBYTE *buffer;

    if (!GetBootNodeDeviceUnit(bn, &device, &unit, &bootblock_size))
        return;

    D(bug("%s: Probing for boot block on %b.%d\n", __func__, device, unit));
    /* memf_chip not required but more compatible with old bootblocks */
    buffer = AllocMem(bootblock_size, MEMF_CHIP);
    if (buffer != NULL)
    {
       D(bug("[Strap] bootblock address %p\n", buffer));
       if ((msgport = CreateMsgPort()))
       {
           if ((io = CreateIORequest(msgport, sizeof(struct IOStdReq))))
           {
               if (!OpenDevice(AROS_BSTR_ADDR(device), unit, (struct IORequest*)io, 0))
               {
                   /* Read the device's boot block */
                   io->io_Length = bootblock_size;
                   io->io_Data = buffer;
                   io->io_Offset = 0;
                   io->io_Command = CMD_READ;
                   D(bug("[Strap] %b.%d bootblock read (%d bytes)\n", device, unit, bootblock_size));
                   DoIO((struct IORequest*)io);

                   if (io->io_Error == 0)
                   {
                       D(bug("[Strap] %b.%d bootblock read to %p ok\n", device, unit, buffer));
                       if (BootBlockChecksum(buffer, bootblock_size))
                       {
                           SetBootNodeDosType(bn, AROS_LONG2BE(*(LONG *)buffer));
                           init = CallBootBlockCode(buffer + 12, io, ExpansionBase);
                       }
                       else
                       {
                           D(bug("[Strap] Not a valid bootblock\n"));
                       }
                   } else {
                       D(bug("[Strap] io_Error %d\n", io->io_Error));
                   }
                   io->io_Command = TD_MOTOR;
                   io->io_Length = 0;
                   DoIO((struct IORequest*)io);
                   CloseDevice((struct IORequest *)io);
               }
               DeleteIORequest((struct IORequest*)io);
           }
           DeleteMsgPort(msgport);
       }
       FreeMem(buffer, bootblock_size);
   }

   if (init != NULL)
   {
       CloseLibrary((APTR)ExpansionBase);
       D(bug("calling bootblock loaded code at %p\n", init));
       init();
   }
}

/* Attempt to boot via dos.library directly
 */
static void dosboot_BootDos(struct BootNode *bn)
{
    APTR DOSBase;
    struct Resident *DOSResident;

    /* First, see if previous attempt was able to initialize
     * dos.library, but dos.library failed to find a
     * bootable paritition.
     */
    DOSBase = OpenLibrary("dos.library", 0);
    if (DOSBase) {
        /* Since DOSBase is now (mostly) set up,
         * just call CliInit(), which will try
         * to boot off of this node.
         */
        if (CliInit(NULL) == RETURN_OK)
            RemTask(NULL);
    } else {
        /*
         * Initialize dos.library manually.
         */
        DOSResident = FindResident( "dos.library" );

        if( DOSResident == NULL )
        {
            Alert( AT_DeadEnd | AG_OpenLib | AN_BootStrap | AO_DOSLib );
        }

        /* InitResident() of dos.library will not return 
         * on success.
         */
        InitResident( DOSResident, BNULL );
    }
}


/* Attempt to boot, first from the BootNode boot blocks,
 * then via the DOS handlers
 */
LONG dosboot_BootStrap(LIBBASETYPEPTR LIBBASE)
{
    struct BootNode *bn;
    struct ExpansionBase *ExpansionBase;
    int i, nodes;

    ExpansionBase = (APTR)OpenLibrary("expansion.library", 0);
    if (ExpansionBase == NULL)
        Alert( AT_DeadEnd | AG_OpenLib | AN_BootStrap | AO_ExpansionLib );

    /*
     * Try to boot from any device in the boot list,
     * highest priority first.
     */
    ListLength(&ExpansionBase->MountList, nodes);
    for (i = 0; i < nodes; i++) {
        bn = (struct BootNode *)GetHead(&ExpansionBase->MountList);

        if (bn->bn_Node.ln_Type != NT_BOOTNODE ||
            bn->bn_Node.ln_Pri <= -128 ||
            bn->bn_DeviceNode == NULL) {
            D(bug("%s: Ignoring %p, not a bootable node\n", __func__, bn));
            REMOVE(bn);
            ADDTAIL(&ExpansionBase->MountList, bn);
            continue;
        }

        /* For each attempt, this node is at the head
         * of the MountList, so that DOS will try to
         * use it as SYS: if the strap works
         */

        /* First try as a BootPoint node */
        dosboot_BootPoint(bn);

        /* Then as a BootBlock */
        dosboot_BootBlock(bn, ExpansionBase);

        /* And finally with DOS */
        dosboot_BootDos(bn);

        /* Didn't work. Next! */
        D(bug("%s: DeviceNode %b (%d) was not bootable\n", __func__,
                    ((struct DeviceNode *)bn->bn_DeviceNode)->dn_Name,
                    bn->bn_Node.ln_Pri));

        REMOVE(bn);
        ADDTAIL(&ExpansionBase->MountList, bn);
    }

    CloseLibrary((APTR)ExpansionBase);

    D(bug("%s: No BootBlock, BootPoint, or BootDos nodes found\n",__func__));

    /* At this point we now know that we were unable to
     * strap any bootable devices.
     */

    return ERROR_NO_DISK;
}


