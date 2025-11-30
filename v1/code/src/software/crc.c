#include "crc.h"

uint32_t crc32(void * ptr, int size)
{
    uint8_t * data = ptr;

    uint32_t r = ~0; 
    const uint8_t * end = data + size;

    while (data < end)
    {
        r ^= *data++;

        for (uint32_t i = 0; i < 8; i++)
        {
            uint32_t t = ~((r & 1) - 1); 
            r = (r >> 1) ^ (0xEDB88320 & t);
        }
    }
 
  return ~r;
}