/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Graphics planar bitmap class implementation.
    Lang: english
*/

/****************************************************************************************/

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/oop.h>

#include <exec/memory.h>
#include <utility/tagitem.h>
#include <graphics/gfx.h>
#include <oop/oop.h>

#include <hidd/graphics.h>

#include <string.h>

#include "graphics_intern.h"

#include <string.h>

#define DEBUG 0
#include <aros/debug.h>

/****************************************************************************************/

struct planarbm_data
{
    UBYTE   **planes;
    ULONG   planebuf_size;
    ULONG   bytesperrow;
    ULONG   rows;
    UBYTE   depth;
    BOOL    planes_alloced;
};

#define csd ((struct class_static_data *)cl->UserData)

/****************************************************************************************/

static OOP_Object *planarbm_new(OOP_Class *cl, OOP_Object *o, struct pRoot_New *msg)
{
    ULONG width, height, depth;
    
    UBYTE alignoffset	= 15;
    UBYTE aligndiv	= 2;
    BOOL  ok = TRUE;   
     
#if 0
    	/* Set the bitmaps' pixelformat */
    struct TagItem pf_tags[] =
    {
      	{ aHidd_PixFmt_ColorModel   , vHidd_ColorModel_Palette	}, /* 0 */
      	{ aHidd_PixFmt_Depth	    , 0				}, /* 1 */
      	{ aHidd_PixFmt_BytesPerPixel, 0				}, /* 2 */
      	{ aHidd_PixFmt_BitsPerPixel , 0				}, /* 3 */
      	{ aHidd_PixFmt_StdPixFmt    , 0				}, /* 4 */
      	{ aHidd_PixFmt_CLUTShift    , 0				}, /* 5 */
	{ aHidd_PixFmt_CLUTMask     , 0x000000FF		}, /* 6 */
	{ aHidd_PixFmt_RedMask	    , 0x00FF0000		}, /* 7 */
	{ aHidd_PixFmt_GreenMask    , 0x0000FF00		}, /* 8 */
	{ aHidd_PixFmt_BlueMask     , 0x000000FF		}, /* 9 */
	{ aHidd_PixFmt_BitMapType   , vHidd_BitMapType_Planar	},
	{ TAG_DONE  	    	    , 0UL   	    	    	}
    };
#endif
    
    struct planarbm_data *data;
    OOP_Object      	 *pf;

    o =(OOP_Object *)OOP_DoSuperMethod(cl, o, (OOP_Msg)msg);
    if (NULL == o)
    	return NULL;
	
    data = OOP_INST_DATA(cl, o);
    memset(data, 0, sizeof  (*data));
    
   
    /* Get some data about the dimensions of the bitmap */

    data->planes_alloced = (BOOL)GetTagData(aHidd_PlanarBM_AllocPlanes, TRUE, msg->attrList);
    
    #warning Fix this hack
    /* Because this class is used to emulate Amiga bitmaps, we
       have to see if it should have late initalisation
    */
    if (!data->planes_alloced)
	return o; /* Late initialization */
	

    /* Not late initalization. Get some info on the bitmap */	
    OOP_GetAttr(o, aHidd_BitMap_Width,	&width);
    OOP_GetAttr(o, aHidd_BitMap_Height,	&height);
    OOP_GetAttr(o,  aHidd_BitMap_PixFmt, (IPTR *)&pf);
    OOP_GetAttr(pf, aHidd_PixFmt_Depth, (IPTR *)&depth);
    
    /* We cache some info */
    data->bytesperrow	  = (width + alignoffset) / aligndiv;
    data->rows 		  = height;
    data->depth		  = depth;
    
    if (ok)
    {
	/* Allocate memory for plane array */
	data->planes = AllocVec(sizeof (UBYTE *) * depth, MEMF_ANY|MEMF_CLEAR);
	if (NULL == data->planes)
	    ok = FALSE;
	else
	{
	    UBYTE i;

	    data->planebuf_size = depth;

	    /* Allocate all the planes */
	    for ( i = 0; i < depth && ok; i ++)
	    {
	    	data->planes[i] = AllocVec(height * data->bytesperrow, MEMF_ANY|MEMF_CLEAR);
	    	if (NULL == data->planes[i])
	    	    ok = FALSE;
	    }
	}
    }
      
    if (!ok)
    {
	OOP_MethodID dispose_mid;
    
	dispose_mid = OOP_GetMethodID(IID_Root, moRoot_Dispose);
	OOP_CoerceMethod(cl, o, (OOP_Msg)&dispose_mid);
	
	o = NULL;
    }
    	
    return o;
}

/****************************************************************************************/

