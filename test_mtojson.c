/*
 * Tests for microtojson.h
 *
 * If generate_json does not detect an expected buffer overflow, running tests
 * will be aborted immediately with an exit status 125.
 *
 * If generate_json does detect an non-expected buffer overflow, running tests
 * will be aborted immediately with an exit status 124.
 *
 * If tests fail exit status is the count of failed tests. All succeeding tests
 * will be run and the number of the failed tests will be printed to stderr.
 *
 * Every test is run twice: the first time a buffer overflow is provoked - the
 * test must fail!
 */

#include "mtojson.h"

#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Bool single_test = 0;
int verbose = 0;

int
check_result(char *test, char *expected, char *result)
{
	int r = strcmp(result, expected);
	if (r != 0){
		fprintf(stderr, "\nFAILED: %s\n", test);
		fprintf(stderr, "Expected : %s\n", expected);
		fprintf(stderr, "Generated: %s\n", result);
	}
	return r != 0;
}

void
tell_single_test(char* test)
{
	if (single_test || verbose){
		printf("Running test: %-30s ", test);
		if (!verbose)
			printf("%s", "\n");
	}
}

int
test_json_string()
{
	char *expected = "{\"key\": \"value\"}";
	char *test = "test_json_string";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ .key = "key", .value = "value", .type = t_to_string, },
		{ NULL },
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_boolean()
{
	char *expected = "{\"key\": true}";
	char *test = "test_json_boolean";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	_Bool value = true;
	struct json_kv jkv[] = {
		{ .key = "key", .value = &value, .type = t_to_boolean, },
		{ NULL },
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_integer()
{
	char *expected = "{\"key\": 1}";
	char *test = "test_json_integer";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	int n = 1;
	struct json_kv jkv[] = {
		{ .key = "key", .value = &n, .type = t_to_integer, },
		{ NULL },
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_integer_two()
{
	char *expected = "{\"key\": 1, \"key\": 2}";
	char *test = "test_json_integer_two";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	int ns[] = {1, 2};

	struct json_kv jkv[] = {
		{ .type = t_to_integer, .key = "key", .value = &ns[0], },
		{ .type = t_to_integer, .key = "key", .value = &ns[1], },
		{ NULL }
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_array_integer()
{
	char *expected = "{\"array\": [1, 2]}";
	char *test = "test_json_array_integer";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	int arr[] = {1, 2};
	struct json_array jar = {
		.value = arr, .count = 2, .type = t_to_integer };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_array_string()
{
	char *expected = "{\"array\": [\"1\", \"23\"]}";
	char *test = "test_json_array_string";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	char *arr[8] = {"1", "23"};
	struct json_array jar = {
		.value = arr, .count = 2, .type = t_to_string };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_array_boolean()
{
	char *expected = "{\"array\": [true, false]}";
	char *test = "test_json_array_boolean";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	_Bool arr [] = {true, false};
	struct json_array jar = {
		.value = arr, .count = 2, .type = t_to_boolean };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_array_array()
{
	char *expected = "{\"array\": [[\"1\", \"2\", \"3\"], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_json_array_array";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	char *arr[] = {"1", "2", "3"};
	struct json_array inner_jar_arr = {
		.value = arr, .count = 3, .type = t_to_string };
	struct json_array *inner_jar[] = { &inner_jar_arr, &inner_jar_arr };
	struct json_array jar = {
		.value = inner_jar, .count = 2, .type = t_to_array };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};

	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_array_empty()
{
	char *expected = "{\"array\": []}";
	char *test = "test_json_array_empty";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	char *arr[1];
	struct json_array jar = {
		.value = arr, .count = 0, .type = t_to_string };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};

	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_array_empty_one()
{
	char *expected = "{\"array\": [[], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_json_array_one_empty";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	char *arr[] = {"1", "2", "3"};
	struct json_array inner_jar_arr = {
		.value = arr, .count = 3, .type = t_to_string };
	struct json_array inner_jar_empty = {
		.value = arr, .count = 0, .type = t_to_string };

	struct json_array *inner_jar[] = { &inner_jar_empty, &inner_jar_arr };
	struct json_array jar = {
		.value = inner_jar, .count = 2, .type = t_to_array };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_to_array, },
		{ NULL }
	};

	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_object_empty()
{
	char *expected = "{}";
	char *test = "test_json_object_empty";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ NULL },
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_object()
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
	char result[strlen(expected) + 1];
	tell_single_test(test);

	char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	struct json_array addarr = {
		.value = addresses, .count = 3, .type = t_to_string };

	int kid = 1;
	int cnt = 3;
	struct json_kv keys[] = {
		{ .key = "key_id", .value = &kid,     .type = t_to_integer },
		{ .key = "count",   .value = &cnt,     .type = t_to_integer },
		{ .key = "values", .value = &addarr, .type = t_to_array },
		{ NULL }
	};

	int nok = 1;
	struct json_kv jkv[] = {
		{ .key = "keys", .value = &keys, .type = t_to_object },
		{ .key = "number_of_keys", .value = &nok, .type = t_to_integer},
		{ NULL }
	};

	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_array_object()
{
	char *expected = "{"
	                   "\"keys\": [{"
	                           "\"key_id\": 1, "
	                           "\"count\": 3, "
	                           "\"values\": [\"DEADBEEF\", \"1337BEEF\", \"0000BEEF\"]"
	                   "}, {"
	                           "\"key_id\": 2, "
	                           "\"count\": 1, "
	                           "\"values\": [\"DEADFEED\"]"
	                   "}], "
	                   "\"number_of_keys\": 2"
	                 "}";

	char *test = "test_json_array_object";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	struct json_array addarr = {
		.value = addresses, .count = 3, .type = t_to_string };

	char *array2[] = {"DEADFEED"};
	struct json_array arr2 = {
		.value = array2, .count = 1, .type = t_to_string };

	int kid[] = { 1, 2 };
	int cnt[] = { 3, 1 };
	struct json_kv keys_kv[][4] = {
		{
		{ .key = "key_id", .value = &kid[0],     .type = t_to_integer },
		{ .key = "count",   .value = &cnt[0],     .type = t_to_integer },
		{ .key = "values", .value = &addarr, .type = t_to_array },
		{ NULL }
		}, {
		{ .key = "key_id", .value = &kid[1],     .type = t_to_integer },
		{ .key = "count",   .value = &cnt[1],     .type = t_to_integer },
		{ .key = "values", .value = &arr2, .type = t_to_array },
		{ NULL }
		}
	};

	int nok = 2;
	struct json_kv *keys_ptr[] = { keys_kv[0], keys_kv[1] };
	struct json_array keys = {
		.value = keys_ptr, .count = 2, .type = t_to_object };
	struct json_kv jkv[] = {
		{ .key = "keys", .value = &keys, .type = t_to_array },
		{ .key = "number_of_keys", .value = &nok, .type = t_to_integer},
		{ NULL }
	};

	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_object_object()
{
	char *expected = "{"
	                   "\"outer\": {"
	                            "\"middle\": {"
	                                          "\"inner\": true"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_json_object_object";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	_Bool value = true;

	struct json_kv inner[] = {
		{ .key = "inner", .value = &value, .type = t_to_boolean },
		{ NULL }
	};

	struct json_kv middle[] = {
		{ .key = "middle", .value = &inner, .type = t_to_object },
		{ NULL }
	};

	struct json_kv jkv[] = {
		{ .key = "outer", .value = &middle, .type = t_to_object },
		{ NULL }
	};

	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_object_nested_empty()
{
	char *expected = "{"
	                   "\"outer\": {"
	                            "\"middle\": {"
	                                          "\"inner\": {}"
	                            "}"
	                   "}"
	                 "}";

	char *test = "test_json_object_nested_empty";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	struct json_kv value[] = {
		{ NULL },
	};

	struct json_kv inner[] = {
		{ .key = "inner", .value = &value, .type = t_to_object },
		{ NULL }
	};

	struct json_kv middle[] = {
		{ .key = "middle", .value = &inner, .type = t_to_object },
		{ NULL }
	};

	struct json_kv jkv[] = {
		{ .key = "outer", .value = &middle, .type = t_to_object },
		{ NULL }
	};

	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
test_json_valuetype()
{
	char *expected = "{\"key\": This is not valid {}JSON!}";
	char *test = "test_json_valuetype";
	char result[strlen(expected) + 1];
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ .key = "key", .value = "This is not valid {}JSON!", .type = t_to_value, },
		{ NULL },
	};
	if (generate_json(result, strlen(expected), jkv)) exit(125);
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(124);
	return check_result(test, expected, result);
}

int
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
	default:
		fputs("No such test!\n", stderr);
		return 1;
	}
	return 1;
}
#define MAXTEST 16

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

	if (failed){
		fprintf(stderr, "\n%s ", "Failed tests:");
		for (int i = 0; i < failed; i++)
			fprintf(stderr, "%d ", failed_tests[i]);
		fprintf(stderr, "%s", "\n");
	}
	return failed;
}
