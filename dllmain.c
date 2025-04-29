#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>

#include "BootImage.h"
#define BOOT_IMAGE_SIZE 3476752

#define DLL_EXPORT __declspec(dllexport)

BOOLEAN NewLine = FALSE;

WCHAR *String = NULL;
ULONG StringIndex = 0;

void mini_rv32ima_putchar(char c) {
	if (StringIndex == 0 && !NewLine) { memset(String, 0, 1024 * 2); }
	if (c == '\r' || c == '\n' || c == '\0') {
		StringIndex = 0;
		NewLine = TRUE;
		return;
	}
	String[StringIndex++] = (WCHAR)c;
}

void mini_rv32ima_print(char* string) {
}

int mini_rv32ima_key_hit(void) {
	return 0;
}

int mini_rv32ima_get_key(void) {
	return -1;
}

void* mini_rv32ima_malloc(size_t count) {
	return malloc(count);
}

uint32_t mini_rv32ima_handle_control_store(uint32_t addy, uint32_t val) {
	if (addy == 0x10000000) mini_rv32ima_putchar(val);

	return 0;
}

uint32_t mini_rv32ima_handle_control_load(uint32_t addy) {
	if (addy == 0x10000005) return 0x60 | mini_rv32ima_key_hit();

	return 0;
}

void mini_rv32ima_handle_other_csr_write(uint16_t csrno, uint32_t value) {
	if (csrno == 0x139) mini_rv32ima_putchar(value);
}

uint32_t mini_rv32ima_handle_other_csr_read(uint16_t csrno) {
	if (csrno == 0x140) {
		if (!mini_rv32ima_key_hit()) return -1;
		return mini_rv32ima_get_key();
	}
	return 0;
}

#define RAM_SIZE  64 * 1024 * 1024

#define MINIRV32WARN
#define MINIRV32_DECORATE  static
#define MINI_RV32_RAM_SIZE RAM_SIZE
#define MINIRV32_IMPLEMENTATION
#define MINIRV32_HANDLE_MEM_STORE_CONTROL(addy, val) if(mini_rv32ima_handle_control_store( addy, val )) return val;
#define MINIRV32_HANDLE_MEM_LOAD_CONTROL(addy, rval) rval = mini_rv32ima_handle_control_load( addy );
#define MINIRV32_OTHERCSR_WRITE(csrno, value) mini_rv32ima_handle_other_csr_write(csrno, value);
#define MINIRV32_OTHERCSR_READ(csrno, value) value = mini_rv32ima_handle_other_csr_read(csrno);

#include "mini-rv32ima.h"
#include "default64mbdtc.h"

uint8_t* memory = NULL;
struct MiniRV32IMAState* cpu = NULL;

DLL_EXPORT BOOLEAN mini_rv32ima_init(VOID) {
    	memory = mini_rv32ima_malloc(RAM_SIZE);
	memset(memory, 0, RAM_SIZE);

	String = malloc(1024 * 2);

	memcpy(memory, BootImage, BOOT_IMAGE_SIZE);

	uintptr_t dtbPtr = RAM_SIZE - sizeof(default64mbdtb) - sizeof(struct MiniRV32IMAState);
	memcpy(memory + dtbPtr, default64mbdtb, sizeof(default64mbdtb));

	cpu = (struct MiniRV32IMAState*)(memory + RAM_SIZE - sizeof(struct MiniRV32IMAState));
	cpu->pc = MINIRV32_RAM_IMAGE_OFFSET;
	cpu->regs[10] = 0x00; //hart ID
	cpu->regs[11] = (dtbPtr + MINIRV32_RAM_IMAGE_OFFSET); //dtb_pa (Must be valid pointer) (Should be pointer to dtb)
	cpu->extraflags |= 3; // Machine-mode.

	uint32_t* dtb = (uint32_t*)(memory + dtbPtr);
	if (dtb[0x13c / 4] == 0x00c0ff03) {
		uint32_t validram = dtbPtr;
		dtb[0x13c / 4] = (validram >> 24) | (((validram >> 16) & 0xff) << 8) | (((validram >> 8) & 0xff) << 16) | ((validram & 0xff) << 24);
	}
    	return TRUE;
}

DLL_EXPORT INT mini_rv32ima_step(INT count) {
    return  MiniRV32IMAStep(cpu, memory, 0, 1, count);
}

DLL_EXPORT WCHAR *mini_rv32ima_get_last_line(VOID) {
	if (!NewLine) { return 0; }
	else {
		SIZE_T length = wcslen(String) + 1;
		WCHAR *ret = malloc(length * 2);
		memcpy(ret, String, length * 2);
		NewLine = FALSE;
		StringIndex = 0;
		return ret;
	}
}
