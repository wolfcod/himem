#ifndef __XMSAPI_H_
#define __XMSAPI_H_

typedef void* HANDLE;

extern int xms_errno;   // latest XMS error

/** XMSReady return XMS driver status => 0 ok. 1 error */
int XMSReady();

// declared in xmm386.asm
extern "C" void switch32();

/** XMM_Control entry routine */
void XMM_Control();

typedef enum xms_error
{
	ERR_NOERROR = 0x00,
	ERR_NOTIMPLEMENTED	= 0x80,
	ERR_VDISKFOUND = 0x81,
	ERR_A20 = 0x82,
	ERR_GENERAL = 0x8e,
	ERR_UNRECOVERABLE = 0x8f,

	ERR_HMANOTEXIST = 0x90,
	ERR_HMAINUSE = 0x91,
	ERR_HMAMINSIZE = 0x92,
	ERR_HMANOTALLOCED = 0x93,

	ERR_OUTOFMEMORY = 0xa0,
	ERR_OUTOFHANDLES = 0xa1,
	ERR_INVALIDHANDLE = 0xa2,
	ERR_SHINVALID = 0xa3,
	ERR_SOINVALID = 0xa4,
	ERR_DHINVALID = 0xa5,
	ERR_DOINVALID = 0xa6,
	ERR_LENINVALID = 0xa7,
	ERR_OVERLAP = 0xa8,
	ERR_PARITY = 0xa9,
	ERR_EMBUNLOCKED = 0xaa,
	ERR_EMBLOCKED = 0xab,
	ERR_LOCKOVERFLOW = 0xac,
	ERR_LOCKFAIL = 0xad,

	ERR_UMBSIZETOOBIG = 0xb0,
	ERR_NOUMBS = 0xb1,
	ERR_INVALIDUMB = 0xb2
};

#define XMS_VERSION 0x0200
#define HIMEM_VERSION 0x0203

#endif

