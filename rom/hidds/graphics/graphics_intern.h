/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef GRAPHICS_HIDD_INTERN_H
#define GRAPHICS_HIDD_INTERN_H

/* Include files */

#include <aros/debug.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <oop/oop.h>
#include <hidd/graphics.h>
#include <dos/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/monitor.h>


#define USE_FAST_PUTPIXEL		1
#define OPTIMIZE_DRAWPIXEL_FOR_COPY	1
#define USE_FAST_DRAWPIXEL		1
#define USE_FAST_GETPIXEL		1
#define COPYBOX_CHECK_FOR_ALIKE_PIXFMT	1

#define HBM(x) ((struct HIDDBitMapData *)x)

#define GOT_PF_ATTR(code)	GOT_ATTR(code, aoHidd_PixFmt, pixfmt)
#define FOUND_PF_ATTR(code)	FOUND_ATTR(code, aoHidd_PixFmt, pixfmt);

#define GOT_SYNC_ATTR(code)	GOT_ATTR(code, aoHidd_Sync, sync)
#define FOUND_SYNC_ATTR(code)	FOUND_ATTR(code, aoHidd_Sync, sync);

#define GOT_BM_ATTR(code)	GOT_ATTR(code, aoHidd_BitMap, bitmap)
#define FOUND_BM_ATTR(code)	FOUND_ATTR(code, aoHidd_BitMap, bitmap);

#define SWAPBYTES_WORD(x) ((((x) >> 8) & 0x00FF) | (((x) & 0x00FF) << 8))

struct colormap_data
{
    HIDDT_ColorLUT clut;
};

struct pixfmt_data
{
     HIDDT_PixelFormat pf; /* Public portion in the beginning    */

     struct MinNode node;  /* Node for linking into the database */
     ULONG refcount;	   /* Reference count			 */
};

/* Use this macro in order to transform node pointer to pixfmt pointer */
#define PIXFMT_OBJ(n) ((HIDDT_PixelFormat *)((char *)(n) - offsetof(struct pixfmt_data, node)))

struct planarbm_data
{
    UBYTE   **planes;
    ULONG   planebuf_size;
    ULONG   bytesperrow;
    ULONG   rows;
    UBYTE   depth;
    BOOL    planes_alloced;
};

struct chunkybm_data
{
    OOP_Object *gfxhidd;       /* Cached driver object */
    UBYTE *buffer;
    ULONG bytesperrow;
    ULONG bytesperpixel;
    BOOL own_buffer;
};

struct sync_data
{
    struct MonitorSpec *mspc;	/* Associated MonitorSpec */

    ULONG pixelclock;		/* pixel time in Hz */

    ULONG hdisp;		/* Data missing from MonitorSpec */
    ULONG htotal;
    ULONG vdisp;

    ULONG flags;		/* Flags */

    UBYTE description[32];
    
    ULONG hmin;			/* Minimum and maximum allowed bitmap size */
    ULONG hmax;
    ULONG vmin;
    ULONG vmax;

    OOP_Object	 *gfxhidd;	 /* Graphics driver that owns this sync		*/
    OOP_MethodID  SetMode_mID;	 /* SetMode method ID, for do_monitor()		*/
    ULONG InternalFlags;	 /* Internal flags, see below			*/
};

/* Sync internal flags */
#define SYNC_FREE_MONITORSPEC    0x0001 /* Allocated own MonitorSpec 				*/
#define SYNC_FREE_SPECIALMONITOR 0x0002 /* Allocated own SpecialMonitor				*/
#define SYNC_VARIABLE		 0x0004 /* Signal timings can be changed			*/

struct mode_bm {
    UBYTE *bm;
    UWORD bpr;
};
struct mode_db {
    /* Array of all available gfxmode PixFmts that are part of 
       gfxmodes
     */
    struct SignalSemaphore sema;
    OOP_Object **pixfmts;
    /* Number of pixfmts in the above array */
    ULONG num_pixfmts;
    
    /* All the sync times that are part of any gfxmode */
    OOP_Object **syncs;
    /* Number of syncs in the above array */
    ULONG num_syncs;
    
    /* A bitmap of size (num_pixfmts * num_syncs), that tells if the
       mode is displayable or not. If a particular (x, y) coordinate
       of the bitmap is 1, it means that the pixfmt and sync objects
       you get by indexing pixfmts[x] and syncs[y] are a valid mode.
       If not, the mode is considered invalid
    */
    
    struct mode_bm orig_mode_bm;	/* Original as supplied by subclass */
    struct mode_bm checked_mode_bm;	/* After applying monitor refresh rate checks etc. */
    
};

