/*
 * Tests for microtojson.h
 *
 * If json_generate DOES NOT detect an EXPECTED buffer overflow, running tests
 * will be aborted immediately with an exit status 125.
 *
 * If json_generate DOES detect an NON-EXPECTED buffer overflow, running tests
 * will be aborted immediately with an exit status 124.
 *
 * If json_generate returns the wrong string length, running tests will be
 * aborted immediately with an exit status 123.
 *
 * If tests fail exit status is the count of failed tests. All succeeding tests
 * will be run and the number of the failed tests will be printed to stderr.
 *
 * Every test is run twice: the first time a buffer overflow is provoked - the
 * test must fail!
 */

/*
 * SPDX-License-Identifier: BSD-2-Clause
 * This file is Copyright (c) 2020, 2021 by Rene Kita
 */

#include "mtojson.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { MAXLEN = 512 };

_Bool single_test = 0;
int verbose = 0;

// Helper pointer, use 'display rp' inside gdb to see the string grow
const char *rp;

static void tell_single_test(char *);

static int
run_test(char *test, char *expected, char *result, const struct to_json *tjs, size_t len)
{
	int err = 0;

	tell_single_test(test);

	memset(result, '\0', len);
	size_t l = json_generate(result, tjs, len);
	if (!l) {
		if (verbose) {
			printf("%s\n", "NON-EXPECTED buffer overflow");
			result[len - 1] = '\0';
			fprintf(stderr, "Expected : %s\n", expected);
			fprintf(stderr, "Generated: %s\n", result);
		}
		exit(124);
	}

	if (l != strlen(result)) {
		if (verbose)
			printf("%s\n", "String length mismatch");
		exit(123);
	}

	err = (strcmp(result, expected) != 0);
	if (err){
		fprintf(stderr, "\nFAILED: %s\n", test);
		fprintf(stderr, "Expected : %s\n", expected);
		fprintf(stderr, "Generated: %s\n", result);
		return err;
	}

	memset(result, '\0', len);
	if (len >= 10 && json_generate(result, tjs, len - 10))
		err += 2;
	memset(result, '\0', len);
	if (json_generate(result, tjs, len / 2))
		err += 4;
	memset(result, '\0', len);
	if (json_generate(result, tjs, len - 1))
		err += 8;
	if (err > 1) {
		if (verbose)
			printf("%s, %d\n", "UNDETECTED buffer overflow", err);
		exit(125);
	}

	return err;
}

static void
tell_single_test(char* test)
{
	if (single_test || verbose){
		printf("Running test: %-35s ", test);
		if (!verbose)
			printf("%s", "\n");
	}
}

