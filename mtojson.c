/*
   SPDX-License-Identifier: BSD-2-Clause
   This file is Copyright (c) 2020 by Rene Kita
*/

#include "mtojson.h"

#include <limits.h>
#include <string.h>

static char* gen_array(char *out, struct json_array *jar);
static char* gen_boolean(char *out, _Bool *val);
static char* gen_integer(char *out, int *val);
static char* gen_object(char *out, const struct json_kv *kv);
static char* gen_string(char *out, char *val);
static char* gen_uinteger(char *out, unsigned *val);
static char* gen_value(char *out, char *val);

char* (*gen_functions[])() = {
	gen_array,
	gen_boolean,
	gen_integer,
	gen_object,
	gen_string,
	gen_uinteger,
	gen_value,
};

static size_t rem_len;
static int nested_object_depth = 0;

static void
utoa(char *dst, unsigned n)
{
	char *s = dst;

	for (unsigned m = n; m >= 10U;  m /= 10U)
		s++;
	s[1] = '\0';

	for ( ; s >= dst; s--, n /= 10)
		*s = (char)('0' + n % 10);
}

static void
itoa(char *dst, int n)
{
	unsigned u = (unsigned)n;
	char *s = dst;
	if (n < 0){
		*s++ = '-';
		u = -(unsigned)n;
	}

	utoa(s, u);
}

static int
reduce_rem_len(size_t len)
{
	if (rem_len < len)
		return 0;
	rem_len -= len;
	return 1;
}

static char*
strcpy_val(char *out, char *val, char *wrapper)
{
	size_t wlen = wrapper ? 2 : 0;
	size_t len = strlen(val);

	if (!reduce_rem_len(len + wlen))
		return NULL;
	if (wrapper)
		*out++ = *wrapper;
	memcpy(out, val, len);
	out += len;
	if (wrapper)
		*out++ = *wrapper;
	return out;
}

static char*
gen_boolean(char *out, _Bool *val)
{
	char *t = "true";
	char *f = "false";
	char *v;
	if (*val)
		v = t;
	else
		v = f;
	return strcpy_val(out, v, NULL);
}

static char*
gen_string(char *out, char *val)
{
	return strcpy_val(out, val, "\"");
}

static char*
gen_integer(char *out, int *val)
{
	#define INT_STRING_SIZE ((sizeof(int)*CHAR_BIT - 1)*28/93 + 3)
	char buf[INT_STRING_SIZE];
	itoa(buf, *val);
	return strcpy_val(out, buf, NULL);
	#undef INT_STRING_SIZE
}

static char*
gen_uinteger(char *out, unsigned *val)
{
	#define INT_STRING_SIZE ((sizeof(int)*CHAR_BIT - 1)*28/93 + 3)
	char buf[INT_STRING_SIZE];
	utoa(buf, *val);
	return strcpy_val(out, buf, NULL);
	#undef INT_STRING_SIZE
}

static char*
gen_value(char *out, char *val)
{
	return strcpy_val(out, val, NULL);
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
gen_array(char *out, struct json_array *jar)
{
	if (!reduce_rem_len(2)) // 2 -> []
		return NULL;

	*out++ = '[';
	if (jar->count == 0){
		*out++ = ']';
		return out;
	}

	_Bool is_last;
	char* (*func)() = gen_functions[jar->type];
	switch (jar->type) {
	case t_to_array: {
		struct json_array * const *val = jar->value;
		for (size_t i = 0; i < jar->count; i++){
			is_last = (i + 1 == jar->count);
			out = gen_array_type(out, val[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_boolean: {
		const _Bool *val = jar->value;
		for (size_t i = 0; i < jar->count; i++){
			is_last = (i + 1 == jar->count);
			out = gen_array_type(out, &val[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_integer: {
		const int *val = jar->value;
		for (size_t i = 0; i < jar->count; i++){
			is_last = (i + 1 == jar->count);
			out = gen_array_type(out, &val[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_object: {
		struct json_kv * const *val = jar->value;
		for (size_t i = 0; i < jar->count; i++){
			is_last = (i + 1 == jar->count);
			out = gen_array_type(out, val[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_string: {
		char * const *val = jar->value;
		for (size_t i = 0; i < jar->count; i++){
			is_last = (i + 1 == jar->count);
			out = gen_array_type(out, val[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_uinteger: {
		const unsigned *val = jar->value;
		for (size_t i = 0; i < jar->count; i++){
			is_last = (i + 1 == jar->count);
			out = gen_array_type(out, &val[i], is_last, func);
			if (!out)
				return NULL;
		}
		break;
	}

	case t_to_value: {
		const char * const *val = jar->value;
		for (size_t i = 0; i < jar->count; i++){
			is_last = (i + 1 == jar->count);
			out = gen_array_type(out, val[i], is_last, func);
			if (!out)
				return NULL;
		}
	}
	}

	*out++ = ']';
	return out;
}

static char*
gen_object(char *out, const struct json_kv *kv)
{
	size_t object_meta_len = 2; // 2 -> {}
	if (nested_object_depth == 0)
		object_meta_len = 3; // 3 -> {}\0
	nested_object_depth++;

	if (!reduce_rem_len(object_meta_len))
		goto fail;

	*out++ = '{';
	while (kv->key){
		char *key = kv->key;
		size_t len = strlen(key);
		if (!reduce_rem_len(len + 4)) // 4 -> "":_
			goto fail;

		*out++ = '"';
		memcpy(out, key, len);
		out += len;
		*out++ = '"';
		*out++ = ':';
		*out++ = ' ';

		out = gen_functions[kv->type](out, kv->value);

		if (!out)
			goto fail;

		if ((kv + 1)->key){
			if (!reduce_rem_len(2))
				goto fail;
			*out++ = ',';
			*out++ = ' ';
		}

		kv++;
	}

	*out++ = '}';
	*out = '\0';
	nested_object_depth--;
	return out;

fail:
	nested_object_depth = 0;
	return NULL;
}

size_t
generate_json(char *out, const struct json_kv *kv, size_t len)
{
	const char *start = out;

	rem_len = len;
	out = gen_object(out, kv);

	if (!out)
		return 0;

	return (size_t)(out - start);
}
