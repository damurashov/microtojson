# microtojson

[![builds.sr.ht status](https://builds.sr.ht/~rkta/microtojson.svg)](https://builds.sr.ht/~rkta/microtojson?)

A simple embedded friendly JSON generator.

**No heap usage, no malloc().** As minimal stack usage as possible.

Supports serialization to objects, arrays and primitives from int, unsigned int, \_Bool and char arrays.

This is still beta!

---

## Usage
### A simple example
```
#include "mtojson.h"

/* Some dummy functions to get and send some data */
size_t receive_data(int*);
void send_data(char *, size_t);

int
main()
{
	size_t len;
	int data[100];

	enum { MAX_STRING_LEN = 1000 };
	char json_text[MAX_STRING_LEN];
	size_t json_len;

	/* Define the JSON object */
	const struct to_json json[] = {
		{ .name = "received_data",
			.value = data,
			.vtype = t_to_integer,
			.count = &len,
			.stype = t_to_object
		},
		{ NULL }
	};

	while (1) {
		len = receive_data(data);
		json_len = json_generate(json_text, json, len);
		send_data(json_text, json_len);
	}
}
```

See `example.c` and `test_mtojson.c` for more usage examples.
Use `make example` to build all four variations of `example.c`.

`json_generate()` returns the length of the generated JSON text or 0 in case of an error.

To create an arbitrary value use `t_to_value` and pass the correctly formatted value as char array.

NULL pointers passed as value are converted to literal 'null'.

Make sure struct arrays are NULL terminated!

---

## About types

### Describing a single primitive type:
` const struct to_json tjs = { .value = &val, .vtype = t_to_integer };`
This is the simplest case, all other members are either not needed or initialized to the correct default value.

### Describing a structured type:
```
const struct to_json tjs[] = {
  { .value = &val, .vtype = t_to_integer, .stype = t_to_array },
  { NULL }
};

const struct to_json tjs[] = {
  { .name = "name", .value = &val, .vtype = t_to_integer, .stype = t_to_object },
  { NULL }
};
```
An structured type can have multiple members, so we need to create an array of structs here.
Arrays of structs MUST be terminated with a NULL member.
Additionally we need to set `.stype` to tell what type of structured type we want to create.

### Dealing with C arrays:
Probably the most common case when creating an array is using values from an C array.
There is a shortcut to make this more convenient.
Instead of creating an explicit array type, use a primitive and provide a count:
` const struct to_json tjs = { .value = int_arr, .count = 2, .vtype = t_to_integer };`

---

## Deviations from RFC8259 / TODOs:

- No explicit UTF-8 support, everything is a char.
- No automatic escaping of all control characters (U+0000 through U+001F).
- No support for floating point values and scientific notation.

---

## Contributing

This project is mainly developed on [sourcehut](https://sr.ht/~rkta/microtojson/).
It is mirrored on [gitlab](https://gitlab.com/rkta/microtojson) and [github](https://github.com/rkta/microtojson).

If you want to contribute or find any bugs, feel free to choose the way that best fits your work style.

---

This project is inspired by [microjson](https://gitlab.com/esr/microjson/) from Eric S. Raymond.
