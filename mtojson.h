/*
   SPDX-License-Identifier: BSD-2-Clause
   This file is Copyright (c) 2020, 2021 by Rene Kita
*/

#ifndef RKTA_MTOJSON_H
#define RKTA_MTOJSON_H

#include <stddef.h>

enum json_to_type {
	t_to_primitive,
	t_to_array,
	t_to_boolean,
	t_to_hex,
	t_to_int,
	t_to_null,
	t_to_object,
	t_to_string,
	t_to_uint,
	t_to_value,
};

struct to_json {
	const char *name;
	const void *value;
	const size_t *count;     // Number of elements in a C array
	enum json_to_type stype; // Type of the struct
	enum json_to_type vtype; // Type of '.value'
};

/* Returns the length of the generated JSON text or 0 in case of an error. */
size_t json_generate(char *out, const struct to_json *tjs, size_t len);
#endif