static VOID planarbm_dispose(OOP_Class *cl, OOP_Object *o, OOP_Msg msg)
{
    struct planarbm_data    *data;
    UBYTE   	    	    i;
    
    data = OOP_INST_DATA(cl, o);
    
    if (data->planes_alloced)
    {
	if (NULL != data->planes)
	{
    	    for (i = 0; i < data->depth; i ++)
	    {
		if (NULL != data->planes[i])
		{
		    FreeVec(data->planes[i]);
		}
	    }
	    FreeVec(data->planes);
	}
    }
    
    OOP_DoSuperMethod(cl, o, msg);
    
    return;
}

/****************************************************************************************/

static VOID planarbm_putpixel(OOP_Class *cl, OOP_Object *o,
    	    	    	      struct pHidd_BitMap_PutPixel *msg)
{
    UBYTE   	    	    **plane;
    struct planarbm_data    *data;
    ULONG   	    	    offset;
    ULONG   	    	    mask;
    UBYTE   	    	    pixel, notpixel;
    UBYTE   	    	    i;
    
    data = OOP_INST_DATA(cl, o);

    /* bitmap in plane-mode */
    plane     = data->planes;
    offset    = msg->x / 8 + msg->y * data->bytesperrow;
    pixel     = 128 >> (msg->x % 8);
    notpixel  = ~pixel;
    mask      = 1;

    for(i = 0; i < data->depth; i++, mask <<=1, plane ++)
    {  
    	if ((*plane != NULL) && (*plane != (UBYTE *)-1))
	{	
	    if(msg->pixel & mask)
	    {
		*(*plane + offset) = *(*plane + offset) | pixel;
	    }
	    else
	    {
		*(*plane + offset) = *(*plane + offset) & notpixel;
	    }
        }
    }
}

/****************************************************************************************/

static ULONG planarbm_getpixel(OOP_Class *cl, OOP_Object *o,
    	    	    	       struct pHidd_BitMap_GetPixel *msg)
{
    struct planarbm_data    *data;
    UBYTE   	    	    **plane;
    ULONG   	    	    offset;
    ULONG   	    	    i;
    UBYTE   	    	    pixel;
    ULONG   	    	    retval;
         
    data = OOP_INST_DATA(cl, o);

    plane     = data->planes;
    offset    = msg->x / 8 + msg->y * data->bytesperrow;
    pixel     = 128 >> (msg->x % 8);
    retval    = 0;

    for(i = 0; i < data->depth; i++, plane ++)
    {
    
        if (*plane == (UBYTE *)-1)
	{
	    retval = retval | (1 << i);
	}
	else if (*plane != NULL)
	{	
	    if(*(*plane + offset) & pixel)
	    {
		retval = retval | (1 << i);
	    }
	}
    }
    
    return retval; 
}

/****************************************************************************************/

static VOID planarbm_putimagelut(OOP_Class *cl, OOP_Object *o,
    	    	    	    struct pHidd_BitMap_PutImageLUT *msg)
{
    WORD    	    	    x, y, d;
    UBYTE   	    	    *pixarray = (UBYTE *)msg->pixels;
    UBYTE   	    	    **plane;
    ULONG   	    	    planeoffset;
    struct planarbm_data   *data;  
    
    data = OOP_INST_DATA(cl, o);
    
    planeoffset = msg->y * data->bytesperrow + msg->x / 8;
    
    for(y = 0; y < msg->height; y++)
    {
    	UBYTE *src = pixarray;
	
    	plane = data->planes;
	
    	for(d = 0; d < data->depth; d++)
	{
	    ULONG dmask = 1L << d;
	    ULONG pmask = 0x80 >> (msg->x & 7);
	    UBYTE *pl = *plane;
	    
	    if (pl == (UBYTE *)-1) continue;
	    if (pl == NULL) continue;
	    
	    pl += planeoffset;

    	    for(x = 0; x < msg->width; x++)
	    {
	    	if (src[x] & dmask)
		{
		    *pl |= pmask;
		}
		else
		{
		    *pl &= ~pmask;
		}
		
		if (pmask == 0x1)
		{
		    pmask = 0x80;
		    pl++;
		}
		else
		{
		    pmask >>= 1;
		}
		
	    } /* for(x = 0; x < msg->width; x++) */
	    
	    plane++;
	    
	} /* for(d = 0; d < data->depth; d++) */
	
	pixarray += msg->modulo;
	planeoffset += data->bytesperrow;
	
    } /* for(y = 0; y < msg->height; y++) */
}

/****************************************************************************************/

