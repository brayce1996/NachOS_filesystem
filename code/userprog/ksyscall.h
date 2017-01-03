/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"

#include "synchconsole.h"


void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

#ifdef FILESYS_STUB
int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename);
}
#endif

#ifndef FILESYS_STUB
int SysCreate(char *filename, int initSize)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename,initSize);
}

int SysOpen(char *filename)
{
	return kernel->interrupt->OpenFile(filename);
}

int SysClose(int fd)
{
	return kernel->interrupt->CloseFile(fd);
}

int SysWrite(char *buf, int size, int fd)
{
	return kernel->interrupt->WriteFile(buf, size, fd);
}

int SysRead(char *buf, int size, int fd)
{
	return kernel->interrupt->ReadFile(buf, size, fd);
}

int SysRemove(char *filename)
{
	return kernel->interrupt->RemoveFile(filename);
}

int SysSeek(int position, OpenFileId id)
{
	return kernel->interrupt->SeekFile(position, id);
}
#endif

#endif /* ! __USERPROG_KSYSCALL_H__ */
