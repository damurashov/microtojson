/*
   SPDX-License-Identifier: BSD-2-Clause
   This file is Copyright (c) 2020 by Rene Kita
*/

#include "mtojson.h"

#include <string.h>

static char* gen_boolean(char *, const void *);
static char* gen_c_array(char *, const void *);
static char* gen_integer(char *, const void *);
static char* gen_object(char *, const void *);
static char* gen_string(char *, const void *);
static char* gen_uinteger(char *, const void *);
static char* gen_value(char *, const void *);

static char* (* const gen_functions[])(char *, const void *) = {
	gen_c_array,
	gen_boolean,
	gen_integer,
	gen_object,
	gen_string,
	gen_uinteger,
	gen_value,
};

static size_t remaining_length;

static int
reduce_rem_len(size_t len)
{
	if (remaining_length < len)
		return 0;
	remaining_length -= len;
	return 1;
}

static char*
strcpy_val(char *out, const char *val, size_t len)
{
	if (!reduce_rem_len(len))
		return NULL;
	memcpy(out, val, len);
	return out + len;
}

static char*
gen_boolean(char *out, const void *val)
{
	if (*(_Bool*)val)
		return strcpy_val(out, "true", 4);
	else
		return strcpy_val(out, "false", 5);
}

static char*
gen_string(char *out, const void *val)
{
	const char *v = (const char*)val;
	if (!reduce_rem_len(2)) // 2 -> ""
		return NULL;
	*out++ = '"';
	if (!(out = strcpy_val(out, v, strlen(v))))
		return NULL;
	*out++ = '"';
	return out;
}

static char*
gen_integer(char *out, const void *val)
{
	int n = *(int*)val;
	unsigned u = (unsigned)n;
	if (n < 0){
		if (!reduce_rem_len(1))
			return NULL;
		*out++ = '-';
		u = -(unsigned)n;
	}

	if (!(out = gen_uinteger(out, &u)))
		return NULL;
	return out;
}

static char*
gen_uinteger(char *out, const void *val)
{
	char *s = out;
	char *r;
	unsigned n = *(unsigned*)val;

	for (unsigned m = n; m >= 10U;  m /= 10U)
		s++;
	r = s + 1;

	size_t len = (size_t)(r - out);
	if (!reduce_rem_len(len))
		return NULL;

	for ( ; s >= out; s--, n /= 10)
		*s = '0' + (char)(n % 10);

	return r;
}

static char*
gen_value(char *out, const void *val)
{
	return strcpy_val(out, (const char*)val, strlen((const char*)val));
}

static char*
gen_array_type(char *out, const void *val, _Bool is_last, char* (*func)())
{
	out = (*func)(out, val);
	if (!out)
		return NULL;
	if (!is_last){
		if (!reduce_rem_len(2))
			return NULL;
		*out++ = ',';
		*out++ = ' ';
	}
	return out;
}

static char*
gen_c_array(char *out, const void *val)
{
	const struct to_json *tjs = (const struct to_json*)val;
	if (!reduce_rem_len(2)) // 2 -> []
		return NULL;

	*out++ = '[';
	if (tjs->count == 0){
		*out++ = ']';
		return out;
	}

	_Bool is_last;
	char* (*func)() = gen_functions[tjs->vtype];
	switch (tjs->vtype) {
	case t_to_array: {
		struct json_array * const *v = tjs->value;
		for (size_t i = 0; i < tjs->count; i++){
			is_last = (i + 1 == tjs->count);
			out = gen_array_type(out, v[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_boolean: {
		const _Bool *v = tjs->value;
		for (size_t i = 0; i < tjs->count; i++){
			is_last = (i + 1 == tjs->count);
			out = gen_array_type(out, &v[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_integer: {
		const int *v = tjs->value;
		for (size_t i = 0; i < tjs->count; i++){
			is_last = (i + 1 == tjs->count);
			out = gen_array_type(out, &v[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_object: {
		struct json_kv * const *v = tjs->value;
		for (size_t i = 0; i < tjs->count; i++){
			is_last = (i + 1 == tjs->count);
			out = gen_array_type(out, v[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_string: {
		char * const *v = tjs->value;
		for (size_t i = 0; i < tjs->count; i++){
			is_last = (i + 1 == tjs->count);
			out = gen_array_type(out, v[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_uinteger: {
		const unsigned *v = tjs->value;
		for (size_t i = 0; i < tjs->count; i++){
			is_last = (i + 1 == tjs->count);
			out = gen_array_type(out, &v[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_value: {
		const char * const *v = tjs->value;
		for (size_t i = 0; i < tjs->count; i++){
			is_last = (i + 1 == tjs->count);
			out = gen_array_type(out, v[i], is_last, func);
			if (!out)
				return NULL;
		}
	}
	}

	*out++ = ']';
	return out;
}

static char*
gen_object(char *out, const void *val)
{
	const struct to_json *tjs = (const struct to_json*)val;

	if (!reduce_rem_len(2)) // 2 -> {}
		return NULL;

	*out++ = '{';
	while (tjs->name){
		const char *name = tjs->name;
		size_t len = strlen(name);
		if (!reduce_rem_len(len + 4)) // 4 -> "":_
			return NULL;

		*out++ = '"';
		memcpy(out, name, len);
		out += len;
		*out++ = '"';
		*out++ = ':';
		*out++ = ' ';

		out = gen_functions[tjs->vtype](out, tjs->value);

		if (!out)
			return NULL;

		tjs++;
		if (tjs->name){
			if (!reduce_rem_len(2))
				return NULL;
			*out++ = ',';
			*out++ = ' ';
		}
	}

	*out++ = '}';
	return out;
}

size_t
json_generate(char *out, const struct to_json *tjs, size_t len)
{
	const char *start = out;

	remaining_length = len;
	if (!reduce_rem_len(1)) // \0
		return 0;

	switch (tjs->stype) {
	case t_to_array:
		out = gen_c_array(out, tjs);
		break;
	case t_to_object:
		out = gen_object(out, tjs);
		break;
	default:
		return 0;
	}

	if (!out)
		return 0;

	*out = '\0';
	return (size_t)(out - start);
}
