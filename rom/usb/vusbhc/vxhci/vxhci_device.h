/*
    Copyright � 2014, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: English
*/
#ifndef VXHCI_DEVICE_H
#define VXHCI_DEVICE_H

#include <aros/debug.h>
#include <aros/macros.h>

#include <proto/exec.h>
#include <proto/stdc.h>
#include <proto/arossupport.h>

#include <devices/usb.h>
#include <devices/usb_hub.h>
#include <devices/newstyle.h>
#include <devices/usbhardware.h>

/* Maximum number of units */
#define VXHCI_NUMCONTROLLERS 1

/* Maximum number of ports per protocol (USB2.0/USB3.0) */
#define VXHCI_NUMPORTS20 1
#define VXHCI_NUMPORTS30 1

#define RC_OK         0
#define RC_DONTREPLY -1

#define MYBUG_LEVEL 200
#define mybug(l, x) D(if ((l>=MYBUG_LEVEL)||(l==-1)) { do { { bug x; } } while (0); } )
#define mybug_unit(l, x) D(if ((l>=MYBUG_LEVEL)||(l==-1)) { do { { bug("%s %s: ", unit->name, __FUNCTION__); bug x; } } while (0); } )

WORD cmdUsbReset(struct IOUsbHWReq *ioreq);
WORD cmdNSDeviceQuery(struct IOStdReq *ioreq);
WORD cmdQueryDevice(struct IOUsbHWReq *ioreq);
WORD cmdControlXFer(struct IOUsbHWReq *ioreq);
WORD cmdControlXFerRootHub(struct IOUsbHWReq *ioreq);
WORD cmdIntXFer(struct IOUsbHWReq *ioreq);
WORD cmdIntXFerRootHub(struct IOUsbHWReq *ioreq);
WORD cmdGetString(struct IOUsbHWReq *ioreq, char *cstring);

struct VXHCIPort {
    struct Node                  node;
    char                         name[256];
    ULONG                        number;
    ULONG                        state;
};

struct VXHCIUnit {
    struct Node                  node;
    char                         name[256];
    ULONG                        number;
    ULONG                        state;

    struct VXHCIRootHub {
        struct List              port_list;
        ULONG                    port_count;

        UWORD                    addr;

        struct UsbStdDevDesc     devdesc;

        struct RHConfig {
            struct UsbStdCfgDesc cfgdesc;
            struct UsbStdIfDesc  ifdesc;
            struct UsbStdEPDesc  epdesc;
        }                        config;

        union {
            struct UsbHubDesc    usb20;
            struct UsbSSHubDesc  usb30;
        }                        hubdesc;

    }                            roothub;

};

struct VXHCIBase {

    struct Device                device;
    struct List                  unit_list;
    ULONG                        unit_count;

};

#endif /* VXHCI_DEVICE_H */
