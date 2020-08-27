/*
   SPDX-License-Identifier: BSD-2-Clause
   This file is Copyright (c) 2020 by Rene Kita
*/

#include "mtojson.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int rem_len;
static int nested_object_depth = 0;

static char*
gen_json_boolean(char *out, _Bool val)
{
	char *t = "true";
	char *f = "false";
	char *v;
	if (val)
		v = t;
	else
		v = f;
	rem_len -= strlen(v);
	if (rem_len < 0) return NULL;
	while ((*out++ = *v++));
	--out; // Discard \0
	return out;
}

static char*
gen_json_string(char *out, char *val)
{
	rem_len -= (strlen(val) + 2); // ""
	if (rem_len < 0)
		return NULL;
	*out++ = '"';
	while ((*out++ = *val++));
	--out; // Discard \0
	*out++ = '"';
	return out;
}

static char*
gen_json_integer(char *out, int val)
{
	#define INT_STRING_SIZE ((sizeof(int)*CHAR_BIT - 1)*28/93 + 3)
	char buf[INT_STRING_SIZE];
	sprintf(buf, "%d", val);
	rem_len -= strlen(buf);
	if (rem_len < 0)
		return NULL;
	char *ptr = &buf[0];
	while ((*out++ = *ptr++));
	--out; // Discard \0
	return out;
	#undef INT_STRING_SIZE
}

static char*
gen_json_value(char *out, char *val)
{
	rem_len -= strlen(val);
	if (rem_len < 0)
		return NULL;
	while ((*out++ = *val++));
	--out; // Discard \0
	return out;
}

static char*
gen_json_array(char *out, struct json_array *jar)
{
	if ((rem_len -= 2) < 0) return NULL; // 2 -> []
	*out++ = '[';
	if (jar->count == 0){
		*out++ = ']';
		return out;
	}
	if (jar->type == t_json_array){
		struct json_array **val = (struct json_array**)jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_json_array(out, val[i]);
			if (!out) return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}
	else if (jar->type == t_json_boolean){
		_Bool *val = jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_json_boolean(out, val[i]);
			if (!out) return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}
	else if (jar->type == t_json_integer){
		int *val = jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_json_integer(out, val[i]);
			if (!out) return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}
	else if (jar->type == t_json_object){
		struct json_kv **val = (struct json_kv**)jar->value;
		for (int i = 0; i < jar->count; i++){
			out = generate_json(out, rem_len, val[i]);
			if (!out) return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}
	else if (jar->type == t_json_string){
		char **val = (char**)jar->value;
		for (int i = 0; i < jar->count; i++){
			out = gen_json_string(out, val[i]);
			if (!out) return NULL;
			rem_len -= 2;
			*out++ = ',';
			*out++ = ' ';
		}
	}
	rem_len += 2;
	--out;
	*--out = ']';
	return ++out;
}

char*
generate_json(char *out, int len, struct json_kv *kv)
{
	int object_meta_len = 2; // 2 -> {}
	if (nested_object_depth == 0)
		object_meta_len = 3; // 3 -> {}\0
	nested_object_depth++;
	if((rem_len = len - object_meta_len) < 0) return NULL;
	*out++ = '{';
	if (!kv->key){
		*out++ = '}';
		*out++ = '\0';
		return out;
	}

	while (kv->key){
		char *key = kv->key;
		rem_len -= (strlen(key) + 4); // 4 -> "":_
		if (rem_len < 0) return NULL;

		*out++ = '"';
		while ((*out++ = *key++));
		--out; // Discard \0
		*out++ = '"';
		*out++ = ':';
		*out++ = ' ';

		switch (kv->type){
		case t_json_array:
			out = gen_json_array(out, (struct json_array*)kv->value);
			break;
		case t_json_boolean:
			out = gen_json_boolean(out, *(_Bool*)kv->value);
			break;
		case t_json_integer:
			out = gen_json_integer(out, *(int*)kv->value);
			break;
		case t_json_object:
			out = generate_json(out, len, (struct json_kv*)kv->value);
			break;
		case t_json_string:
			out = gen_json_string(out, (char*)kv->value);
			break;
		case t_json_value:
			out = gen_json_value(out, (char*)kv->value);
			break;
		}

		if (!out) return NULL;
		rem_len -= 2;
		*out++ = ',';
		*out++ = ' ';
		kv++;
	}

	rem_len += 2;
	--out;
	*--out = '}';
	*++out = '\0';
	nested_object_depth--;
	return out;
}
