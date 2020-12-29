/*
   SPDX-License-Identifier: BSD-2-Clause
   This file is Copyright (c) 2020 by Rene Kita
*/

#include "mtojson.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static size_t rem_len;
static int nested_object_depth = 0;

static int
reduce_rem_len(size_t len)
{
	if (rem_len < len)
		return 0;
	rem_len -= len;
	return 1;
}

static char*
gen_boolean(char *out, _Bool val)
{
	char *t = "true";
	char *f = "false";
	char *v;
	if (val)
		v = t;
	else
		v = f;
	size_t len = strlen(v);
	if (!reduce_rem_len(len))
		return NULL;
	while ((*out++ = *v++));
	--out; // Discard \0
	return out;
}

static char*
gen_string(char *out, char *val)
{
	size_t len = strlen(val);
	if (!reduce_rem_len(len + 2)) // ""
		return NULL;
	*out++ = '"';
	while ((*out++ = *val++));
	--out; // Discard \0
	*out++ = '"';
	return out;
}

static char*
gen_integer(char *out, int val)
{
	#define INT_STRING_SIZE ((sizeof(int)*CHAR_BIT - 1)*28/93 + 3)
	char buf[INT_STRING_SIZE];
	sprintf(buf, "%d", val);
	size_t len = strlen(buf);
	if (!reduce_rem_len(len + 0))
		return NULL;
	char *ptr = &buf[0];
	while ((*out++ = *ptr++));
	--out; // Discard \0
	return out;
	#undef INT_STRING_SIZE
}

static char*
gen_value(char *out, char *val)
{
	size_t len = strlen(val);
	if (!reduce_rem_len(len + 0))
		return NULL;
	while ((*out++ = *val++));
	--out; // Discard \0
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

	if (jar->type == t_to_array){
		struct json_array **val = (struct json_array**)jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_array(out, val[i]);
			if (!out)
				return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}

	else if (jar->type == t_to_boolean){
		_Bool *val = jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_boolean(out, val[i]);
			if (!out)
				return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}

	else if (jar->type == t_to_integer){
		int *val = jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_integer(out, val[i]);
			if (!out)
				return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}

	else if (jar->type == t_to_object){
		struct json_kv **val = (struct json_kv**)jar->value;
		for (int i = 0; i < jar->count; i++){
			out = generate_json(out, val[i], rem_len);
			if (!out)
				return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}

	else if (jar->type == t_to_string){
		char **val = (char**)jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_string(out, val[i]);
			if (!out)
				return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}

	// Remove the ', '
	rem_len += 2;
	--out;
	*--out = ']';
	return ++out;
}

char*
generate_json(char *out, struct json_kv *kv, size_t len)
{
	rem_len = len;
	size_t object_meta_len = 2; // 2 -> {}
	if (nested_object_depth == 0)
		object_meta_len = 3; // 3 -> {}\0
	nested_object_depth++;

	if (!reduce_rem_len(object_meta_len))
		goto fail;

	*out++ = '{';
	if (!kv->key)
		goto done;

	while (kv->key){
		char *key = kv->key;
		size_t l = strlen(key);
		if (!reduce_rem_len(l + 4)) // 4 -> "":_
			goto fail;

		*out++ = '"';
		while ((*out++ = *key++));
		--out; // Discard \0
		*out++ = '"';
		*out++ = ':';
		*out++ = ' ';

		switch (kv->type){
		case t_to_array:
			out = gen_array(out, (struct json_array*)kv->value);
			break;
		case t_to_boolean:
			out = gen_boolean(out, *(_Bool*)kv->value);
			break;
		case t_to_integer:
			out = gen_integer(out, *(int*)kv->value);
			break;
		case t_to_object:
			out = generate_json(out, (struct json_kv*)kv->value, rem_len);
			break;
		case t_to_string:
			out = gen_string(out, (char*)kv->value);
			break;
		case t_to_value:
			out = gen_value(out, (char*)kv->value);
			break;
		}

		if (!out)
			goto fail;

		if ((kv + 1)->key){
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}

		kv++;
	}

done:
	*out++ = '}';
	*out = '\0';
	nested_object_depth--;
	return out;

fail:
	nested_object_depth = 0;
	return NULL;
}
