#include <stdio.h>

int cpu_detect()
{
	int _cpu_type = 0;	// assume is 8086

	asm {
		pushf
		pop	ax
		mov cx, ax
		and cx, 0x0fff
		push ax
		popf
		pushf
		pop ax
		and ax, 0x0f000
		cmp ax, 0x0f000
		mov word ptr [_cpu_type], 0x00
		je end_cpu_type
		or cx, 0x0f000
		push cx
		popf
		pushf
		pop ax
		and ax, 0x0f000
		mov word ptr [_cpu_type], 0x02
		jz end_cpu_type
		mov word ptr [_cpu_type], 0x03
	}
end_cpu_type:

	return _cpu_type;
}

int main()
{
	char *cpu[] = { "8086/8088", "80186", "80286", "80386 or higher" };

	printf("HIMEM.EXE TSR\n");
	printf("COVID #sideeffect written by cod\n");

	printf("CPU detect = %s\n", cpu[cpu_detect()]);
	return 0;
}
