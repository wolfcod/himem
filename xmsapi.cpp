#include <stdio.h>
#include <dos.h>
#include <mem.h>
#include "xmsapi.h"

int (far *xms_control)() = NULL;
static int xms_errno = 0;

typedef unsigned int UINT;

/** return size in number of pages... */
static int size_in_page(size_t size)
{
	if ((size % 16) != 0)
		return ((size / 16) + 1);

	return size / 16;
}

static int size_in_kb(size_t size)
{
	if ((size % 1024) != 0)
		return ((size / 1024) + 1);

   return size / 1024;

}

/** XMSReady return XMS driver status => 0 ok. 1 error */
int XMSReady()
{
	_AX = 0x4300;	// service is installed?
	geninterrupt(0x2f);
	int ret = _AL;

	if (ret == 0x80)
	{
		_AX = 0x4310;	// query xms control pointers
		geninterrupt(0x2f);
		_asm {
			mov word ptr [xms_control], bx
			mov word ptr [xms_control+2], es
		}

		printf("XMSControl found at %Fp\n", xms_control);
		return 0;
	}
	// error .. service not ready!
	return 1;
}

/** XMM Version
 *	ARGS: None
 *	RETS: AX = XMS Version
 *		  BX = Internal Driver Version Number
 *		  DX = 1 if HMA exists, 0 if it doesn't
 */
void XMM_Version(REGS *regs)
{
	regs->x.ax = XMS_VERSION;
	regs->x.bx = HIMEM_VERSION;

	return;
}

static char fHMAInUse = 0;
static char fHMAExists = 0;
static size_t minHMASize = 0;

/** XMM RequestHMA
 * ARGS: bytes (DX)
 * RETS: error or 0 */
int XMM_RequestHMA(REGS *regs, size_t bytes)
{
	if (fHMAInUse == 1)
		return ERR_HMAINUSE;

	if (fHMAExists == 0)
		return ERR_HMANOTEXIST;

	if (minHMASize < bytes)
		return ERR_HMAMINSIZE;

	fHMAInUse = 1;
	regs->x.ax = 1;
	return ERR_NOERROR;
}

int XMM_ReleaseHMA(REGS *regs)
{
	if (fHMAInUse == 0)
		return ERR_HMANOTALLOCED;

	regs->x.ax = 1;
	fHMAInUse = 0;
	return ERR_NOERROR;
}

static char fCanChangeA20 = 0;
static int EnableCount = 0;

static int (*a20Handler)(int enable) = NULL;

int XMM_LocalEnableA20(REGS *regs)
{
	return ERR_A20;
}

int XMM_LocalDisableA20(REGS *regs)
{
	return ERR_A20;
}

/** IsA20On => compare two different memory location if a20 is on,
otherwise the address on bus is the same! */
int XMM_IsA20On(REGS *regs)
{
	void far *lowmem = MK_FP(0x0000, 0x0080);
	void far *highmem = MK_FP(0xFFFF, 0x0090);
	if (_fmemcmp(lowmem, highmem, 8) == 0)
		regs->x.ax = 1;
	else
		regs->x.ax = 0;
	return ERR_NOERROR;
}

static char fGlobalEnable = 0;
int XMM_GlobalEnableA20(REGS *regs)
{
	if (fGlobalEnable == 0) {
		int r = XMM_LocalEnableA20(regs);

		if (r == ERR_NOERROR) {
			fGlobalEnable = 1;
		} else {
			return r;
		}
	}

	regs->x.ax = 1;
	return ERR_NOERROR;
}

int XMM_GlobalDisableA20(REGS *regs)
{
	if (fGlobalEnable == 1) {
		int r = XMM_LocalDisableA20(regs);

		if (r == ERR_NOERROR) {
			fGlobalEnable = 0;
		} else {
			return r;
		}
	}

	regs->x.ax = 1;
	return ERR_NOERROR;
}

union SegOff
{
	struct {
		unsigned offset;
		unsigned segment;
	} addr;
	unsigned long address;
};

struct ExtMemMove
{
	unsigned long	Length;
	HANDLE			SourceHandle;
	union SegOff	SourceOffset;
	HANDLE			DestHandle;
	union SegOff	DestOffset;
};

/** AmountOfMemory */
static long AmountOfMemory = 0;
static long SizeOfLargestFreeBlock = 0;
static long AmountOfUsedMemory = 0;

int XMM_QueryExtMemory(REGS *regs)
{
	int r = ERR_OUTOFMEMORY;

	regs->x.ax = (int) SizeOfLargestFreeBlock / 1024;
	regs->x.dx = (int) (AmountOfMemory - AmountOfUsedMemory) / 1024;

	r = ERR_NOERROR;
	return r;
}

