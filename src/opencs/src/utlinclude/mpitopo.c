#ifndef mpitopo_SOURCE_INCLUDED
#define mpitopo_SOURCE_INCLUDED
/* mpitopo.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Find out the MPI topology.
 *    In a multi node environment, with multi core nodes, we need to find out the
 *    "node master" process.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2015/Jan: Carsten Dehning, Initial release
 *    $Id: mpitopo.c 1875 2013-11-04 10:43:00Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmem.h"
#include "mpi.h"

#define DO_TEST 0


C_FUNC_PREFIX int *MPI_topology(int *prank, int *pnprocs)
{
   static int *procnodeids = NULL;
   int         rank,nprocs;



   MPI_Comm_size(MPI_COMM_WORLD,&nprocs); /* find out how many slaves there are */
   MPI_Comm_rank(MPI_COMM_WORLD,&rank);   /* get my rank */

   printf("MPI_topology(rank=%d, nprocs=%d) ... start\n",rank,nprocs);
   if (!procnodeids)
   {
      char *nodenames;
      int   mylen,maxlen,nodeid,i,j;
      char  nodenm[HOST_NAME_MAX];


      /* Get the name of this node and its strlen */
      mylen = sizeof(nodenm);
      MEMZERO(nodenm,sizeof(nodenm));
      MPI_Get_processor_name(nodenm,&mylen);
      mylen = (int)(strlen(nodenm)+1);

      /* Get length of the longest nodename */
      maxlen = -1;
      MPI_Allreduce(&mylen,&maxlen,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
      printf("MPI_topology(rank=%d) ... maxlen=%d, node=\"%s\"\n",rank,maxlen,nodenm);

      procnodeids = (int  *)MALLOC(nprocs*sizeof(int));
      nodenames   = (char *)MALLOC(nprocs*maxlen);

      MPI_Allgather
      (
         nodenm   ,maxlen,MPI_CHAR,
         nodenames,maxlen,MPI_CHAR,
         MPI_COMM_WORLD
      );

      for(i=0; i<nprocs; i++)
      {
         procnodeids[i] = INT_MAX;
      }

      nodeid = -1;
      for(i=0; i<nprocs; i++)
      {
         if (procnodeids[i] == INT_MAX) /* Not visited/unchanged */
         {
            procnodeids[i] = ++nodeid; /* Positive index: master */
            printf("MPI_topology(rank=%d) ... Master(%d): \"%s\"\n",rank,i,nodenames+i*maxlen);
            for(j=i+1; j<nprocs; j++)
            {
               if (procnodeids[j] == INT_MAX && !strcmp(nodenames+i*maxlen,nodenames+j*maxlen))
               {
                  printf("MPI_topology(rank=%d) ... Slave(%d): \"%s\"\n",rank,j,nodenames+j*maxlen);
                  procnodeids[j] = -nodeid; /* Negative index: slave */
               }
            }
         }
      }
      #if 0
      FREE(nodenames);
      #endif
   }

   printf("MPI_topology(rank=%d) ... done\n",rank);
   if (pnprocs) *pnprocs = nprocs;
   if (prank  ) *prank   = rank;
   return procnodeids;
}


#if DO_TEST
int main(int argc, char *argv[])
{
   int *procnodeids;
   int  rank,nprocs;


   MPI_Init(&argc,&argv);
   procnodeids = MPI_topology(&rank,&nprocs);

   printf("FINAL: MyRank=%d, myColor=%d\n",rank,procnodeids[rank]);

   MPI_Finalize();
   return 0;
}
#endif

#undef DO_TEST

#endif
