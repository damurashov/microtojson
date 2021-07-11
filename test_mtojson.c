/*
 * Tests for microtojson.h
 *
 * If generate_json DOES NOT detect an EXPECTED buffer overflow, running tests
 * will be aborted immediately with an exit status 125.
 *
 * If generate_json DOES detect an NON-EXPECTED buffer overflow, running tests
 * will be aborted immediately with an exit status 124.
 *
 * If generate_json returns the wrong string length, running tests will be
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
 * This file is Copyright (c) 2020 by Rene Kita
 */

#include "mtojson.h"

#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

_Bool single_test = 0;
int verbose = 0;

// Helper pointer, use 'display rp' inside gdb to see the string grow
const char *rp;

static void tell_single_test();

static int
run_test(char *test, char *expected, char *result, const struct json_kv *jkv, size_t len)
{
	int err = 0;

	tell_single_test(test);

	memset(result, '\0', len);
	size_t l = generate_json(result, jkv, len);
	if (!l) {
		if (verbose)
			printf("%s\n", "NON-EXPECTED buffer overflow");
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
	if (len >= 10 && generate_json(result, jkv, len - 10))
		err += 2;
	memset(result, '\0', len);
	if (generate_json(result, jkv, len / 2))
		err += 4;
	memset(result, '\0', len);
	if (generate_json(result, jkv, len - 1))
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
		printf("Running test: %-30s ", test);
		if (!verbose)
			printf("%s", "\n");
	}
}

