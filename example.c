#include "mtojson.h"

#include <stdio.h>

volatile char *rp;

int
main(void)
{
	int data[] = { 0, 1, 2, 3 };
	(void)data; /* Suppress a cppcheck warning */

	enum { MAX_STRING_LEN = 1000 };
	char json_text[MAX_STRING_LEN];
	size_t json_len = 0;
	rp = json_text;

#ifdef OBJECT
	size_t len = sizeof(data)/sizeof(data[0]);
	/* Define the JSON object */
	const struct to_json json[] = {
		{ .name = "received_data",
			.value = data,
			.vtype = t_to_int,
			.count = &len,
			.stype = t_to_object
		},
		{ NULL }
	};
	json_len = json_generate(json_text, json, MAX_STRING_LEN);
#endif

#ifdef ARRAY
	/* Define the JSON array */
	const struct to_json json[] = {
		{ .value = &data[0], .vtype = t_to_int, .stype = t_to_array, },
		{ .value = &data[1], .vtype = t_to_int, },
		{ .value = &data[2], .vtype = t_to_int, },
		{ .value = &data[3], .vtype = t_to_int, },
		{ NULL }
	};
	json_len = json_generate(json_text, json, MAX_STRING_LEN);
#endif

#ifdef C_ARRAY
	size_t len = sizeof(data)/sizeof(data[0]);
	/* Define the JSON array */
	const struct to_json json = {
		.value = data, .vtype = t_to_int, .count = &len,
	};
	json_len = json_generate(json_text, &json, MAX_STRING_LEN);
#endif

#ifdef PRIMITIVE
	/* Define the JSON primitive */
	const struct to_json json = {
		.value = &data[0], .vtype = t_to_int,
	};
	json_len = json_generate(json_text, &json, MAX_STRING_LEN);
#endif

	printf("Length of the generated JSON text is %lu\n",
			(unsigned long)json_len);
	printf("Generated JSON text: %s\n", json_text);
}
