
#ifndef LIBC_CUSTOM_H
#define LIBC_CUSTOM_H

namespace std
{
	struct nothrow_t {};
	extern const nothrow_t nothrow;
}

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;
typedef unsigned int size_t;

#define NULL	nullptr

int snprintf(char *pBuf, size_t size, const char *format, ...);

void *operator new(size_t size, const std::nothrow_t &t);
void *operator new[](size_t size, const std::nothrow_t &t);
void operator delete(void *p);
void operator delete[](void *p);
void operator delete(void *p, size_t t);
void operator delete(void *p, std::nothrow_t const &t);

#endif

