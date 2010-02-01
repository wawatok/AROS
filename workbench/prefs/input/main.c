/*
    Copyright  2003-2010, The AROS Development Team. All rights reserved.
    $Id$
*/

// #define MUIMASTER_YES_INLINE_STDARG

#include <proto/alib.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include <libraries/mui.h>
#include <prefs/input.h>

#include <zune/systemprefswindow.h>

#include "locale.h"
#include "args.h"
#include "ipeditor.h"
#include "prefs.h"

#include <aros/debug.h>

#define VERSION "$VER: Input 0.2 ("ADATE") AROS Dev Team"

/*********************************************************************************************/

static struct MsgPort     *InputMP;

/*********************************************************************************************/

static BOOL OpenInputDev(void)
{
    if ((InputMP = CreateMsgPort()))
    {
        if ((InputIO = (struct timerequest *) CreateIORequest(InputMP, sizeof(struct IOStdReq))))
        {
            OpenDevice("input.device", 0, (struct IORequest *)InputIO, 0);
            return TRUE;
        }
    }
    return FALSE;
}

/*********************************************************************************************/

static void CloseInputDev(void)
{
    if (InputIO)
    {
        CloseDevice((struct IORequest *)InputIO);
        DeleteIORequest((struct IORequest *)InputIO);
    }

    if (InputMP)
    {
        DeleteMsgPort(InputMP);
    }
}


/*********************************************************************************************/


int main(int argc, char **argv)
{
    Object *application,  *window;

    if (!OpenInputDev())
        return 0;

    Locale_Initialize();

    if (ReadArguments(argc, argv))
    {
        if (ARG(USE) || ARG(SAVE))
        {
            Prefs_HandleArgs((STRPTR)ARG(FROM), ARG(USE), ARG(SAVE));
        }
        else
        {
            Prefs_Default();
            Prefs_Backup();

            NewList(&keymap_list);

            mempool = (IPTR) CreatePool(MEMF_PUBLIC | MEMF_CLEAR, 2048, 2048);
            if (mempool != 0)
            {
                Prefs_ScanDirectory("DEVS:Keymaps/#?_~(#?.info)", &keymap_list, sizeof(struct KeymapEntry));

                application = ApplicationObject,
                    MUIA_Application_Title,  __(MSG_NAME),
                    MUIA_Application_Version, (IPTR) VERSION,
                    MUIA_Application_Description,  __(MSG_DESCRIPTION),
                    MUIA_Application_Base, (IPTR) "INPUTPREF",
                    SubWindow, (IPTR) (window = SystemPrefsWindowObject,
                        MUIA_Window_ID, MAKE_ID('I','W','I','N'),
                        WindowContents, (IPTR) IPEditorObject,
                        TAG_DONE),
                    End),
                End;

                if (application != NULL)
                {
                    SET(window, MUIA_Window_Open, TRUE);
                    DoMethod(application, MUIM_Application_Execute);
                    SET(window, MUIA_Window_Open, FALSE);

                    MUI_DisposeObject(application);
                }

                DeletePool((APTR)mempool);
            }
        }
        FreeArguments();
    }

    Prefs_kbd_cleanup();

    CloseInputDev();

    Locale_Deinitialize();

    return 0;
}
