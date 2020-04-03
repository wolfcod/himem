#include <stdio.h>
#include <dos.h>

static int (far *xms_control)() = NULL;
static int xms_errno = 0;

/** through interrupt 2f query XMS service (service 43h)*/
int isXMSReady()
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

int malloc_hma();
int free_hma();

int enable_a20();
int disable_a20();

int l_enable_a20();
int l_disable_a20();

int query_a20();

int query_xms();
int malloc_xms();
int free_xms();

int move_xms();
int lock_xms();
int unlock_xms();

int get_xms_info();

int realloc_xms()
{
}

void far *malloc_umb(size_t size)
{
	void far *r;
	if ((size % 16) != 0)
		size = (size / 16) + 1;
	else
		size = size / 16;

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
