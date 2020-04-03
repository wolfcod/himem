#include <stdio.h>
#include <dos.h>
#include "cpuid.h"

int isXMSReady();

int main()
{
	char *cpu[] = { "8086/8088", "80186", "80286", "80386 or higher" };

	printf("HIMEM.EXE TSR\n");
	printf("COVID #sideeffect written by cod\n");

	printf("CPU detect = %s\n", cpu[cpu_detect()]);

	printf("XMS ready? %s\n", (isXMSReady() == 0) ? "Yes" : "No");
	return 0;
}
