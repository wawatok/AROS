/* 
    Copyright � 1999, David Le Corfec.
    Copyright � 2002, The AROS Development Team.
    All rights reserved.

    $Id$
*/

#ifndef __ZUNE_PREFS_H__
#define __ZUNE_PREFS_H__

#ifndef LIBRARIES_COMMODITIES_H
#include <libraries/commodities.h>
#endif

#ifndef LIBRARIES_MUI_H
#include "mui.h"
#endif

#ifndef _MUI_FRAME_H
#include "frame.h"
#endif

typedef enum CycleMenuPosition {
    CYCLE_MENU_POSITION_BELOW,
    CYCLE_MENU_POSITION_ONACTIVE,
} CycleMenuPosition;

typedef enum GroupTitlePosition {
    GROUP_TITLE_POSITION_ABOVE,
    GROUP_TITLE_POSITION_CENTERED,
} GroupTitlePosition;

typedef enum GroupTitleColor {
    GROUP_TITLE_COLOR_STANDARD,
    GROUP_TITLE_COLOR_HILITE,
    GROUP_TITLE_COLOR_3D,
} GroupTitleColor;

typedef enum WindowPosition {
    WINDOW_POSITION_FORGET_ON_EXIT,
    WINDOW_POSITION_SAVE_ON_EXIT,
} WindowPosition;

typedef enum DNDLook {
    DND_LOOK_ALWAYS_SOLID,
    DND_LOOK_GHOSTED_ON_BOX,
    DND_LOOK_GHOSTED_OUTSIDE_BOX,
    DND_LOOK_ALWAYS_GHOSTED,
} DNDLook;

typedef enum ScrollbarType {
    SCROLLBAR_TYPE_STANDARD,
    SCROLLBAR_TYPE_NEWLOOK,
    SCROLLBAR_TYPE_CUSTOM,
} ScrollbarType;

typedef enum ScrollbarArrangement {
    SCROLLBAR_ARRANGEMENT_TOP,
    SCROLLBAR_ARRANGEMENT_MIDDLE,
    SCROLLBAR_ARRANGEMENT_BOTTOM,
} ScrollbarArrangement;

typedef enum BalancingLook {
    BALANCING_SHOW_FRAMES,
    BALANCING_SHOW_OBJECTS,
} BalancingLook;

typedef enum ActiveObjectLook {
    ACTIVE_OBJECT_LOOK_FRAME,
    ACTIVE_OBJECT_LOOK_CORNER,
} ActiveObjectLook;

typedef struct _ZuneKeySpec {
    STRPTR readable_hotkey;
    LONG ix_well;
    IX ix;
} ZuneKeySpec;

typedef enum RegisterLook {
    REGISTER_LOOK_TRADITIONAL,
    REGISTER_LOOK_GADTOOLS,
} RegisterLook;

typedef enum ListviewRefresh {
    LISTVIEW_REFRESH_LINEAR,
    LISTVIEW_REFRESH_MIXED,
} ListviewRefresh;

typedef enum ListviewMulti {
    LISTVIEW_MULTI_SHIFTED,
    LISTVIEW_MULTI_ALWAYS,
} ListviewMulti;

/*
 * User Prefs for interface drawing
 */

struct ZunePrefsNew
{
    CONST_STRPTR fonts[-MUIV_Font_NegCount];
    CONST_STRPTR imagespecs[MUII_Count];
    struct MUI_FrameSpec_intern frames[MUIV_Frame_Count];

    /* Groups */
    GroupTitlePosition group_title_position;
    GroupTitleColor    group_title_color;
    WORD     group_hspacing;
    WORD     group_vspacing;

    /* Windows */
    WindowPosition  window_position;
    WORD     window_inner_left;
    WORD     window_inner_right;
    WORD     window_inner_top;
    WORD     window_inner_bottom;

    /* MUI Keys */
    ZuneKeySpec muikeys[MUIKEY_COUNT];

    /* Zune registers */
    RegisterLook register_look; /* yet unused, remove this comment when handled */
    BOOL         register_truncate_titles; /* ok, waiting for prefs editor */

    /* Buttons */
    WORD     radiobutton_hspacing;
    WORD     radiobutton_vspacing;

    /* Cycles */
    CycleMenuPosition cycle_menu_position; /* yet unused, remove this comment when handled */
    WORD     cycle_menu_min_entries; /* yet unused, remove this comment when handled */
    WORD     cycle_menu_speed; /* yet unused, remove this comment when handled */
    BOOL     cycle_menu_recessed_entries; /* yet unused, remove this comment when handled */

    /* Strings */
/* old zune used struct MUI_PenSpec, new will probably differ -dlc */
    struct MUI_PenSpec string_bg_active; /* yet unused, remove this comment when handled */
    struct MUI_PenSpec string_text_active; /* yet unused, remove this comment when handled */
    struct MUI_PenSpec string_bg_inactive; /* yet unused, remove this comment when handled */
    struct MUI_PenSpec string_text_inactive; /* yet unused, remove this comment when handled */

    /* Lists */
    ListviewMulti   list_multi; /* yet unused, remove this comment when handled */
    ListviewRefresh list_refresh; /* yet unused, remove this comment when handled */
    UWORD           list_linespacing; /* yet unused, remove this comment when handled */
    BOOL            list_smoothed; /* yet unused, remove this comment when handled */
    UWORD           list_smoothval; /* yet unused, remove this comment when handled */

    /* Navigation */
    BOOL                 dragndrop_left_button; /* yet unused, remove this comment when handled */
    ZuneKeySpec          dragndrop_left_modifier; /* yet unused, remove this comment when handled */
    BOOL                 dragndrop_middle_button; /* yet unused, remove this comment when handled */
    ZuneKeySpec          dragndrop_middle_modifier; /* yet unused, remove this comment when handled */
    LONG                 dragndrop_autostart; /* yet unused, remove this comment when handled */
    DNDLook              dragndrop_look; /* yet unused, remove this comment when handled */
    BalancingLook        balancing_look; /* yet unused, remove this comment when handled */
    ActiveObjectLook     active_object_look; /* yet unused, remove this comment when handled */
    struct MUI_PenSpec   active_object_color; /* yet unused, remove this comment when handled */

    /* Scrollbars */
    ScrollbarType        scrollbar_type;
    ScrollbarArrangement scrollbar_arrangement;
};

#endif