static VOID planarbm_getimagelut(OOP_Class *cl, OOP_Object *o,
    	    	    	    struct pHidd_BitMap_GetImageLUT *msg)
{
    WORD    	    	    x, y, d;
    UBYTE   	    	    *pixarray = (UBYTE *)msg->pixels;
    UBYTE   	    	    **plane;
    ULONG   	    	    planeoffset;
    struct planarbm_data    *data;  
    UBYTE   	    	    prefill;
    
    data = OOP_INST_DATA(cl, o);
    
    planeoffset = msg->y * data->bytesperrow + msg->x / 8;
    
    prefill = 0;
    for(d = 0; d < data->depth; d++)
    {
    	if (data->planes[d] == (UBYTE *)-1)
	{
	    prefill |= (1L << d);
	}
    }
    
    for(y = 0; y < msg->height; y++)
    {
    	UBYTE *dest = pixarray;
	
    	plane = data->planes;
	
	for(x = 0; x < msg->width; x++)
	{
	    dest[x] = prefill;
	}
	
    	for(d = 0; d < data->depth; d++)
	{
	    ULONG dmask = 1L << d;
	    ULONG pmask = 0x80 >> (msg->x & 7);
	    UBYTE *pl = *plane;
	    
	    if (pl == (UBYTE *)-1) continue;
	    if (pl == NULL) continue;
	    
	    pl += planeoffset;

    	    for(x = 0; x < msg->width; x++)
	    {
	    	if (*pl & pmask)
		{
		    dest[x] |= dmask;
		}
		else
		{
		    dest[x] &= ~dmask;
		}
		
		if (pmask == 0x1)
		{
		    pmask = 0x80;
		    pl++;
		}
		else
		{
		    pmask >>= 1;
		}
		
	    } /* for(x = 0; x < msg->width; x++) */
	    
	    plane++;
	    
	} /* for(d = 0; d < data->depth; d++) */
	
	pixarray += msg->modulo;
	planeoffset += data->bytesperrow;
	
    } /* for(y = 0; y < msg->height; y++) */
    
}

/****************************************************************************************/

BOOL planarbm_setbitmap(OOP_Class *cl, OOP_Object *o,
    	    	    	struct pHidd_PlanarBM_SetBitMap *msg)
{
    struct planarbm_data *data;
    struct BitMap   	 *bm;
    
    struct TagItem  	 pftags[] =
    {
    	{ aHidd_PixFmt_Depth	    , 0UL			},	/* 0 */
    	{ aHidd_PixFmt_BitsPerPixel , 0UL			},	/* 1 */
    	{ aHidd_PixFmt_BytesPerPixel, 1UL			},	/* 2 */
    	{ aHidd_PixFmt_ColorModel   , vHidd_ColorModel_Palette	},	/* 3 */
    	{ aHidd_PixFmt_BitMapType   , vHidd_BitMapType_Planar	},	/* 4 */
    	{ aHidd_PixFmt_CLUTShift    , 0UL			},	/* 5 */
    	{ aHidd_PixFmt_CLUTMask     , 0x000000FF		},	/* 6 */
	{ aHidd_PixFmt_RedMask	    , 0x00FF0000		}, 	/* 7 */
	{ aHidd_PixFmt_GreenMask    , 0x0000FF00		}, 	/* 8 */
	{ aHidd_PixFmt_BlueMask     , 0x000000FF		}, 	/* 9 */
    	{ TAG_DONE  	    	    , 0UL   	    	    	}	/* 7 */
    };
    struct TagItem  	bmtags[] =
    {
	{ aHidd_BitMap_Width	    , 0UL },
	{ aHidd_BitMap_Height	    , 0UL },
    	{ aHidd_BitMap_PixFmtTags   , 0UL },
	{ TAG_DONE  	    	    , 0UL }
    };
	
    ULONG i;
    
    data = OOP_INST_DATA(cl, o);
    bm = msg->bitMap;
    
    if (data->planes_alloced)
    {
    	D(bug(" !!!!! PlanarBM: Trying to set bitmap in one that allready has planes allocated\n"));
	return FALSE;
    }
    
    /* Check if plane array allready allocated */
    if (NULL != data->planes)
    {
    	if (bm->Depth > data->planebuf_size)
	{
	    FreeVec(data->planes);
	    data->planes = NULL;
	}
    }
    
    if (NULL == data->planes)
    {
	data->planes = AllocVec(sizeof (UBYTE *) * bm->Depth, MEMF_CLEAR);

	if (NULL == data->planes)
	     return FALSE;
	     
	data->planebuf_size = bm->Depth;
    }
    
    
    /* Update the planes */
    for (i = 0; i < data->planebuf_size; i ++)
    {
    	if (i < bm->Depth) 
   	    data->planes[i] = bm->Planes[i];
	else
	    data->planes[i] = NULL;
    }

