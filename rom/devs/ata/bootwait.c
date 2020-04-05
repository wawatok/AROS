/*
    Copyright � 1995-2013, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <aros/debug.h>
#include <proto/exec.h>

#include <aros/asmcall.h>
#include <exec/resident.h>
#include <libraries/expansionbase.h>

#include LC_LIBDEFS_FILE

#include "ata.h"

#if defined(__AROSPLATFORM_SMP__)
#include <aros/types/spinlock_s.h>
#include <proto/execlock.h>
#include <resources/execlock.h>
#endif

extern const char ata_LibName[];
extern const char ata_LibID[];
extern const int ata_End;

AROS_UFP3(static APTR, ata_Wait,
	  AROS_UFPA(void *, dummy, D0),
	  AROS_UFPA(BPTR, segList, A0),
	  AROS_UFPA(struct ExecBase *, SysBase, A6));

const struct Resident ata_BootWait =
{
    RTC_MATCHWORD,
    (struct Resident *)&ata_BootWait,
    (void *)&ata_End,
    RTF_COLDSTART,
    VERSION_NUMBER,
    NT_TASK,
    -49, /* dosboot.resource is -50 */
    "ATA boot wait",
    &ata_LibID[6],
    &ata_Wait,
};

/* Delay just like Dos/Delay(), ticks are
 * in 1/50th of a second.
 */
static void bootDelay(ULONG timeout)
{
    struct timerequest  timerio;
    struct MsgPort     timermp;

    memset(&timermp, 0, sizeof(timermp));

    timermp.mp_Node.ln_Type = NT_MSGPORT;
    timermp.mp_Flags        = PA_SIGNAL;
    timermp.mp_SigBit       = SIGB_SINGLE;
    timermp.mp_SigTask      = FindTask(NULL);
    NEWLIST(&timermp.mp_MsgList);

    timerio.tr_node.io_Message.mn_Node.ln_Type  = NT_REPLYMSG;
    timerio.tr_node.io_Message.mn_ReplyPort     = &timermp;
    timerio.tr_node.io_Message.mn_Length        = sizeof(timermp);

    if (OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)&timerio, 0) != 0) {
        D(bug("dosboot: Can't open timer.device unit 0\n"));
        return;
    }

    timerio.tr_node.io_Command  = TR_ADDREQUEST;
    timerio.tr_time.tv_secs     = timeout / TICKS_PER_SECOND;
    timerio.tr_time.tv_micro    = 1000000UL / TICKS_PER_SECOND * (timeout % TICKS_PER_SECOND);

    SetSignal(0, SIGF_SINGLE);

    DoIO(&timerio.tr_node);

    CloseDevice((struct IORequest *)&timerio);
}

/*
 * The purpose of this delay is to wait until device detection is done
 * before boot sequence enters DOS bootstrap. Without this we reach the
 * bootstrap earlier than devices are detected (and BootNodes inserted).
 * As a result, we end up in unbootable system.
 * Actually, i dislike this solution a bit. I think something else has
 * to be implemented. However i do not know what. Even if we rewrite
 * adding BootNodes, bootmenu still has to wait until all nodes are added.
 * Making device detection synchronous is IMHO not a good option, it will
 * increase booting time of our OS.
 */

AROS_UFH3(static APTR, ata_Wait,
	  AROS_UFPA(void *, dummy, D0),
	  AROS_UFPA(BPTR, segList, A0),
	  AROS_UFPA(struct ExecBase *, SysBase, A6))
{
    AROS_USERFUNC_INIT

    struct ataBase *ATABase;
#if defined(__AROSPLATFORM_SMP__)
    void *ExecLockBase = OpenResource("execlock.resource");
#endif

#if defined(__AROSPLATFORM_SMP__)
    if (ExecLockBase)
        ObtainSystemLock(&SysBase->DeviceList, SPINLOCK_MODE_READ, LOCKF_FORBID);
    else
        Forbid();
#else
    Forbid();
#endif

    //bootDelay( 5* 50 );

    /* We do not want to deal with IORequest and units, so just FindName() */
    ATABase = (struct ataBase *)FindName(&SysBase->DeviceList, ata_LibName);

#if defined(__AROSPLATFORM_SMP__)
    if (ExecLockBase)
        ReleaseSystemLock(&SysBase->DeviceList, LOCKF_FORBID);
    else
        Permit();
#else
    Permit();
#endif

    if (ATABase)
    {
        D(bug("[ATA  ] Waiting for device detection to complete...\n"));
        ObtainSemaphore(&ATABase->DetectionSem);
        ReleaseSemaphore(&ATABase->DetectionSem);
    }
    
    return NULL;
    
    AROS_USERFUNC_EXIT
}