struct HIDDGraphicsData
{
	/* Gfx mode "database" */
	struct mode_db mdb;

	/* Framebuffer control stuff */
	OOP_Object *framebuffer;
	OOP_Object *shownbm;
	
	/* gc used for stuff like rendering cursor */
	OOP_Object *gc;
	
	/* The mode currently used (obsolete ?)
	HIDDT_ModeID curmode; */
};

/* Private gfxhidd methods */
OOP_Object *GFX__Hidd_Gfx__RegisterPixFmt(OOP_Class *cl, OOP_Object *o, struct TagItem *pixFmtTags);
VOID GFX__Hidd_Gfx__ReleasePixFmt(OOP_Class *cl, OOP_Object *pf);

/* Private bitmap methods */
BOOL BM__Hidd_BitMap__SetBitMapTags(OOP_Class *cl, OOP_Object *o, struct TagItem *bitMapTags);

struct HIDDBitMapData
{
    struct _hidd_bitmap_protected prot;
    
    ULONG width;         /* width of the bitmap in pixel  */
    ULONG height;        /* height of the bitmap in pixel */
    BOOL  displayable;   /* bitmap displayable?           */
    BOOL  pf_registered;
    ULONG flags;         /* see hidd/graphic.h 'flags for */
    ULONG bytesPerRow;   /* bytes per row                 */
    /* WARNING: structure could be extented in the future                */
    
    OOP_Object *friend;	/* Friend bitmap */
    
    OOP_Object *gfxhidd;
    
    OOP_Object *colmap;
    
    HIDDT_ModeID modeid;

    /* Optimize these two method calls */
#if USE_FAST_PUTPIXEL    
    OOP_MethodFunc putpixel;
    OOP_Class *putpixel_Class;
#endif
#if USE_FAST_GETPIXEL    
    OOP_MethodFunc getpixel;
    OOP_Class *getpixel_Class;
#endif
#if USE_FAST_DRAWPIXEL    
    OOP_MethodFunc drawpixel;
    OOP_Class *drawpixel_Class;
#endif
};

/* Private bitmap attrs */

enum
{
    aoHidd_BitMap_Dummy = num_Hidd_BitMap_Attrs,    
    num_Total_BitMap_Attrs
};

#if 0
struct HIDDGCData
{
#if 0
    APTR bitMap;     /* bitmap to which this gc is connected             */
#endif
    APTR  userData;  /* pointer to own data                              */
    ULONG fg;        /* foreground color                                 */
    ULONG bg;        /* background color                                 */
    ULONG drMode;    /* drawmode                                         */
    /* WARNING: type of font could be change */
    APTR  font;      /* current fonts                                    */
    ULONG colMask;   /* ColorMask prevents some color bits from changing */
    UWORD linePat;   /* LinePattern                                      */
    APTR  planeMask; /* Pointer to a shape bitMap                        */
    ULONG colExp;
    
    /* WARNING: structure could be extented in the future                */
};
#endif    

#define NUM_ATTRBASES   9
#define NUM_METHODBASES 4

struct class_static_data
{
    struct GfxBase	 *GfxBase;
    struct SignalSemaphore sema;

    OOP_AttrBase	 attrBases[NUM_ATTRBASES];
    OOP_MethodID	 methodBases[NUM_METHODBASES];

    OOP_Class            *gfxhiddclass; /* graphics hidd class    */
    OOP_Class            *bitmapclass;  /* bitmap class           */
    OOP_Class            *gcclass;      /* graphics context class */
    OOP_Class		 *colormapclass; /* colormap class	  */
    
    OOP_Class		 *pixfmtclass;	/* describing bitmap pixel formats */
    OOP_Class		 *syncclass;	/* describing gfxmode sync times */
    
    
    OOP_Class		 *planarbmclass;
    OOP_Class		 *chunkybmclass;

    /*
       Pixel format "database". This is a list
       of all pixelformats currently used bu some bitmap.
       The point of having this as a central db in the gfx hidd is
       that if several bitmaps are of the same pixel format
       they may point to the same PixFmt object instead
       of allocating their own instance. Thus we are saving mem
     */
    struct SignalSemaphore pfsema;
    struct MinList pflist;
    /* Index of standard pixelformats for quick access */
    HIDDT_PixelFormat	 *std_pixfmts[num_Hidd_StdPixFmt];

    HIDDT_RGBConversionFunction rgbconvertfuncs[NUM_RGB_STDPIXFMT][NUM_RGB_STDPIXFMT];
    struct SignalSemaphore rgbconvertfuncs_sem;
};

