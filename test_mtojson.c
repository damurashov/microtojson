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
run_test(char *test, char *expected, char *result, const struct to_json *tjs, size_t len)
{
	int err = 0;

	tell_single_test(test);

	memset(result, '\0', len);
	size_t l = json_generate(result, tjs, len);
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
	char *expected = "{\"name\": \"value\"}";
	char *test = "test_object_string";
	size_t len = strlen(expected) + 1;
	char result[len];
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
	char *expected = "{\"name\": true}";
	char *test = "test_object_boolean";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const _Bool value = true;
	const struct to_json tjs[] = {
		{ .name = "name", .value = &value, .vtype = t_to_boolean, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_integer(void)
{
	char *expected = "{\"name\": 1}";
	char *test = "test_object_integer";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int n = 1;
	const struct to_json tjs[] = {
		{ .name = "name", .value = &n, .vtype = t_to_integer, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_integer_two(void)
{
	char *expected = "{\"name\": -32767, \"name\": 32767}";
	char *test = "test_object_integer_two";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int ns[] = {-32767, 32767};

	const struct to_json tjs[] = {
		{ .vtype = t_to_integer, .name = "name", .value = &ns[0], .stype = t_to_object, },
		{ .vtype = t_to_integer, .name = "name", .value = &ns[1], },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_uinteger(void)
{
	char *expected = "{\"name\": 65535}";
	char *test = "test_object_uinteger";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int n = 65535;
	const struct to_json tjs[] = {
		{ .name = "name", .value = &n, .vtype = t_to_uinteger, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}


static int
test_object_c_array_integer(void)
{
	char *expected = "{\"array\": [9, 10, 11, 99, 100, 101, 110, 1000, 1001, 1010, 1100]}";
	char *test = "test_object_c_array_integer";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int arr[] = {9, 10, 11, 99, 100, 101, 110, 1000, 1001, 1010, 1100};
	const struct to_json jar = {
		.value = arr, .count = sizeof(arr)/sizeof(arr[0]), .vtype = t_to_integer, .stype = t_to_object,
	};

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_string(void)
{
	char *expected = "{\"array\": [\"1\", \"23\"]}";
	char *test = "test_object_c_array_string";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[8] = {"1", "23"};
	const struct to_json jar = {
		.value = arr, .count = 2, .vtype = t_to_string, .stype = t_to_object, };

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_boolean(void)
{
	char *expected = "{\"array\": [true, false]}";
	char *test = "test_object_c_array_boolean";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const _Bool arr [] = {true, false};
	const struct to_json jar = {
		.value = arr, .count = 2, .vtype = t_to_boolean, .stype = t_to_object, };

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_array(void)
{
	char *expected = "{\"array\": [[\"1\", \"2\", \"3\"], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_object_c_array_array";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[] = {"1", "2", "3"};
	const struct to_json inner_jar_arr = {
		.value = arr, .count = 3, .vtype = t_to_string, .stype = t_to_object, };

	const struct to_json *inner_jar[] = { &inner_jar_arr, &inner_jar_arr };
	const struct to_json jar = {
		.value = inner_jar, .count = 2, .vtype = t_to_array, .stype = t_to_object, };

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_empty(void)
{
	char *expected = "{\"array\": []}";
	char *test = "test_object_c_array_empty";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[1];
	const struct to_json jar = {
		.value = arr, .count = 0, .vtype = t_to_string, .stype = t_to_object, };

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_empty_one(void)
{
	char *expected = "{\"array\": [[], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_object_c_array_one_empty";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *arr[] = {"1", "2", "3"};
	const struct to_json inner_jar_arr = {
		.value = arr, .count = 3, .vtype = t_to_string, .stype = t_to_object, };
	const struct to_json inner_jar_empty = {
		.value = NULL, .count = 0, .vtype = t_to_string, .stype = t_to_object, };

	const struct to_json *inner_jar[] = { &inner_jar_empty, &inner_jar_arr };
	const struct to_json jar = {
		.value = inner_jar, .count = 2, .vtype = t_to_array, .stype = t_to_object, };

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
	char result[len];
	rp = result;

	struct to_json tjs[] = {
		{ .stype = t_to_object },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_object(void)
{
	char *expected = "{"
	                   "\"names\": {"
	                           "\"name_id\": 1, "
	                           "\"count\": 3, "
	                           "\"values\": [\"DEADBEEF\", \"1337BEEF\", \"0000BEEF\"]"
	                   "}, "
	                   "\"number_of_names\": 1"
	                 "}";

	char *test = "test_object_object";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	const struct to_json addarr = {
		.value = addresses, .count = 3, .vtype = t_to_string, .stype = t_to_array, };

	const int nid = 1;
	const int cnt = 3;
	const struct to_json names[] = {
		{ .name = "name_id", .value = &nid,    .vtype = t_to_integer, .stype = t_to_object, },
		{ .name = "count",  .value = &cnt,    .vtype = t_to_integer, },
		{ .name = "values", .value = &addarr, .vtype = t_to_array, },
		{ NULL }
	};

	const int non = 1;
	const struct to_json tjs[] = {
		{ .name = "names",           .value = &names, .vtype = t_to_object, .stype = t_to_object, },
		{ .name = "number_of_names", .value = &non,  .vtype = t_to_integer},
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_object(void)
{
	char *expected = "{"
	                   "\"names\": [{"
	                           "\"name_id\": 1, "
	                           "\"count\": 3, "
	                           "\"values\": [\"DEADBEEF\", \"1337BEEF\", \"0000BEEF\"]"
	                   "}, {}, {"
	                           "\"name_id\": 2, "
	                           "\"count\": 1, "
	                           "\"values\": [\"DEADFEED\"]"
	                   "}], "
	                   "\"number_of_names\": 2"
	                 "}";

	char *test = "test_object_c_array_object";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	const struct to_json addarr = {
		.value = addresses, .count = 3, .vtype = t_to_string, .stype = t_to_object, };

	const char *array2[] = {"DEADFEED"};
	const struct to_json arr2 = {
		.value = array2, .count = 1, .vtype = t_to_string, .stype = t_to_object, };

	const int nid[] = { 1, 2 };
	const int cnt[] = { 3, 1 };
	const struct to_json names_kv[][4] = {
		{
			{ .name = "name_id", .value = &nid[0], .vtype = t_to_integer, .stype = t_to_object, },
			{ .name = "count",  .value = &cnt[0], .vtype = t_to_integer, },
			{ .name = "values", .value = &addarr, .vtype = t_to_array, },
			{ NULL }
		}, {
			{ NULL }
		}, {
			{ .name = "name_id", .value = &nid[1], .vtype = t_to_integer, .stype = t_to_object, },
			{ .name = "count",  .value = &cnt[1], .vtype = t_to_integer, },
			{ .name = "values", .value = &arr2,   .vtype = t_to_array, },
			{ NULL }
		}
	};

	const int non = 2;
	const struct to_json *names_ptr[] = { names_kv[0], names_kv[1], names_kv[2] };
	const struct to_json names = {
		.value = names_ptr, .count = 3, .vtype = t_to_object, .stype = t_to_object, };
	const struct to_json tjs[] = {
		{ .name = "names",           .value = &names, .vtype = t_to_array, .stype = t_to_object, },
		{ .name = "number_of_names", .value = &non,  .vtype = t_to_integer},
		{ NULL }
	};

	return run_test(test, expected, result, tjs, len);
}

static int
test_object_object_object(void)
{
	char *expected = "{"
	                   "\"outer\": {"
	                            "\"middle\": {"
	                                          "\"inner\": true"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_object_object_object";
	size_t len = strlen(expected) + 1;
	char result[len];
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
	                   "\"outer\": {"
	                            "\"middle\": {"
	                                          "\"inner\": {}"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_object_object_nested_empty";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const struct to_json value[] = {
		{ NULL },
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
	char *expected = "{\"name\": This is not valid {}JSON!}";
	char *test = "test_object_valuetype";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const struct to_json tjs[] = {
		{ .name = "name", .value = "This is not valid {}JSON!", .vtype = t_to_value, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_int_max(void)
{
	char int_max[20];
	sprintf(int_max, "%d", INT_MAX);
	char expected[30];
	strcpy(expected, "{\"name\": ");
	strcat(strcat(expected, int_max), "}");

	char *test = "test_object_int_max";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const int n = INT_MAX;
	const struct to_json tjs[] = {
		{ .name = "name", .value = &n, .vtype = t_to_integer, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);

}

static int
test_object_int_min(void)
{
	char uint_min[20];
	sprintf(uint_min, "%d", INT_MIN);
	char expected[30];
	strcpy(expected, "{\"name\": ");
	strcat(strcat(expected, uint_min), "}");

	char *test = "test_object_int_min";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	int n = INT_MIN;
	const struct to_json tjs[] = {
		{ .name = "name", .value = &n, .vtype = t_to_integer, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_uint_max(void)
{
	char uint_max[20];
	sprintf(uint_max, "%u", UINT_MAX);
	char expected[30];
	strcpy(expected, "{\"name\": ");
	strcat(strcat(expected, uint_max), "}");

	char *test = "test_object_uint_max";
	size_t len = strlen(expected) + 1;
	char result[len];
	rp = result;

	const unsigned n = UINT_MAX;
	const struct to_json tjs[] = {
		{ .name = "name", .value = &n, .vtype = t_to_uinteger, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_uinteger(void)
{
	char *expected = "{\"array\": [1, 2]}";
	char *test = "test_object_c_array_uinteger";
	size_t len = strlen(expected) + 1;
	char result[len];
	memset(result, '\0', len);
	rp = result;

	const unsigned arr[] = {1, 2};
	const struct to_json jar = {
		.value = arr, .count = 2, .vtype = t_to_uinteger, .stype = t_to_object, };

	const struct to_json tjs[] = {
		{ .name = "array", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL }
	};
	return run_test(test, expected, result, tjs, len);
}

static int
test_object_c_array_valuetype(void)
{
	char *expected = "{\"name\": ["
	                            "This is not valid {}JSON!, "
	                            "This not valid {}JSON!, "
	                            "]}";
	char *test = "test_object_c_array_valuetype";
	size_t len = strlen(expected) + 1;
	char result[len];
	memset(result, '\0', len);
	rp = result;

	const char *arr[] = {"This is not valid {}JSON!", "This not valid {}JSON!, "};
	const struct to_json jar = {
		.value = arr, .count = 2, .vtype = t_to_value, };

	const struct to_json jkv[] = {
		{ .name = "name", .value = &jar, .vtype = t_to_array, .stype = t_to_object, },
		{ NULL },
	};
	return run_test(test, expected, result, jkv, len);
}

static int
exec_test(int i)
{
	switch (i){
	case 1:
		return test_object_integer();
		break;
	case 2:
		return test_object_integer_two();
		break;
	case 3:
		return test_object_string();
		break;
	case 4:
		return test_object_boolean();
		break;
	case 5:
		return test_object_valuetype();
		break;
	case 6:
		return test_object_c_array_integer();
		break;
	case 7:
		return test_object_c_array_boolean();
		break;
	case 8:
		return test_object_c_array_string();
		break;
	case 9:
		return test_object_c_array_array();
		break;
	case 10:
		return test_object_c_array_empty();
		break;
	case 11:
		return test_object_c_array_empty_one();
		break;
	case 12:
		return test_object_object();
		break;
	case 13:
		return test_object_c_array_object();
		break;
	case 14:
		return test_object_empty();
		break;
	case 15:
		return test_object_object_object();
		break;
	case 16:
		return test_object_object_nested_empty();
		break;
	case 17:
		return test_object_uinteger();
		break;
	case 18:
		return test_object_int_max();
		break;
	case 19:
		return test_object_int_min();
		break;
	case 20:
		return test_object_uint_max();
		break;
	case 21:
		return test_object_c_array_uinteger();
		break;
	case 22:
		return test_object_c_array_valuetype();
		break;
	default:
		fputs("No such test!\n", stderr);
		return 1;
	}
	return 1;
}
#define MAXTEST 22

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
