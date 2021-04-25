#ifndef SIMD_DATA_OBJECT_H
#define SIMD_DATA_OBJECT_H

#include "core_simd_info.h"

struct pysimd_data_obj {
	size_t alignment;
	size_t size;
	uint8_t* data;
};

void pysimd_data_obj_init(pysimd_data_obj* obj, size_t align, size_t size, void* data)
{
	obj->alignment = align;
	obj->size = size;
	obj->data = malloc(size);
	// copying todo
}

#endif // SIMD_DATA_OBJECT_H