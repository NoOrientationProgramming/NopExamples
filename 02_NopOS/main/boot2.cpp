
#include "types.h"
#include "Processing.h"

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
	(void)numMagic;

	char buff[257];
	char *pBuf = buff;
	char *pBufEnd = pBuf + sizeof(buff);

	ctorsExec();

	printf("NopOS - Hello!");

	dInfo("foo: %s", "test");

	if (bootMulti)
		dInfo("bar: %s", "test");

	while(1);
}

