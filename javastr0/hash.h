#pragma once
#include <stdint.h>

uint32_t hash(uint8_t *s)
{
    uint8_t *p = s;
    uint32_t h = 0;
    while (*p)
        h = 31*h + (uint32_t)(*(p++));
    return h;
}
