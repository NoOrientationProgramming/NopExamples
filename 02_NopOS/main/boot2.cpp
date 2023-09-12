
#include "types.h"

void printf(const char *pStr)
{
	if (!pStr)
		return;

	uint16_t *pVideo = (uint16_t *)0xb8000;

	for (; *pStr; ++pStr, ++pVideo)
		*pVideo = (*pVideo & 0xFF00) | *pStr;
}

typedef void (*pFctCtor)();

extern "C" pFctCtor ctors_start;
extern "C" pFctCtor ctors_end;

extern "C" void ctorsExec()
{
	pFctCtor *pCtor = &ctors_start;

	while (pCtor != &ctors_end)
		(*pCtor++)();
}

extern "C" void kernelMain(void *bootMulti, unsigned int numMagic)
{
	(void)bootMulti;
	(void)numMagic;

	ctorsExec();

	printf("NopOS - Hello!");

	while(1);
}

