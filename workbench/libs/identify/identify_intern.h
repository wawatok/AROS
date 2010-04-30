#ifndef IDENDIFY_INTERN_H
#define IDENDIFY_INTERN_H

#include <exec/types.h>
#include <exec/libraries.h>

#define STRBUFSIZE (30)

struct HardwareBuffer
{
    TEXT buf_OsVer[STRBUFSIZE];
    TEXT buf_OsVerLoc[STRBUFSIZE];
    TEXT buf_ExecVer[STRBUFSIZE];
    TEXT buf_ExecVerLoc[STRBUFSIZE];
    TEXT buf_WbVer[STRBUFSIZE];
    TEXT buf_WbVerLoc[STRBUFSIZE];
    TEXT buf_RomSize[STRBUFSIZE];
    TEXT buf_RomSizeLoc[STRBUFSIZE];
    TEXT buf_ChipRAM[STRBUFSIZE];
    TEXT buf_ChipRAMLoc[STRBUFSIZE];
    TEXT buf_FastRAM[STRBUFSIZE];
    TEXT buf_FastRAMLoc[STRBUFSIZE];
    TEXT buf_RAM[STRBUFSIZE];
    TEXT buf_RAMLoc[STRBUFSIZE];
    TEXT buf_SetPatchVer[STRBUFSIZE];
    TEXT buf_SetPatchVerLoc[STRBUFSIZE];
    TEXT buf_VMChipRAM[STRBUFSIZE];
    TEXT buf_VMChipRAMLoc[STRBUFSIZE];
    TEXT buf_VMFastRAM[STRBUFSIZE];
    TEXT buf_VMFastRAMLoc[STRBUFSIZE];
    TEXT buf_VMRAM[STRBUFSIZE];
    TEXT buf_VMRAMLoc[STRBUFSIZE];
    TEXT buf_PlainChipRAM[STRBUFSIZE];
    TEXT buf_PlainChipRAMLoc[STRBUFSIZE];
    TEXT buf_PlainFastRAM[STRBUFSIZE];
    TEXT buf_PlainFastRAMLoc[STRBUFSIZE];
    TEXT buf_PlainRAM[STRBUFSIZE];
    TEXT buf_PlainRAMLoc[STRBUFSIZE];
    TEXT buf_VBR[STRBUFSIZE]; // not cached
    TEXT buf_LastAlert[STRBUFSIZE]; // not cached
    TEXT buf_VBlankFreq[STRBUFSIZE];
    TEXT buf_PowerFreq[STRBUFSIZE];
    TEXT buf_EClock[STRBUFSIZE];
    TEXT buf_SlowRAM[STRBUFSIZE];
    TEXT buf_SlowRAMLoc[STRBUFSIZE];
    TEXT buf_PPCClock[STRBUFSIZE];
    TEXT buf_CPURev[STRBUFSIZE];
    TEXT buf_CPUClock[STRBUFSIZE];
    TEXT buf_FPUClock[STRBUFSIZE];
    TEXT buf_RAMAccess[STRBUFSIZE];
    TEXT buf_RAMWidth[STRBUFSIZE];
    TEXT buf_RAMBandwidth[STRBUFSIZE];
    TEXT buf_DeniseRev[STRBUFSIZE];
    TEXT buf_BoingBag[STRBUFSIZE];
    TEXT buf_XLVersion[STRBUFSIZE];
    TEXT buf_HostOS[STRBUFSIZE];
    TEXT buf_HostVers[STRBUFSIZE];
    TEXT buf_HostMachine[STRBUFSIZE];
    TEXT buf_HostCPU[STRBUFSIZE];
    TEXT buf_HostSpeed[STRBUFSIZE];
};

struct IdentifyBaseIntern
{
    struct Library base;
    BOOL dirtyflag;
    struct HardwareBuffer hwb;
};

#endif
