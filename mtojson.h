#ifndef RKTA_MTOJSON_H
#define RKTA_MTOJSON_H

#include <stdint.h>
#include <stddef.h>

enum json_value_type {
	t_to_array,
	t_to_boolean,
	t_to_integer,
	t_to_object,
	t_to_string,
	t_to_value,
};

struct json_kv {
	char *key;
	void *value;
	enum json_value_type type;
};

struct json_array {
	void *value;
	size_t count;
	enum json_value_type type;
};

char* generate_json(char *out, struct json_kv *kv, size_t len);
#endif