#define __IHidd_BitMap	    (csd->attrBases[0])
#define __IHidd_Gfx 	    (csd->attrBases[1])
#define __IHidd_GC  	    (csd->attrBases[2])
#define __IHidd_ColorMap    (csd->attrBases[3])
#define __IHidd_Overlay	    (csd->attrBases[4])
#define __IHidd_Sync	    (csd->attrBases[5])
#define __IHidd_PixFmt      (csd->attrBases[6])
#define __IHidd_PlanarBM    (csd->attrBases[7])
#define __IHidd_ChunkyBM    (csd->attrBases[8])

#undef HiddGfxBase
#undef HiddBitMapBase
#undef HiddColorMapBase
#undef HiddGCBase
#define HiddBitMapBase	    (csd->methodBases[0])
#define HiddGfxBase	    (csd->methodBases[1])
#define HiddGCBase	    (csd->methodBases[2])
#define HiddColorMapBase    (csd->methodBases[3])

/* Library base */

struct IntHIDDGraphicsBase
{
    struct Library            hdg_LibNode;

    struct class_static_data  hdg_csd;
};


/* pre declarations */

BOOL parse_pixfmt_tags(struct TagItem *tags, HIDDT_PixelFormat *pf, ULONG attrcheck, struct class_static_data *csd);

static inline ULONG color_distance(UWORD a1, UWORD r1, UWORD g1, UWORD b1, UWORD a2, UWORD r2, UWORD g2, UWORD b2)
{
    LONG da = (a1 >> 8) - (a2 >> 8);
    LONG dr = (r1 >> 8) - (r2 >> 8);
    LONG dg = (g1 >> 8) - (g2 >> 8);
    LONG db = (b1 >> 8) - (b2 >> 8);

    DB2(bug("[color_distance] a1 = 0x%04X a2 = 0x%04X da = %d\n", a1, a2, da));
    DB2(bug("[color_distance] r1 = 0x%04X r2 = 0x%04X dr = %d\n", r1, r2, dr));
    DB2(bug("[color_distance] g1 = 0x%04X g2 = 0x%04X dg = %d\n", g1, g2, dg));
    DB2(bug("[color_distance] b1 = 0x%04X b2 = 0x%04X db = %d\n", b1, b2, db));

    /* '4' here is a result of trial and error. The idea behind this is to increase
       the weight of alpha difference in order to make the function prefer colors with
       the same alpha value. This is important for correct mouse pointer remapping. */
    return da*da*4 + dr*dr + dg*dg + db*db;
}

#define CSD(x) (&((struct IntHIDDGraphicsBase *)x->UserData)->hdg_csd)
#define csd CSD(cl)

/* The following calls are optimized by calling the method functions directly */

#if USE_FAST_GETPIXEL
static inline HIDDT_Pixel GETPIXEL(OOP_Class *cl, OOP_Object *o, WORD x, WORD y)
{
    struct pHidd_BitMap_GetPixel get_p;

    get_p.mID = HiddBitMapBase + moHidd_BitMap_GetPixel;
    get_p.x   = x;
    get_p.y   = y;

    return HBM(o)->getpixel(HBM(o)->getpixel_Class, o, &get_p.mID);
}
#else
#define GETPIXEL(cl, obj, x, y) HIDD_BM_GetPixel(obj, x, y)
#endif

#if USE_FAST_PUTPIXEL
static inline void PUTPIXEL(OOP_Class *cl, OOP_Object *o, WORD x, WORD y, HIDDT_Pixel val)
{
    struct pHidd_BitMap_PutPixel put_p;

    put_p.mID   = HiddBitMapBase + moHidd_BitMap_PutPixel;
    put_p.x     = x;
    put_p.y     = y;
    put_p.pixel = val;

    HBM(o)->putpixel(HBM(o)->putpixel_Class, o, &put_p.mID);
}
#else
#define PUTPIXEL(cl, obj, x, y, val) HIDD_BM_PutPixel(obj, x, y, val)
#endif

#if USE_FAST_DRAWPIXEL
static inline void DRAWPIXEL(OOP_Class *cl, OOP_Object *o, OOP_Object *gc, WORD x, WORD y)
{
    struct pHidd_BitMap_DrawPixel draw_p;

    draw_p.mID = HiddBitMapBase + moHidd_BitMap_DrawPixel;
    draw_p.gc  = gc;
    draw_p.x   = x;
    draw_p.y   = y;

    HBM(o)->drawpixel(HBM(o)->drawpixel_Class, o, &draw_p.mID);
}
#else
#define DRAWPIXEL(cl, obj, gc, x, y) HIDD_BM_PutPixel(obj, gc, x, y)
#endif

#endif /* GRAPHICS_HIDD_INTERN_H */
