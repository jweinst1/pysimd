#ifndef SIMD_DATA_OBJECT_H
#define SIMD_DATA_OBJECT_H

#include "core_simd_info.h"

struct pysimd_data_obj {
	size_t alignment;
	size_t len;
	size_t cap;
	uint8_t* data;
};

void pysimd_data_obj_init(pysimd_data_obj* obj, size_t align, size_t capac)
{
	obj->alignment = align;
	obj->cap = capac;
	obj->len = 0;
	obj->data = calloc(1, capac);
}

#endif // SIMD_DATA_OBJECT_H