    data->depth		= bm->Depth;
    data->bytesperrow	= bm->BytesPerRow;
    data->rows		= bm->Rows;
    
    pftags[0].ti_Data = bm->Depth;	/* PixFmt_Depth */
    pftags[1].ti_Data = bm->Depth;	/* PixFmt_BitsPerPixel */
    
    bmtags[0].ti_Data = bm->BytesPerRow * 8;
    bmtags[1].ti_Data = bm->Rows;
    bmtags[2].ti_Data = (IPTR)pftags;
    
    /* Call private bitmap method to update superclass */
    if (!HIDD_BitMap_SetBitMapTags(o, bmtags))
    {
    	ULONG i;
	
	for (i = 0; i < data->planebuf_size; i ++)
	{
	    data->planes[i] = NULL;
	}
    }

    return TRUE;
}

/****************************************************************************************/

#undef OOPBase
#undef SysBase

#undef csd

#define OOPBase (csd->oopbase)
#define SysBase (csd->sysbase)

#define NUM_ROOT_METHODS   2
#define NUM_BITMAP_METHODS 4
#define NUM_PLANARBM_METHODS 1

/****************************************************************************************/

OOP_Class *init_planarbmclass(struct class_static_data *csd)
{
    struct OOP_MethodDescr root_descr[NUM_ROOT_METHODS + 1] =
    {
        {(IPTR (*)())planarbm_new    , moRoot_New    	},
        {(IPTR (*)())planarbm_dispose, moRoot_Dispose	},
        {NULL	    	    	     , 0UL  	    	}
    };

    struct OOP_MethodDescr bitmap_descr[NUM_BITMAP_METHODS + 1] =
    {
        {(IPTR (*)())planarbm_putpixel	    , moHidd_BitMap_PutPixel	},
        {(IPTR (*)())planarbm_getpixel	    , moHidd_BitMap_GetPixel	},
	{(IPTR (*)())planarbm_putimagelut   , moHidd_BitMap_PutImageLUT },
	{(IPTR (*)())planarbm_getimagelut   , moHidd_BitMap_GetImageLUT },
        {NULL	    	    	    	    , 0UL	    	    	}
    };

    struct OOP_MethodDescr planarbm_descr[NUM_PLANARBM_METHODS + 1] =
    {
        {(IPTR (*)())planarbm_setbitmap	, moHidd_PlanarBM_SetBitMap },
        {NULL	    	    	    	, 0UL	    	    	    }
    };
    
    struct OOP_InterfaceDescr ifdescr[] =
    {
        {root_descr 	, IID_Root          , NUM_ROOT_METHODS	    },
        {bitmap_descr	, IID_Hidd_BitMap   , NUM_BITMAP_METHODS    },
        {planarbm_descr , IID_Hidd_PlanarBM , NUM_PLANARBM_METHODS  },
        {NULL	    	, NULL	    	    , 0     	    	    }
    };

    OOP_AttrBase MetaAttrBase = OOP_GetAttrBase(IID_Meta);

    struct TagItem tags[] =
    {
        {aMeta_SuperID	    	, (IPTR) CLID_Hidd_BitMap   	    	},
        {aMeta_InterfaceDescr	, (IPTR) ifdescr    	    	    	},
        {aMeta_ID   	    	, (IPTR) CLID_Hidd_PlanarBM 	    	},
        {aMeta_InstSize     	, (IPTR) sizeof(struct planarbm_data)	},
        {TAG_DONE   	    	, 0UL	    	    	    	    	}
    };
    
    OOP_Class *cl = NULL;

    EnterFunc(bug("init_planarbmclass(csd=%p)\n", csd));

    if(MetaAttrBase) 
    {
    	cl = OOP_NewObject(NULL, CLID_HiddMeta, tags);
    	if(NULL != cl)
	{
            D(bug("BitMap class ok\n"));
            csd->planarbmclass = cl;
            cl->UserData     = (APTR) csd;

    	    OOP_AddClass(cl);
        }
	
    } /* if(MetaAttrBase) */
    
    if (NULL == cl)
	free_planarbmclass(csd);

    ReturnPtr("init_planarbmclass", OOP_Class *,  cl);
}

/****************************************************************************************/

void free_planarbmclass(struct class_static_data *csd)
{
    EnterFunc(bug("free_planarbmclass(csd=%p)\n", csd));

    if(NULL != csd)
    {  
	if (NULL != csd->planarbmclass)
	{
    	    OOP_RemoveClass(csd->planarbmclass);
	    OOP_DisposeObject((OOP_Object *) csd->planarbmclass);
    	    csd->planarbmclass = NULL;
	}
    }

    ReturnVoid("free_planarbmclass");
}

/****************************************************************************************/
