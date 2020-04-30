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

/** query xms version or return -1 */
int xms_version()
{
	if (xms_control == NULL)
		return -1;

	_AH = 0x00;
	return xms_control();
}

/** allocate deallocate memory from HMA */
void far *malloc_hma(size_t size)
{
	return NULL;
}

int free_hma(void far *ptr)
{
	return 0;
}

/** enable/disable a20 */
int enable_a20()
{
	_AH = 0x03;
	return xms_control();
}

int disable_a20()
{
	_AH = 0x04;
	return xms_control();
}

/** enable/disable a20 for direct access from software */
int l_enable_a20()
{
	_AH = 0x05;
	return xms_control();
}

int l_disable_a20()
{
	_AH = 0x06;
	return xms_control();
}

/** return a20 bit status */
int query_a20()
{
	_AH = 0x07;
	return xms_control();
}

/** allocate a block of bytes... size_t is in bytes! */
HANDLE malloc_xms(size_t size)
{
	return hugealloc_xms(size_in_kb(size));
}

HANDLE hugealloc_xms(size_t kbSize)
{
	HANDLE hHandle = (HANDLE) 0;
	_AH = 0x09;
	_DX = kbSize;
	int r = xms_control();
	if (r == 0) {
		xms_errno = _BL;
	} else {
		hHandle = (HANDLE) _DX;
		xms_errno = 0;
	}

	return hHandle;
}


/** destroy an handle.. and free xms memory */
int free_xms(HANDLE hHandle)
{
	_AH = 0x0a;
	_DX = (unsigned int) hHandle;
	int r = xms_control();

	if (r == 0) {
		xms_errno = _BL;
		return 1;
	}
	xms_errno = 0;
	return 0;
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

/** move size bytes from source to dest at specified offset */
int memmove_xms(HANDLE hDst, HANDLE hSrc, size_t size, long offset)
{
	return 0;
}

int write_xms(HANDLE hDst, void far *ptr, size_t size)
{
	struct ExtMemMove w;

	w.Length = (unsigned long) size;
	w.SourceHandle = 0;	// 0 on this..
	w.SourceOffset.addr.offset = FP_OFF(ptr);
	w.SourceOffset.addr.segment = FP_SEG(ptr);
	w.DestHandle = hDst;
	w.DestOffset.address = 0;

	// on tiny model.. DS is equal to SS .. so DS:SI can be initialized only
	// on SI

	_AH = 0x0b;
	_SI = (unsigned int) &w;
	int r = xms_control();
	if (r == 0) {
		xms_errno = _BL;
		return 1;
	}

	return 0;
}

int read_xms(void far *ptr, HANDLE hSrc, size_t size)
{
	struct ExtMemMove w;

	w.Length = (unsigned long) size;
	w.SourceHandle = hSrc;	// 0 on this..
	w.SourceOffset.address = 0;
	w.DestOffset.addr.offset = FP_OFF(ptr);
	w.DestOffset.addr.segment = FP_SEG(ptr);
	w.DestHandle = 0;

	// on tiny model.. DS is equal to SS .. so DS:SI can be initialized only
	// on SI

	_AH = 0x0b;
	_SI = (unsigned int) &w;
	int r = xms_control();
	if (r == 0) {
		xms_errno = _BL;
		return 1;
	}

	return 0;

}

/** lock a memory block => the return value is a physical address not
accessible on real mode */
void far *lock_xms(HANDLE hHandle)
{
	void far *addr = NULL;
	_AH = 0x0c;
	_DX = (int) hHandle;
	int r = xms_control();
	xms_errno = _BL;
	unsigned off = _BX;
	unsigned seg = _DX;

	addr = MK_FP(seg, off);

	if (r == 0)
		return 0;

	xms_errno = 0;
	return addr;
}

/** unlock a memory block */
int unlock_xms(HANDLE hHandle)
{
	_AH = 0x0d;
	_DX = (int) hHandle;
	int r = xms_control();

	if (r != 1) {
		xms_errno = _BL;
		return 1;
	}

	xms_errno = 0;
	return 0;
}

/** return information about an handle */
size_t get_xms_info(HANDLE hHandle, int *pAvailableHandle)
{
	size_t size = 0;
	int handles = 0;
	int blockCount = 0;
	_AH = 0x0e;
	_DX = (int) hHandle;
	int r = xms_control();

	if (r != 1) {
		xms_errno = _BL;
	} else {
		size = _DX;
		handles = _BL;
		blockCount = _BH;
	}

	if (pAvailableHandle != NULL)
		*pAvailableHandle = handles;

	return size;
}

int realloc_xms(HANDLE hHandle, size_t newsize)
{
	newsize = size_in_kb(newsize);

	_AH = 0x0f;
	_BX = newsize;
	_DX = (int) hHandle;
	int r = xms_control();
	if (r != 1)
		xms_errno = _BL;
	else
		xms_errno = 0;

	return !(r == 1);
}

/** alloc an upper memory segment.. size will be aligned to 16 */
void far *malloc_umb(size_t size)
{
	void far *r;
	size = size_in_page(size);

	asm {
		mov dx, word ptr [size]		// retrieve only segment...
		mov ax, 0x1000
		call far ptr [xms_control]
		mov word ptr [r], 0
		mov word ptr [r+2], bx
	}
	if (_AX == 1) {
		xms_errno = 0;
		return r;
	}
	else {
		xms_errno = _BL;
	}

	return 0;
}

/** release an upper segment of memory.. */
int free_umb(void far *arg)
{
	int r;
	asm {
		mov dx, word ptr [arg+2]		// retrieve only segment...
		mov ax, 0x1100
		call far ptr [xms_control]
	}
	if (_AX == 0) {
		xms_errno = _BL;
		return 1;
	}
	else
		xms_errno = 0;

	return 0;
}
