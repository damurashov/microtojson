#ifndef RKTA_MTOJSON_H
#define RKTA_MTOJSON_H

#include <stdint.h>

enum json_value_type {
	t_json_array,
	t_json_integer,
	t_json_string,
	t_json_object,
	t_json_value,
};

struct json_kv {
	char *key;
	void *value;
	enum json_value_type type;
};

struct json_array {
	void *value;
	int count;
	enum json_value_type type;
};

char* generate_json(char *out, int len, struct json_kv *kvs);
#endif