static int
test_object_string(void)
{
	char *expected = "{\"name\":\"value\"}";
	char *test = "test_object_string";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const struct to_json tjs[] = {
		{ .name = "name", .value = "value", .vtype = t_to_string, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_boolean(void)
{
	char *expected = "{\"name\":true}";
	char *test = "test_object_boolean";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const _Bool value = true;
	const struct to_json tjs[] = {
		{ .name = "name", .value = &value, .vtype = t_to_boolean, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_c_array_boolean(void)
{
	char *expected = "[true,false]";
	char *test = "test_c_array_boolean";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const _Bool arr[] = {true, false};
	const size_t cnt = sizeof(arr) / sizeof(arr[0]);
	const struct to_json tjs = { .value = arr, .count = &cnt, .vtype = t_to_boolean, };
	return run_test(test, expected, result, &tjs, len);
}

static int
test_object_array_c_array(void)
{
	char *expected = "{\"array\":[[1,2,3],[1,2,3]]}";
	char *test = "test_object_array_c_array";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const int arr[] = {1, 2, 3};
	const size_t cnt = sizeof(arr) / sizeof(arr[0]);
	const struct to_json jar[] = {
		{ .value = arr, .count = &cnt, .vtype = t_to_int, .stype = t_to_array, },
		{ .value = arr, .count = &cnt, .vtype = t_to_int, },
		{ NULL }
		};

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_c_array_empty(void)
{
	char *expected = "[]";
	char *test = "test_c_array_empty";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const char *arr[1];
	const size_t cnt = 0;
	const struct to_json tjs = { .value = arr, .count = &cnt, .vtype = t_to_array, };
	return run_test(test, expected, result, &tjs, len);
}

static int
test_object_array_one_empty(void)
{
	char *expected = "{\"array\":[[],[1,2,3]]}";
	char *test = "test_object_array_one_empty";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const int arr[] = {1, 2, 3};
	const size_t cnt = sizeof(arr) / sizeof(arr[0]);
	const size_t zero = 0;
	const struct to_json jar[] = {
		{ .value = arr, .count = &zero, .vtype = t_to_int, .stype = t_to_array, },
		{ .value = arr, .count = &cnt, .vtype = t_to_int, },
		{ NULL }
	};

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_empty(void)
{
	char *expected = "{}";
	char *test = "test_object_empty";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	struct to_json tjs[] = {
		{ .stype = t_to_object },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_object_null(void)
{
	char *expected = "{\"name\":null}";
	char *test = "test_object_object_null";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	struct to_json tjs[] = {
		{ .name = "name", .value = NULL, .vtype = t_to_object, .stype = t_to_object, },
		{ NULL }
	}
;
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_object(void)
{
	char *expected = "{"
	                   "\"names\":{"
	                           "\"name_id\":1,"
	                           "\"count\":3,"
	                           "\"values\":[\"DEADBEEF\",\"1337BEEF\",\"0000BEEF\"]"
	                   "},"
	                   "\"number_of_names\":1"
	                 "}";

	char *test = "test_object_object";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	const size_t cnt = sizeof(addresses) / sizeof(addresses[0]);
	const int nid = 1;
	const struct to_json addr_arr[] = {
		{ .value = addresses[0], .vtype = t_to_string, .stype = t_to_array, },
		{ .value = addresses[1], .vtype = t_to_string, .stype = t_to_array, },
		{ .value = addresses[2], .vtype = t_to_string, .stype = t_to_array, },
		{ NULL }
	};

	const struct to_json names[] = {
		{ .name = "name_id", .value = &nid, .vtype = t_to_int, .stype = t_to_object, },
		{ .name = "count",  .value = &cnt, .vtype = t_to_int, },
		{ .name = "values", .value = addr_arr, .vtype = t_to_array, },
		{ NULL }
	};

	const int non = 1;
	const struct to_json tjs[] = {
		{ .name = "names",           .value = &names, .vtype = t_to_object, .stype = t_to_object, },
		{ .name = "number_of_names", .value = &non,   .vtype = t_to_int, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_object_object(void)
{
	char *expected = "{"
	                   "\"outer\":{"
	                            "\"middle\":{"
	                                          "\"inner\":true"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_object_object_object";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const _Bool value = true;

	const struct to_json inner[] = {
		{ .name = "inner", .value = &value, .vtype = t_to_boolean, .stype = t_to_object, },
		{ NULL }
	};

	const struct to_json middle[] = {
		{ .name = "middle", .value = &inner, .vtype = t_to_object, .stype = t_to_object, },
		{ NULL }
	};

	const struct to_json tjs[] = {
		{ .name = "outer", .value = &middle, .vtype = t_to_object, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_object_nested_empty(void)
{
	char *expected = "{"
	                   "\"outer\":{"
	                            "\"middle\":{"
	                                          "\"inner\":{}"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_object_object_nested_empty";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const struct to_json value[] = {
		{ .stype = t_to_object, },
	};

	const struct to_json inner[] = {
		{ .name = "inner", .value = &value, .vtype = t_to_object, .stype = t_to_object, },
		{ NULL }
	};

	const struct to_json middle[] = {
		{ .name = "middle", .value = &inner, .vtype = t_to_object, .stype = t_to_object, },
		{ NULL }
	};

	const struct to_json tjs[] = {
		{ .name = "outer", .value = &middle, .vtype = t_to_object, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_valuetype(void)
{
	char *expected = "{\"name\":This is not valid {}JSON!}";
	char *test = "test_object_valuetype";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const struct to_json tjs[] = {
		{ .name = "name", .value = "This is not valid {}JSON!", .vtype = t_to_value, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_c_array_uint(void)
{
	char *expected = "[1,2]";
	char *test = "test_c_array_uint";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	memset(result, '\0', len);
	rp = result;

	const unsigned arr[] = {1, 2};
	const size_t cnt = sizeof(arr) / sizeof(arr[0]);
	const struct to_json tjs = { .value = arr, .count = &cnt, .vtype = t_to_uint, };
	return run_test(test, expected, result, &tjs, len);
}

static int
test_primitive_string(void)
{
	char *expected = "\"value\"";
	char *test = "test_primitive_string";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const struct to_json tjs = { .vtype = t_to_string, .value = "value", };
	return run_test(test, expected, result, &tjs, len);
}

static int
test_primitive_null(void)
{
	char *expected = "null";
	char *test = "test_primitive_null";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const struct to_json tjs = { .vtype = t_to_null, };
	return run_test(test, expected, result, &tjs, len);
}

static int
test_array_int(void)
{
	char *expected = "[1,2]";
	char *test = "test_array_int";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const int arr[] = {1, 2};
	const struct to_json tjs[] = {
		{ .value = &arr[0], .vtype = t_to_int, .stype = t_to_array, },
		{ .value = &arr[1], .vtype = t_to_int, .stype = t_to_array, },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_array_mixed(void)
{
	char *expected = "[1,\"2\"]";
	char *test = "test_array_mixed";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const int i = 1;
	const char *c = "2";
	const struct to_json tjs[] = {
		{ .value = &i, .vtype = t_to_int, .stype = t_to_array, },
		{ .value = c, .vtype = t_to_string, .stype = t_to_array, },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_c_array_int(void)
{
	char *expected = "[1,2]";
	char *test = "test_c_array_int";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const int arr[] = {1, 2};
	const size_t cnt = sizeof(arr) / sizeof(arr[0]);
	const struct to_json tjs = {
		.value = arr, .count = &cnt, .vtype = t_to_int,
	};
	return run_test(test, expected, result, &tjs, len);
}

static int
test_primitive_string_escape_chars(void)
{
	char *expected = "\"1\\\"2\\\\3\\\\4\\\"\"";
	char *test = "test_primitive_string_escape_chars";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const struct to_json tjs = {
		.vtype = t_to_string, .value = "1\"2\\3\\4\""
	};

	return run_test(test, expected, result, &tjs, len);
}

static int
test_object_from_rfc8259(void)
{
	char *expected = "{"
	                    "\"Image\":{"
	                        "\"Width\":800,"
	                        "\"Height\":600,"
	                        "\"Title\":\"View from 15th Floor\","
	                        "\"Thumbnail\":{"
	                            "\"Url\":\"http://www.example.com/image/481989943\","
	                            "\"Height\":125,"
	                            "\"Width\":100"
	                        "},"
	                        "\"Animated\":false,"
	                        "\"IDs\":[116,943,234,38793]"
	                      "}"
	                 "}";
	char *test = "test_object_from_rfc8259";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	_Bool animated = 0;
	unsigned ids[] = { 116, 943, 234, 38793 };
	size_t num_ids = sizeof(ids)/sizeof(ids[0]);

	char thumbnail_url[] = "http://www.example.com/image/481989943";
	unsigned thumbnail_height = 125;
	unsigned thumbnail_width = 100;

	unsigned image_width = 800;
	unsigned image_height = 600;
	char image_title[] = "View from 15th Floor";

	struct to_json thumbnail_tjs[] = {
		{. name = "Url", .value = &thumbnail_url, .vtype = t_to_string, .stype = t_to_object, },
		{ .name = "Height", .value = &thumbnail_height, .vtype = t_to_uint, },
		{ .name = "Width", .value = &thumbnail_width, .vtype = t_to_uint, },
		{ NULL }
	};

	struct to_json image_tjs[] = {
		{ .name = "Width", .value = &image_width, .vtype = t_to_uint, .stype = t_to_object, },
		{ .name = "Height", .value = &image_height, .vtype = t_to_uint, },
		{ .name = "Title", .value = image_title, .vtype = t_to_string, },
		{ .name = "Thumbnail", .value = thumbnail_tjs, .vtype = t_to_object, },
		{ .name = "Animated", .value = &animated, .vtype = t_to_boolean, },
		{ .name = "IDs", .value = &ids, .vtype = t_to_uint, .count = &num_ids, },
		{ NULL }
	};

	struct to_json tjs[] = {
		{ .name = "Image", .value = image_tjs, .vtype = t_to_object, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_array_from_rfc8259(void)
{
	char *expected = "["
	                   "{"
	                      "\"precision\":\"zip\","
	                      "\"Latitude\":37.7668,"
	                      "\"Longitude\":-122.3959,"
	                      "\"Address\":\"\","
	                      "\"City\":\"SAN FRANCISCO\","
	                      "\"State\":\"CA\","
	                      "\"Zip\":\"94107\","
	                      "\"Country\":\"US\""
	                   "},"
	                   "{"
	                      "\"precision\":\"zip\","
	                      "\"Latitude\":37.371991,"
	                      "\"Longitude\":-122.026020,"
	                      "\"Address\":\"\","
	                      "\"City\":\"SUNNYVALE\","
	                      "\"State\":\"CA\","
	                      "\"Zip\":\"94085\","
	                      "\"Country\":\"US\""
	                   "}"
	                 "]";
	char *test = "test_array_from_rfc8259";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	/*
	 * Since we do not have support for floating point values yet, we help
	 * ourselfs here by using t_to_value.
	 */
	char *precision[] = { "zip", "zip" };
	char *latitude[] = { "37.7668", "37.371991" };
	char *longitude[] = { "-122.3959", "-122.026020" };
	char *address[] = { "", "" };
	char *city[] = { "SAN FRANCISCO", "SUNNYVALE" };
	char *state[] = { "CA", "CA" };
	char *zip[] = { "94107", "94085" };
	char *country[] = { "US", "US" };

	struct to_json data_tjs[2][9] = {
		{
			{ .name = "precision", .value = precision[0], .vtype = t_to_string, .stype = t_to_array, },
			{ .name = "Latitude", .value = latitude[0], .vtype = t_to_value, },
			{ .name = "Longitude", .value = longitude[0], .vtype = t_to_value, },
			{ .name = "Address", .value = address[0], .vtype = t_to_string, },
			{ .name = "City", .value = city[0], .vtype = t_to_string, },
			{ .name = "State", .value = state[0], .vtype = t_to_string, },
			{ .name = "Zip", .value = zip[0], .vtype = t_to_string, },
			{ .name = "Country", .value = country[0], .vtype = t_to_string, },
			{ NULL }
		}, {
			{ .name = "precision", .value = precision[1], .vtype = t_to_string, .stype = t_to_array, },
			{ .name = "Latitude", .value = latitude[1], .vtype = t_to_value, },
			{ .name = "Longitude", .value = longitude[1], .vtype = t_to_value, },
			{ .name = "Address", .value = address[1], .vtype = t_to_string, },
			{ .name = "City", .value = city[1], .vtype = t_to_string, },
			{ .name = "State", .value = state[1], .vtype = t_to_string, },
			{ .name = "Zip", .value = zip[1], .vtype = t_to_string, },
			{ .name = "Country", .value = country[1], .vtype = t_to_string, },
			{ NULL }
		}
	};

	struct to_json tjs[] = {
		{ .value = data_tjs[0], .vtype = t_to_object, .stype = t_to_array, },
		{ .value = data_tjs[1], .vtype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_null_value(void)
{
	char *expected = "{\"name\":null}";
	char *test = "test_object_null_value";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	_Bool tmp = verbose;
	tell_single_test(test);
	verbose = 0;

	struct to_json tjs[] = {
		{ .name = "name", .stype = t_to_object, },
		{ NULL }
	};

	int err = 0;
	for (int i = 1; !err && i < t_to_value; i++) {
		tjs[0].vtype = (enum json_to_type)i;
		if (run_test(test, expected, result, tjs, len))
			err = i;
	}

	verbose = tmp;
	return err;
}

static int
test_primitive_hex(void)
{
	char *expected = "\"F\"";
	char *test = "test_primitive_hex";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	unsigned n = 15;
	const struct to_json tjs = { .value = &n, .vtype = t_to_hex, };
	return run_test(test, expected, result, &tjs, len);
}

static int
test_c_array_hex(void)
{
	char *expected = "[\"9\",\"A\",\"B\",\"F\",\"10\",\"11\",\"FE\",\"FF\",\"100\",\"FFF\",\"1000\",\"1001\",\"1010\",\"FFFE\",\"FFFF\"]";
	char *test = "test_c_array_hex";
	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	unsigned arr[] = { 9, 10, 11, 15, 16, 17, 254, 255, 256, 4095, 4096, 4097, 4112, 65534, 65535 };
	const size_t cnt = sizeof(arr) / sizeof(arr[0]);
	const struct to_json tjs = {
		.value = arr, .vtype = t_to_hex, .count = &cnt,
	};

	return run_test(test, expected, result, &tjs, len);
}

static int
test_array_primitive_all_int_types(void)
{
	char expected[512];
	char *test = "test_array_primitive_all_int_types";

	int max_int = INT_MAX;
	int min_int = INT_MIN;
	unsigned int max_uint = UINT_MAX;

	long max_long = LONG_MAX;
	long min_long = LONG_MIN;
	unsigned long max_ulong = ULONG_MAX;

	long long max_longlong = LLONG_MAX;
	long long min_longlong = LLONG_MIN;
	unsigned long long max_ulonglong = ULLONG_MAX;

	int8_t max_int8_t = INT8_MAX;
	int8_t min_int8_t = INT8_MIN;
	uint8_t max_uint8_t = UINT8_MAX;

	int16_t max_int16_t = INT16_MAX;
	int16_t min_int16_t = INT16_MIN;
	uint16_t max_uint16_t = UINT16_MAX;

	int32_t max_int32_t = INT32_MAX;
	int32_t min_int32_t = INT32_MIN;
	uint32_t max_uint32_t = UINT32_MAX;

	int64_t max_int64_t = INT64_MAX;
	int64_t min_int64_t = INT64_MIN;
	uint64_t max_uint64_t = UINT64_MAX;

	uint8_t max_hex_uint8_t = UINT8_MAX;
	uint16_t max_hex_uint16_t = UINT16_MAX;
	uint32_t max_hex_uint32_t = UINT32_MAX;
	uint64_t max_hex_uint64_t = UINT64_MAX;

	sprintf(expected, "[%d,%d,%u,"
	                  "%ld,%ld,%lu,"
	                  "%lld,%lld,%llu,"
	                  "%d,%d,%u,"
	                  "%d,%d,%u,"
	                  "%ld,%ld,%lu,"
	                  "%lld,%lld,%llu,"
	                  "\"%X\",\"%X\",\"%lX\",\"%llX\"]",
	                    max_int, min_int, max_uint,
	                    max_long, min_long, max_ulong,
	                    max_longlong, min_longlong, max_ulonglong,
	                    max_int8_t, min_int8_t, max_uint8_t,
	                    max_int16_t, min_int16_t, max_uint16_t,
	                    (long)max_int32_t, (long)min_int32_t, (unsigned long)max_uint32_t,
	                    (long long)max_int64_t, (long long)min_int64_t, (unsigned long long)max_uint64_t,
	                    max_hex_uint8_t, max_hex_uint16_t, (unsigned long)max_hex_uint32_t, (unsigned long long)max_hex_uint64_t
	       );

	size_t len = strlen(expected) + 1;
	assert(len <= MAXLEN);
	char result[MAXLEN];
	rp = result;

	const struct to_json tjs[] = {
		{ .value = &max_int, .vtype = t_to_int, .stype = t_to_array },
		{ .value = &min_int, .vtype = t_to_int },
		{ .value = &max_uint, .vtype = t_to_uint },
		{ .value = &max_long, .vtype = t_to_long },
		{ .value = &min_long, .vtype = t_to_long },
		{ .value = &max_ulong, .vtype = t_to_ulong },
		{ .value = &max_longlong, .vtype = t_to_longlong },
		{ .value = &min_longlong, .vtype = t_to_longlong },
		{ .value = &max_ulonglong, .vtype = t_to_ulonglong },
		{ .value = &max_int8_t, .vtype = t_to_int8_t },
		{ .value = &min_int8_t, .vtype = t_to_int8_t },
		{ .value = &max_uint8_t, .vtype = t_to_uint8_t },
		{ .value = &max_int16_t, .vtype = t_to_int16_t },
		{ .value = &min_int16_t, .vtype = t_to_int16_t },
		{ .value = &max_uint16_t, .vtype = t_to_uint16_t },
		{ .value = &max_int32_t, .vtype = t_to_int32_t },
		{ .value = &min_int32_t, .vtype = t_to_int32_t },
		{ .value = &max_uint32_t, .vtype = t_to_uint32_t },
		{ .value = &max_int64_t, .vtype = t_to_int64_t },
		{ .value = &min_int64_t, .vtype = t_to_int64_t },
		{ .value = &max_uint64_t, .vtype = t_to_uint64_t },
		{ .value = &max_hex_uint8_t, .vtype = t_to_hex_u8 },
		{ .value = &max_hex_uint16_t, .vtype = t_to_hex_u16 },
		{ .value = &max_hex_uint32_t, .vtype = t_to_hex_u32 },
		{ .value = &max_hex_uint64_t, .vtype = t_to_hex_u64 },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
exec_test(int i)
{
	switch (i){
	case 1:
		return test_array_primitive_all_int_types();
		break;
	case 2:
		return test_object_string();
		break;
	case 3:
		return test_object_boolean();
		break;
	case 4:
		return test_object_valuetype();
		break;
	case 5:
		return test_c_array_int();
		break;
	case 6:
		return test_c_array_boolean();
		break;
	case 7:
		return test_object_array_c_array();
		break;
	case 8:
		return test_c_array_empty();
		break;
	case 9:
		return test_object_array_one_empty();
		break;
	case 10:
		return test_object_object();
		break;
	case 11:
		return test_object_empty();
		break;
	case 12:
		return test_object_object_object();
		break;
	case 13:
		return test_object_object_nested_empty();
		break;
	case 14:
		return test_c_array_uint();
		break;
	case 15:
		return test_primitive_string();
		break;
	case 16:
		return test_primitive_null();
		break;
	case 17:
		return test_object_object_null();
		break;
	case 18:
		return test_array_int();
		break;
	case 19:
		return test_array_mixed();
		break;
	case 20:
		return test_primitive_string_escape_chars();
		break;
	case 21:
		return test_object_from_rfc8259();
		break;
	case 22:
		return test_array_from_rfc8259();
		break;
	case 23:
		return test_object_null_value();
		break;
	case 24:
		return test_primitive_hex();
		break;
	case 25:
		return test_c_array_hex();
		break;
#define MAXTEST 26
	case MAXTEST:
		return 0;
	default:
		fputs("No such test!\n", stderr);
		return 1;
	}
	return 1;
}

int
main(int argc, char *argv[])
{
	int failed_tests[MAXTEST];
	int failed = 0;
	int opt;
	int test = 0;
	int rv = 0;

	while ((opt = getopt(argc, argv, "hn:v")) != -1){
		switch (opt){
		case 'n':
			single_test = 1;
			if (!(test = atoi(optarg))) {
				fprintf(stderr, "Could not parse test number: %s\n",
						optarg);
				return 1;
			}
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			fputs("usage: test_mtojson [-n number]\n", stderr);
			return 1;
		}
	}

	if (test){
		rv = exec_test(test);
	} else {
		for (int i = 1; i <= MAXTEST; i++){
			rv = exec_test(i);
			if (verbose)
				printf("%d: %d\n", i, rv);
			if (rv)
				failed_tests[failed++] = i;
		}
	}

	if (verbose)
		printf("%s", "\n");

	if (failed){
		fprintf(stderr, "\n%s ", "Failed tests:");
		for (int i = 0; i < failed; i++)
			fprintf(stderr, "%d ", failed_tests[i]);
		fprintf(stderr, "%s", "\n");
	}
	return failed;
}