int XMM_AllocExtMemory(REGS *regs, size_t size)
{
	long mem_available = AmountOfMemory - AmountOfUsedMemory;
	long mem_required = size * 1024;

	if (mem_available < mem_required) {
		return ERR_OUTOFMEMORY;
	}

	unsigned segp;
	if (_dos_allocmem(1, &segp) != 0) { // cannot allocate handle!
		return ERR_OUTOFMEMORY;
	}

	HANDLE handle = (HANDLE)(MK_FP(segp, 0));
	_fmemset(handle, 0, sizeof(HANDLE));

	handle->TAG = XMM_TAG;

	regs->x.ax = 1;
	regs->x.dx = segp;

	return ERR_NOERROR;

}

int XMM_FreeExtMemory(REGS *regs, unsigned argdx)
{
	HANDLE handle = (HANDLE) MK_FP(argdx, 0);

	if (handle->TAG != XMM_TAG) {
		return ERR_INVALIDHANDLE;
	}

	// mark as free memory...
	_dos_freemem(argdx);	// deallocate memory...

	return ERR_NOERROR;
}

/** this operation require a switch in 32bit! */
int XMM_MoveExtMemory(REGS *regs, struct ExtMemMove *far emb)
{
	return ERR_INVALIDHANDLE;
}

int XMM_LockExtMemory(REGS *regs, unsigned argdx)
{
	HANDLE handle = (HANDLE) MK_FP(argdx, 0);

	if (handle->TAG != XMM_TAG) {
		return ERR_INVALIDHANDLE;
	}

	return ERR_NOERROR;
}

int XMM_UnlockExtMemory(REGS *regs, unsigned argdx)
{
	HANDLE handle = (HANDLE) MK_FP(argdx, 0);

	if (handle->TAG != XMM_TAG) {
		return ERR_INVALIDHANDLE;
	}

	return ERR_NOERROR;
}

int XMM_GetExtMemoryInfo(REGS *regs, unsigned argdx)
{
	HANDLE handle = (HANDLE) MK_FP(argdx, 0);

	if (handle->TAG != XMM_TAG) {
		return ERR_INVALIDHANDLE;
	}

	regs->x.ax = 1;
	regs->h.bh = 0;
	regs->h.bl = 0;
	regs->x.dx = 0;

	return ERR_NOERROR;
}

int XMM_ReallocExtMemory(REGS *regs, unsigned argdx)
{
	HANDLE handle = (HANDLE) MK_FP(argdx, 0);

	if (handle->TAG != XMM_TAG) {
		return ERR_INVALIDHANDLE;
	}

	regs->x.ax = 1;
	regs->h.bh = 0;
	regs->h.bl = 0;
	regs->x.dx = 0;

	return ERR_NOERROR;
}

/** Entry point of XMM Control called by software */
void XMM_DISPATCH(int Arg1, int Arg2, void far *arg)
{
	REGS regs;
	memset(&regs, 0, sizeof(REGS));

	int err = ERR_NOERROR;
	int FunctionId = (Arg1 & 0xFF00) >> 8;

	switch(FunctionId) {
		case 0:	/* Version */
			XMM_Version(&regs); break;
		case 1: /* RequestHMA */
			err = XMM_RequestHMA(&regs, Arg2); break;
		case 2:
			err = XMM_ReleaseHMA(&regs); break;
		case 3:
			err = XMM_GlobalEnableA20(&regs); break;
		case 4:
			err = XMM_GlobalDisableA20(&regs); break;
		case 5:
			err = XMM_LocalEnableA20(&regs); break;
		case 6:
			err = XMM_LocalDisableA20(&regs); break;
		case 7:
			err = XMM_IsA20On(&regs); break;
		case 8:
			err = XMM_QueryExtMemory(&regs); break;
		case 9:
			err = XMM_AllocExtMemory(&regs, Arg2); break;
		case 10:
			err = XMM_FreeExtMemory(&regs, Arg2); break;
		case 11:
			err = XMM_MoveExtMemory(&regs, (struct ExtMemMove *far ) arg); break;
		case 12:
			err = XMM_LockExtMemory(&regs, Arg2); break;
		case 13:
			err = XMM_UnlockExtMemory(&regs, Arg2); break;
		case 14:
			err = XMM_GetExtMemoryInfo(&regs, Arg2); break;
		case 15:
			err = XMM_ReallocExtMemory(&regs, Arg2); break;
		default:
			err = ERR_NOTIMPLEMENTED;
			break;
	}

	if (err == ERR_NOERROR) {
		_AX = regs.x.ax;
		_DX = regs.x.dx;
		_BX = regs.x.bx;
	} else {
		_AX = regs.x.ax;
		_BH = 0;
		_BL = err;
	}
}
