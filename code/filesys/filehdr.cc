// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::FileHeader
//	There is no need to initialize a fileheader,
//	since all the information should be initialized by Allocate or FetchFrom.
//	The purpose of this function is to keep valgrind happy.
//----------------------------------------------------------------------
FileHeader::FileHeader()
{
	numBytes = -1;
	numSectors = -1;
        fileDescriptor=-1;
        numLevel = 0;
	memset(dataSectors, -1, sizeof(dataSectors));
}

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::~FileHeader
//	Currently, there is not need to do anything in destructor function.
//	However, if you decide to add some "in-core" data in header
//	Always remember to deallocate their space or you will leak memory
//----------------------------------------------------------------------
FileHeader::~FileHeader()
{
	// nothing to do now
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{ 
    numBytes        = fileSize;
    numSectors      = divRoundUp(fileSize, SectorSize);
    int allocSecNum = 0;
    if (freeMap->NumClear() < numSectors)
	      return FALSE;		// not enough space
		DEBUG(dbgFile, "numSectors = "<< numSectors);
    
    indirectTable *indirTbl = new indirectTable;
	  memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy
	  dataSectors[0] = freeMap->FindAndSet();
    numLevel = (int)(log10(numSectors)/log10(32))+1; //see how many indirect level file need
		DEBUG(dbgFile, "nLevel = "<<numLevel<<", allocSecNum="<<allocSecNum);
    AllocSector(freeMap, numLevel, indirTbl, &allocSecNum, numSectors);
    kernel->synchDisk->WriteSector(dataSectors[0], (char *)indirTbl); 
    delete indirTbl;
    return TRUE;
    /***
    int numseclevel = 1;
    for(num=0;num<nemLevl;num++)
    {
      for(sec=0;sec<numseclevel;sec++){
          freeMap->FindAndSet();
      }  
      numseclevel*=NumInDirect;
    }

    **/

    /*
    if(numSectors>NumDirect)
      numInDir = divRoundUp(numSectors, NumInDirect);
    
    if(freeMap->NumClear()<(numSectors+numInDir))
      return FALSE;

    if(numInDir>0){
        int fillSectorNum = 0;
        indirectTable *indirTbl = new indirectTable;
        for(int i = 0; i<numInDir; i++){
            dataSectors[i] = freeMap->FindAndSet();
	          memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy
            for(int j = 0; (j<NumInDirect)&&(fillSectorNum<numSectors); j++){
               indirTbl->dataSectors[j] = freeMap->FindAndSet();
               fillSectorNum++;
            } 
            kernel->synchDisk->WriteSector(dataSectors[i], (char *)indirTbl); 

        }
        delete indirTbl;
    }else{
        for (int i = 0; i < numSectors; i++) {
	          dataSectors[i] = freeMap->FindAndSet();
	          // since we checked that there was enough free space,
	          // we expect this to succeed
	          ASSERT(dataSectors[i] >= 0);
        }
    }
    return TRUE;*/
}


//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(PersistentBitmap *freeMap)
{
   //if(numInDir>0){
   //int tmpNumSector = 0;
   int deallocSecNum = 0;
   indirectTable *indirTbl = new indirectTable;
	 memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy
   kernel->synchDisk->ReadSector(dataSectors[0], (char *)indirTbl);

   DeallocSector(freeMap, numLevel, indirTbl, &deallocSecNum, numSectors);
   delete indirTbl;
   /*
   indirectTable *indirTbl = new indirectTable;
   for (int j=0;j<numInDir;j++){  
	    memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy
      kernel->synchDisk->ReadSector(dataSectors[j], (char *)indirTbl);
      for (int i = 0; (i < NumInDirect)&&(tmpNumSector<numSectors); i++) {
	        ASSERT(freeMap->Test((int) indirTbl->dataSectors[i]));  // ought to be marked!
	        freeMap->Clear((int) indirTbl->dataSectors[i]);
      }
	    freeMap->Clear((int) dataSectors[j]);
   }
   delete indirTbl;
   }else{
      for (int i = 0; (i < numSectors); i++) {
	        ASSERT(freeMap->Test((int)dataSectors[i]));  // ought to be marked!
	        freeMap->Clear((int) dataSectors[i]);
      }
   
   }*/
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    kernel->synchDisk->ReadSector(sector, (char *)this);
	
	/*
		MP4 Hint:
		After you add some in-core informations, you will need to rebuild the header's structure
	*/
	
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    kernel->synchDisk->WriteSector(sector, (char *)this); 
	
	/*
		MP4 Hint:
		After you add some in-core informations, you may not want to write all fields into disk.
		Use this instead:
		char buf[SectorSize];
		memcpy(buf + offset, &dataToBeWritten, sizeof(dataToBeWritten));
		...
	*/
	
}

int
GetSector(int secNum,int LvlNum, indirectTable *tbl)
{
    int thesector = secNum/LvlNum;
    int nextlevl  = secNum%LvlNum;
    int reSec     = -1;

		//DEBUG(dbgFile, "[GetSector]thesector = "<< thesector);
		//DEBUG(dbgFile, "[GetSector]nextlevl = "<< nextlevl<<", levelnum="<<LvlNum);

    
    if(LvlNum==1){
       return  tbl->dataSectors[thesector];
    }else{
        indirectTable *indirTbl = new indirectTable;
        kernel->synchDisk->ReadSector(tbl->dataSectors[thesector], (char *)indirTbl);
        reSec = GetSector(nextlevl,LvlNum/NumInDirect,indirTbl);
        delete indirTbl;
        return reSec;
    }
}
//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{


  int  sector = -1;
  
  indirectTable *indirTbl = new indirectTable;
	memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy
  kernel->synchDisk->ReadSector(dataSectors[0], (char *)indirTbl);

  sector = GetSector(offset/SectorSize,pow(NumInDirect,numLevel-1),indirTbl);
  
  delete indirTbl;
  /*
  if(numInDir>0){
       int numSec = offset/SectorSize; 
       int numIndr = numSec/NumInDirect;
       int secOffset = numSec%NumInDirect;
       indirectTable *indirTbl = new indirectTable;
       kernel->synchDisk->ReadSector(dataSectors[numIndr], (char *)indirTbl);
       sector = indirTbl->dataSectors[secOffset];
       delete indirTbl;
   }else{
        sector=(dataSectors[offset / SectorSize]);
   }
*/
  return sector;
}


