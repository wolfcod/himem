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
		return ERR_HMANOEXIST;

	if (MinHMASize < bytes)
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

int (*a20Handler)(int enable);

int XMM_LocalEnableA20(REGS *regs)
{
	return ERR_A20;
}

int XMM_LocalDisableA20(REGS *regs)
{
	return ERR_A20;
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

/** Entry point of XMM Control called by software */
void XMM_DISPATCH(int Arg1, int Arg2, void far *arg)
{
	REGS regs;
	memset(&regs, 0, sizeof(REGS));

	int err = ERR_NOERROR;
	int FunctionId = (Arg1 & 0xFF00) >> 8;

	switch(FunctionId) {
		case 0:	/* Version */
			XMM_Version(&regs);
			break;
		case 1: /* RequestHMA */
			err = XMM_RequestHMA(&regs, Arg2);
			break;
		case 2:
			err = XMM_ReleaseHMA(&regs);
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		default:
			err = ERR_NOTIMPLEMENTED;
			break;
	}

	if (err == ERR_NOERROR) {
		_AX = regs.x.ax;
		_DX = regs.x.dx;
	} else {
		_AX = regs.x.ax;
		_BL = err;
	}
}
