#ifndef _GRAPH_INTERN_H_
#define _GRAPH_INTERN_H_

#include <exec/types.h>
#include <utility/date.h>
#include <libraries/mui.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>

/* Descriptions and data for a single source .. */
struct Graph_SourceDATA
{
    struct Hook                         *gs_ReadHook;
    WORD    	    	    	        gs_PlotPen;
    WORD    	    	    	        gs_PlotFillPen;
    IPTR                                *gs_Entries;
};

/*** Instance data **********************************************************/
struct Graph_DATA
{
    struct MUI_InputHandlerNode ihn;

    IPTR                                graph_Flags;            // see below

    struct BitMap   	    	        *graph_BitMap;          // Bitmap we render to ...
    struct RastPort 	    	        *graph_RastPort;        // RastPort to render into graph_BitMap

    IPTR                                graph_SourceCount;      // Number of data sources 
    IPTR                                graph_EntryCount;       // Total number of entries in a source's array
    IPTR                                graph_EntryPtr;         // "current" entry in the source's array
    struct Graph_SourceDATA             *graph_Sources;         // Data sources ...

    IPTR                                graph_Max;              // Maximum value of an entry
    IPTR                                graph_Tick;             // tick counter used in periodic rendering

    /* InfoText displayed on the graph ... */
    struct List                         graph_InfoText;        // Text displayed infront of graph
    IPTR                                graph_ITHeight;

    /* used Pens ... */
    
    WORD    	    	    	        graph_BackPen;          // The backrgound pen
    WORD    	    	    	        graph_AxisPen;          // The outer frame pan and larger divisions
    WORD    	    	    	        graph_SegmentPen;       // the secment pen

    UWORD    	    	    	        graph_SegmentSize;      // size of a segment in pixels
};

#define GRAPHF_SETUP    (1 << 0)
#define GRAPHF_HANDLER  (1 << 1)
#define GRAPHF_PERIODIC (1 << 2)
#define GRAPHF_FIXEDLEN (1 << 3)
#define GRAPHF_AGGR     (1 << 4)
#define GRAPHF_CHANGED  (1 << 31)

#endif /* _GRAPH_INTERN_H_ */
