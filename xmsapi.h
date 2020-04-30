#ifndef __XMSAPI_H_
#define __XMSAPI_H_

typedef void* HANDLE;

extern int xms_errno;   // latest XMS error

/** XMSReady return XMS driver status => 0 ok. 1 error */
int XMSReady();

/** xms_version() return XMS driver version .*/
int xms_version();

/** allocate deallocate memory from HMA */
void far *malloc_hma(size_t size);
int free_hma(void far *ptr);

/** enable/disable a20 */
int enable_a20();
int disable_a20();

/** enable/disable a20 for direct access from software */
int l_enable_a20();
int l_disable_a20();

/** return a20 bit status */
int query_a20();

/** allocate a block of bytes... size_t is in bytes! */
HANDLE malloc_xms(size_t size);
HANDLE hugealloc_xms(size_t kbSize);

/** destroy an handle.. and free xms memory */
int free_xms(HANDLE hHandle);

/** move size bytes from source to dest at specified offset */
int memmove_xms(HANDLE hDst, HANDLE hSrc, size_t size, long offset = 0);
int write_xms(HANDLE hDst, void far *ptr, size_t size);
int read_xms(void far *ptr, HANDLE hSrc, size_t size);

/** lock a memory block => the return value is a physical address not
accessible on real mode */
void far *lock_xms(HANDLE hHandle);

/** unlock a memory block */
int unlock_xms(HANDLE hHandle);

/** return information about an handle */
size_t get_xms_info(HANDLE hHandle, int *pAvailableHandle);

int realloc_xms(HANDLE hHandle, size_t newsize);

/** alloc an upper memory segment.. size will be aligned to 16 */
void far *malloc_umb(size_t size);

/** release an upper segment of memory.. */
int free_umb(void far *ptr);


// declared in xmm386.asm
extern "C" void switch32();
#endif

