#include <unistd.h>
#include <sys/types.h>
#include "pf.h"
#include "rm_internal.h"


RM_Manager::RM_Manager(PF_Manager &pfm){
  // store the page manager
  this->pfm = pfm;
  
}

RM_Manager::~RM_Manager(){}

RC RM_Manager::CreateFile (const char *fileName, int recordSize) { 
  RC rc;
  if(recordSize <= 0 || recordSize > PF_PAGE_SIZE)
    return RM_BADRECORDSIZE;

  pfm.CreateFile(fileName);

  struct RM_FileHeader header;
  header.recordSize = recordSize;
  header.bitmapSize = RM_FileHandle::NumBitsToCharSize(recordSize);
  header.bitmapOffset = sizeof(struct RM_PageHeader);

  header.numPages = 1;
  header.firstFreePage = NO_MORE_FREE_PAGES;

  int numRecordsPerPage = (PF_PAGE_SIZE - (header.bitmapSize) - (header.bitmapOffset))/recordSize;
  if(numRecordsPerPage <= 0)
    return RM_BADRECORDSIZE;
  header.numRecordsPerPage = numRecordsPerPage;

  PF_PageHandle ph;
  PF_FileHandle fh;
  if((rc = pfm.OpenFile(fileName, fh)))
    return (rc);
  if((rc = fh.AllocatePage(ph)))
    return (rc);

  PageNum page;
  if((rc = ph.GetPageNum(page)) || (rc = fh.UnpinPage(page))){
    return (rc);
  }

  char *pData;
  if((rc = ph.GetData(pData))){
    return (rc);
  }

  memcpy(pData, &header, sizeof(struct RM_FileHeader));

  if((rc = fh.MarkDirty(page))){
    return (rc);
  }

  if((rc = pfm.CloseFile(fh)))
    return (rc);

  // error: check size of record 
  // open file with PF_Manager
  // store record size/filename

  // Call SetUpFileHeader

  return (0); 
}


RC RM_Manager::DestroyFile(const char *fileName) {
  RC rc;
  if((rc = pfm.DestroyFile(fileName)))
    return (rc);
  // call PF_manager destroy file
  return (0); 
}

RC RM_Manager::SetUpFH(RM_FileHandle& fileHandle, PF_FileHandle *fh, struct RM_FileHeader* header){
  fileHandle.header = new struct RM_FileHeader();
  memcpy(fileHandle.header, header, sizeof(struct RM_FileHeader));
  fileHandle.pfh = new PF_FileHandle();
  *fileHandle.pfh = *fh;
  fileHandle.header_modified = false;

  // TODO: Check for vulnerabilities
  return (0);
}

RC RM_Manager::OpenFile   (const char *fileName, RM_FileHandle &fileHandle){
  RC rc;
  PF_FileHandle fh;
  if((rc = pfm.OpenFile(fileName, fh)))
    return (rc);

  PF_PageHandle ph;
  PageNum page;
  if((rc = fh.GetFirstPage(ph)) || (ph.GetPageNum(page)) || (rc = fh.UnpinPage(page)))
    return (rc);

  
  char *pData;
  if((rc = ph.GetData(pData)))
    return (rc);
  struct RM_FileHeader * header = (struct RM_FileHeader *) pData;

  if((rc = SetUpFH(fileHandle, &fh, header)))
    return (rc);

  if(!fileHandle.isValidFileHeader())
    return (RM_INVALIDFILE);

  // call pf_open file
  
    // ERROR - if already exists a PM_FileHandle in this FileHAndle
  // ERROR - if invalid PM_FileHandle
  // Retrieve Header and save
  // ERROR - if header doesnt exist, or first page doesnt exist
  // Set modified boolean
  // Save PM_FileHandler
  // Unpin page

  // Check file header credentials
  // ERROR - invalidFileCredentials

  // return file handle
  return (0); 
}

RC RM_Manager::CleanUpFH(RM_FileHandle &fileHandle){
  if(fileHandle.header == NULL || fileHandle.pfh == NULL)
    return RM_NULLFILEHANDLE;
  delete fileHandle.header;
  delete fileHandle.pfh;
  return (0);
}

RC RM_Manager::CloseFile  (RM_FileHandle &fileHandle) {
  // CHECK FILE HEADER CREDENTIALS
  RC rc;

  if(fileHandle.header_modified == true){
    PF_PageHandle ph;
    if((rc = fileHandle.pfh->GetFirstPage(ph)))
      return (rc);
    char *pData;
    if((rc = ph.GetData(pData)))
      return (rc);
    memcpy(pData, fileHandle.header, sizeof(struct RM_FileHeader));
    PageNum page;
    if((rc = ph.GetPageNum(page)))
      return (rc);
    if((rc = fileHandle.pfh->MarkDirty(page)) || (rc = fileHandle.pfh->UnpinPage(page)))
      return (rc);

  }

  if((rc = pfm.CloseFile(*fileHandle.pfh)))
    return (rc);

  if((rc = CleanUpFH(fileHandle)))
    return (rc);
  // Check if modified
  // If so, get first page
  // write to it
  // mark as dirty
  // unpin
  // flush page to disk

  // set PM_FIleHAndle to NULL

  // call pf_close file
  return (0);
}

