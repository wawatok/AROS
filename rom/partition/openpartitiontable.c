/*
    Copyright � 1995-2017, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <exec/memory.h>
#include <proto/exec.h>
#include <aros/debug.h>

#include "partition_intern.h"
#include "partition_support.h"
#include "platform.h"

/*****************************************************************************

    NAME */
#include <libraries/partition.h>

        AROS_LH1(LONG, OpenPartitionTable,

/*  SYNOPSIS */
        AROS_LHA(struct PartitionHandle *, root, A1),

/*  LOCATION */
        struct Library *, PartitionBase, 7, Partition)

/*  FUNCTION
        Open a partition table. On success root->list will be filled with a
        list of PartitionHandles. If one partition contains more
        subpartitions, the caller should call OpenPartitionTable() on the
        PartitionHandle recursively.

    INPUTS
        root - root partition

    RESULT
        0 for success; an error code otherwise.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    const struct PTFunctionTable * const *pst;

    bug("%s() started.\n", __FUNCTION__ );

    bug("%s() Checking the partition table against list of supported tables.\n", __FUNCTION__ );
    pst = PartitionSupport;
    while (pst[0])
    {
    	bug("%s() Checking partition table at address 0x%x.\n", __FUNCTION__, (ULONG)pst );
        if (pst[0]->checkPartitionTable(PartitionBase, root))
        {
        	bug("%s() partition table found.\n", __FUNCTION__ );
        	bug("%s() allocating memory.\n", __FUNCTION__ );
            root->table = AllocMem(sizeof(struct PartitionTableHandler), MEMF_PUBLIC | MEMF_CLEAR);

            bug("%s() Checking for table contents.\n", __FUNCTION__ );
            if (root->table)
            {
            	LONG retval;
            	bug("%s() Table contents found.\n", __FUNCTION__ );

            	NEWLIST(&root->table->list);

                root->table->type    = pst[0]->type;
                root->table->handler = (void *)pst[0];

                bug("%s() Opening the partition table.\n", __FUNCTION__ );
                retval = pst[0]->openPartitionTable(PartitionBase, root);
                if (retval)
                {
                	bug("%s() Failed to open partition table.\n", __FUNCTION__ );
                    FreeMem(root->table, sizeof(struct PartitionTableHandler));

                    root->table = NULL;
                }else
                {
                	bug("%s() Opened the partition table successfully.\n", __FUNCTION__ );
                }
                return retval;
            }else
            {
            	bug("%s() Sorry, no contents found.\n", __FUNCTION__ );
            }
        }
        bug("%s() Sadly no match.\n", __FUNCTION__ );
        pst++;
    }
    return 1;

    AROS_LIBFUNC_EXIT
}