//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

int FileHeader::GetFd()
{
    return fileDescriptor;
}

int FileHeader::SetFd(int fd)
{
    fileDescriptor = fd;
    return fileDescriptor;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];
/*
    int tmpNumSector = 0;
  indirectTable *indirTbl = new indirectTable;
   for (int j=0;j<numInDir;j++){  
	    memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy
      kernel->synchDisk->ReadSector(dataSectors[j], (char *)indirTbl);
      for (int i = 0; (i < NumInDirect)&&(tmpNumSector<numSectors); i++) {
	        ASSERT(freeMap->Test((int) indirTbl->dataSectors[i]));  // ought to be marked!
	        freeMap->Clear((int) indirTbl->dataSectors[i]);
      }
	    freeMap->Clear((int) dataSectors[j]);
   }
   delete indirTbl;
*/
    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	kernel->synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}


bool
FileHeader::AllocSector(PersistentBitmap *freeMap, int n, indirectTable *tbl,int *allocSecNum, int needSecNum)
{ 
    if(n<=0||((*allocSecNum)==needSecNum)) return FALSE;
    n--;
    indirectTable *indirTbl = NULL;;
    for(int i=0;(i<NumInDirect)&&((*allocSecNum)<needSecNum);i++){
        tbl->dataSectors[i]=freeMap->FindAndSet();
		    DEBUG(dbgFile, "i="<<i<<",Alloc sectors = "<<tbl->dataSectors[i]<<",n="<<n);
        indirTbl = new indirectTable;
	      memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy
        if(AllocSector(freeMap,n,indirTbl,allocSecNum,needSecNum)){
            kernel->synchDisk->WriteSector(tbl->dataSectors[i], (char *)indirTbl);
        }
        else{
          if((*allocSecNum)<needSecNum){ 
            (*allocSecNum)++;
          }
        }
        
		    DEBUG(dbgFile, "allocSecNum = "<< (*allocSecNum));
        delete indirTbl;
    }
    return TRUE;
}



bool 
FileHeader::DeallocSector(PersistentBitmap *freeMap, int n, indirectTable *tbl, int *deallocSecNum, int needSecNum)
{
    if(n<=0) return FALSE;
    n--;
    indirectTable *indirTbl = new indirectTable;
	  memset(indirTbl, -1, sizeof(indirectTable));  // dummy operation to keep valgrind happy


    for(int i=0;(i<NumInDirect)&&((*deallocSecNum)<needSecNum);i++)
    {
      kernel->synchDisk->ReadSector(tbl->dataSectors[i], (char *)indirTbl);
      if(!DeallocSector(freeMap,n,indirTbl,deallocSecNum, needSecNum)){
        (*deallocSecNum)++;
		    DEBUG(dbgFile, "i="<<i<<", deAlloc Sector= "<<(*deallocSecNum)<<",n="<<n);

      }
      freeMap->Clear((int) tbl->dataSectors[i]);
    }

		    DEBUG(dbgFile, "total Sector Num="<<needSecNum);
    delete indirTbl;
    
    return TRUE;

}
