
#include "LibcCustom.h"

const std::nothrow_t std::nothrow;

int snprintf(char *pBuf, size_t size, const char *format, ...)
{
	(void)pBuf;
	(void)size;
	(void)format;

	return 0;
}

void *operator new(size_t size, const std::nothrow_t &t)
{
	void *pData = NULL;
	(void)size;
	(void)t;
	return pData;
}

void *operator new[](size_t size, const std::nothrow_t &t)
{
	void *pData = NULL;
	(void)size;
	(void)t;
	return pData;
}

void operator delete(void *p)
{
	(void)p;
}

void operator delete[](void *p)
{
	(void)p;
}

void operator delete(void *p, size_t t)
{
	(void)p;
	(void)t;
}

void operator delete(void *p, std::nothrow_t const &t)
{
	(void)p;
	(void)t;
}

