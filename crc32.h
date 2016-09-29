//header for crc32 borrowed from http://www.hackersdelight.org/hdcodetxt/crc.c.txt

#include <sys/param.h>
#include <stdint.h>

uint32_t crc32(uint32_t crc, const void *buf, size_t size);