static int
test_json_string(void)
{
	char *expected = "{\"key\": \"value\"}";
	char *test = "test_json_string";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const struct json_kv jkv[] = {
		{ .key = "key", .value = "value", .type = t_to_string, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_boolean(void)
{
	char *expected = "{\"key\": true}";
	char *test = "test_json_boolean";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const _Bool value = true;
	const struct json_kv jkv[] = {
		{ .key = "key", .value = &value, .type = t_to_boolean, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_integer(void)
{
	char *expected = "{\"key\": 1}";
	char *test = "test_json_integer";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int n = 1;
	const struct json_kv jkv[] = {
		{ .key = "key", .value = &n, .type = t_to_integer, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_integer_two(void)
{
	char *expected = "{\"key\": -32767, \"key\": 32767}";
	char *test = "test_json_integer_two";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int ns[] = {-32767, 32767};

	const struct json_kv jkv[] = {
		{ .type = t_to_integer, .key = "key", .value = &ns[0], },
		{ .type = t_to_integer, .key = "key", .value = &ns[1], },
		{ NULL }
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_uinteger(void)
{
	char *expected = "{\"key\": 65535}";
	char *test = "test_json_uinteger";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int n = 65535;
	const struct json_kv jkv[] = {
		{ .key = "key", .value = &n, .type = t_to_uinteger, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}


static int
test_json_array_integer(void)
{
	char *expected = "{\"array\": [1, 2]}";
	char *test = "test_json_array_integer";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int arr[] = {1, 2};
	const struct json_array jar = {
		.value = arr, .count = 2, .type = t_to_integer };

	const struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_array_string(void)
{
	char *expected = "{\"array\": [\"1\", \"23\"]}";
	char *test = "test_json_array_string";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[8] = {"1", "23"};
	const struct json_array jar = {
		.value = arr, .count = 2, .type = t_to_string };

	const struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_array_boolean(void)
{
	char *expected = "{\"array\": [true, false]}";
	char *test = "test_json_array_boolean";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const _Bool arr [] = {true, false};
	const struct json_array jar = {
		.value = arr, .count = 2, .type = t_to_boolean };

	const struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_array_array(void)
{
	char *expected = "{\"array\": [[\"1\", \"2\", \"3\"], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_json_array_array";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[] = {"1", "2", "3"};
	const struct json_array inner_jar_arr = {
		.value = arr, .count = 3, .type = t_to_string };

	const struct json_array *inner_jar[] = { &inner_jar_arr, &inner_jar_arr };
	const struct json_array jar = {
		.value = inner_jar, .count = 2, .type = t_to_array };

	const struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};

	return run_test(test, expected, result, jkv, len);
}

static int
test_json_array_empty(void)
{
	char *expected = "{\"array\": []}";
	char *test = "test_json_array_empty";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[1];
	const struct json_array jar = {
		.value = arr, .count = 0, .type = t_to_string };

	const struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};

	return run_test(test, expected, result, jkv, len);
}

static int
test_json_array_empty_one(void)
{
	char *expected = "{\"array\": [[], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_json_array_one_empty";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[] = {"1", "2", "3"};
	const struct json_array inner_jar_arr = {
		.value = arr, .count = 3, .type = t_to_string };
	const struct json_array inner_jar_empty = {
		.value = NULL, .count = 0, .type = t_to_string };

	const struct json_array *inner_jar[] = { &inner_jar_empty, &inner_jar_arr };
	const struct json_array jar = {
		.value = inner_jar, .count = 2, .type = t_to_array };

	const struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};

	return run_test(test, expected, result, jkv, len);
}

static int
test_json_object_empty(void)
{
	char *expected = "{}";
	char *test = "test_json_object_empty";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	struct json_kv jkv[] = {
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_object(void)
{
	char *expected = "{"
	                   "\"keys\": {"
	                           "\"key_id\": 1, "
	                           "\"count\": 3, "
	                           "\"values\": [\"DEADBEEF\", \"1337BEEF\", \"0000BEEF\"]"
	                   "}, "
	                   "\"number_of_keys\": 1"
	                 "}";

	char *test = "test_json_object";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	const struct json_array addarr = {
		.value = addresses, .count = 3, .type = t_to_string };

	const int kid = 1;
	const int cnt = 3;
	const struct json_kv keys[] = {
		{ .key = "key_id", .value = &kid,    .type = t_to_integer },
		{ .key = "count",  .value = &cnt,    .type = t_to_integer },
		{ .key = "values", .value = &addarr, .type = t_to_array },
		{ NULL }
	};

	const int nok = 1;
	const struct json_kv jkv[] = {
		{ .key = "keys",           .value = &keys, .type = t_to_object },
		{ .key = "number_of_keys", .value = &nok,  .type = t_to_integer},
		{ NULL }
	};

	return run_test(test, expected, result, jkv, len);
}

static int
test_json_array_object(void)
{
	char *expected = "{"
	                   "\"keys\": [{"
	                           "\"key_id\": 1, "
	                           "\"count\": 3, "
	                           "\"values\": [\"DEADBEEF\", \"1337BEEF\", \"0000BEEF\"]"
	                   "}, {}, {"
	                           "\"key_id\": 2, "
	                           "\"count\": 1, "
	                           "\"values\": [\"DEADFEED\"]"
	                   "}], "
	                   "\"number_of_keys\": 2"
	                 "}";

	char *test = "test_json_array_object";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	const struct json_array addarr = {
		.value = addresses, .count = 3, .type = t_to_string };

	const char *array2[] = {"DEADFEED"};
	const struct json_array arr2 = {
		.value = array2, .count = 1, .type = t_to_string };

	const int kid[] = { 1, 2 };
	const int cnt[] = { 3, 1 };
	const struct json_kv keys_kv[][4] = {
		{
			{ .key = "key_id", .value = &kid[0], .type = t_to_integer },
			{ .key = "count",  .value = &cnt[0], .type = t_to_integer },
			{ .key = "values", .value = &addarr, .type = t_to_array },
			{ NULL }
		}, {
			{ NULL }
		}, {
			{ .key = "key_id", .value = &kid[1], .type = t_to_integer },
			{ .key = "count",  .value = &cnt[1], .type = t_to_integer },
			{ .key = "values", .value = &arr2,   .type = t_to_array },
			{ NULL }
		}
	};

	const int nok = 2;
	const struct json_kv *keys_ptr[] = { keys_kv[0], keys_kv[1], keys_kv[2] };
	const struct json_array keys = {
		.value = keys_ptr, .count = 3, .type = t_to_object };
	const struct json_kv jkv[] = {
		{ .key = "keys",           .value = &keys, .type = t_to_array },
		{ .key = "number_of_keys", .value = &nok,  .type = t_to_integer},
		{ NULL }
	};

	return run_test(test, expected, result, jkv, len);
}

static int
test_json_object_object(void)
{
	char *expected = "{"
	                   "\"outer\": {"
	                            "\"middle\": {"
	                                          "\"inner\": true"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_json_object_object";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const _Bool value = true;

	const struct json_kv inner[] = {
		{ .key = "inner", .value = &value, .type = t_to_boolean },
		{ NULL }
	};

	const struct json_kv middle[] = {
		{ .key = "middle", .value = &inner, .type = t_to_object },
		{ NULL }
	};

	const struct json_kv jkv[] = {
		{ .key = "outer", .value = &middle, .type = t_to_object },
		{ NULL }
	};

	return run_test(test, expected, result, jkv, len);
}

static int
test_json_object_nested_empty(void)
{
	char *expected = "{"
	                   "\"outer\": {"
	                            "\"middle\": {"
	                                          "\"inner\": {}"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_json_object_nested_empty";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const struct json_kv value[] = {
		{ NULL },
	};

	const struct json_kv inner[] = {
		{ .key = "inner", .value = &value, .type = t_to_object },
		{ NULL }
	};

	const struct json_kv middle[] = {
		{ .key = "middle", .value = &inner, .type = t_to_object },
		{ NULL }
	};

	const struct json_kv jkv[] = {
		{ .key = "outer", .value = &middle, .type = t_to_object },
		{ NULL }
	};

	return run_test(test, expected, result, jkv, len);
}

static int
test_json_valuetype(void)
{
	char *expected = "{\"key\": This is not valid {}JSON!}";
	char *test = "test_json_valuetype";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const struct json_kv jkv[] = {
		{ .key = "key", .value = "This is not valid {}JSON!", .type = t_to_value, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_int_max(void)
{
	char int_max[20];
	sprintf(int_max, "%d", INT_MAX);
	char expected[30];
	strcpy(expected, "{\"key\": ");
	strcat(strcat(expected, int_max), "}");

	char *test = "test_json_int_max";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int n = INT_MAX;
	const struct json_kv jkv[] = {
		{ .key = "key", .value = &n, .type = t_to_integer, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);

}

static int
test_json_int_min(void)
{
	char uint_min[20];
	sprintf(uint_min, "%d", INT_MIN);
	char expected[30];
	strcpy(expected, "{\"key\": ");
	strcat(strcat(expected, uint_min), "}");

	char *test = "test_json_int_min";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	int n = INT_MIN;
	const struct json_kv jkv[] = {
		{ .key = "key", .value = &n, .type = t_to_integer, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
test_json_uint_max(void)
{
	char uint_max[20];
	sprintf(uint_max, "%u", UINT_MAX);
	char expected[30];
	strcpy(expected, "{\"key\": ");
	strcat(strcat(expected, uint_max), "}");

	char *test = "test_json_uint_max";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const unsigned n = UINT_MAX;
	const struct json_kv jkv[] = {
		{ .key = "key", .value = &n, .type = t_to_uinteger, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
exec_test(int i)
{
	switch (i){
	case 1:
		return test_json_integer();
		break;
	case 2:
		return test_json_integer_two();
		break;
	case 3:
		return test_json_string();
		break;
	case 4:
		return test_json_boolean();
		break;
	case 5:
		return test_json_valuetype();
		break;
	case 6:
		return test_json_array_integer();
		break;
	case 7:
		return test_json_array_boolean();
		break;
	case 8:
		return test_json_array_string();
		break;
	case 9:
		return test_json_array_array();
		break;
	case 10:
		return test_json_array_empty();
		break;
	case 11:
		return test_json_array_empty_one();
		break;
	case 12:
		return test_json_object();
		break;
	case 13:
		return test_json_array_object();
		break;
	case 14:
		return test_json_object_empty();
		break;
	case 15:
		return test_json_object_object();
		break;
	case 16:
		return test_json_object_nested_empty();
		break;
	case 17:
		return test_json_uinteger();
		break;
	case 18:
		return test_json_int_max();
		break;
	case 19:
		return test_json_int_min();
		break;
	case 20:
		return test_json_uint_max();
		break;
	default:
		fputs("No such test!\n", stderr);
		return 1;
	}
	return 1;
}
#define MAXTEST 20

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
			test = atoi(optarg);
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
