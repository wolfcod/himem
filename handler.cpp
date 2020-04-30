/** XMS Handler */
#include <dos.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

typedef struct _ReqHdr {
	BYTE    ReqLen;
	BYTE    Unit;
	BYTE    Command;
	WORD    Status;
	BYTE    Reserved[8];
	BYTE    Units;
	DWORD   Address;
	DWORD   pCmdLine;
} ReqHdr;

typedef struct _Handle {
	BYTE    Flags;
	BYTE    cLock;
	DWORD   Base;   /** Linear address of block */
	DWORD   Length; /** Length in byte */
} XMSHandle;

typedef union _Offset {
	struct {
		WORD    Offset;
		WORD    Segment;
	} segoff;
	unsigned long address;
} Offset;

typedef struct _MoveExtended {
	DWORD   bCount;
	WORD    SourceHandle;
	Offset  SourceOffset;
	WORD    DestHandle;
	Offset  DestOffset;
} MoveExtended;

/** global request */
ReqHdr far *pReqHdr = 0;


void interrupt (*old_2f)();

/** Strategy => Called by MS-DOS when even the driver is accessed.
ARGS: ES:BX = Address of Request Header
RETS: Nothing
REGS: Preserved
*/
extern "C" void Strategy()
{
	asm {
		mov word ptr cs:[pReqHdr], bx
		mov word ptr cs:[pReqHdr+2], es
	}
}

int InitDriver();       /** prototype of InitDriver */

/** Interrupt => Called by MS-ODS after strategy routine
ARGS: None
RETS: Return code in Request Header's Status Field
REGS: Preserved
*/
extern "C" void interrupt XMSInterrupt()
{
	BYTE cmd = pReqHdr->Command;
	WORD status = 0;

	if (cmd == 0x00)
		status = InitDriver();

	status = status | 0x0100;
	pReqHdr->Status = status;
}

extern int (far *xms_control)();

#ifdef __cplusplus
	#define __CPPARGS ...
#else
	#define __CPPARGS
#endif

void interrupt Int2F(__CPPARGS)
{
	enable();

	if (_AH == 0x43) {      // we are interested in this handler...
		if (_AL == 0x00) {      // return 0x80
			_AL = 0x80;
			return;
		}
		if (_AL == 0x10) {
			asm {
				push cs
				pop     es
				mov bx, offset xms_control
			}
			return;
		}
	}

	disable();
	asm {
		jmp dword ptr cs:[old_2f]
	}

}


/** Driver Initialization */
int InitDriver()
{
	_AX = 0x4300;
	geninterrupt(0x2f);
	if (_AL == 0x80) {

	}

	old_2f = (void (interrupt *)())_dos_getvect(0x2f);
	_dos_setvect(0x2f, Int2F);

end:

	return 0;
}
