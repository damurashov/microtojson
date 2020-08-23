#include "mtojson.h"

#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { len = 10000 };
char result[len];
_Bool single_test = 0;

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
	if (single_test)
		fprintf(stderr, "Running test: %s\n", test);
}

int
test_json_string()
{
	char *expected = "{\"key\": \"value\"}";
	char *test = "test_json_string";
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ .key = "key", .value = "value", .type = t_json_string, },
		{ NULL },
	};
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_integer()
{
	char *expected = "{\"key\": 1}";
	char *test = "test_json_integer";
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ .key = "key", .value = "1", .type = t_json_integer, },
		{ NULL },
	};
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_integer_two()
{
	char *expected = "{\"key\": 1, \"key\": 2}";
	char *test = "test_json_integer";
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ .type = t_json_integer, .key = "key", .value = "1", },
		{ .type = t_json_integer, .key = "key", .value = "2", },
		{ NULL }
	};
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_array_integer()
{
	char *expected = "{\"array\": [1, 2]}";
	char *test = "test_json_array_integer";
	tell_single_test(test);

	char *arr[8] = {"1", "2"};
	struct json_array jar = {
		.value = (void*)arr, .count = 2, .type = t_json_integer };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_json_array, },
		{ NULL }
	};
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_array_string()
{
	char *expected = "{\"array\": [\"1\", \"23\"]}";
	char *test = "test_json_array_string";
	tell_single_test(test);

	char *arr[8] = {"1", "23"};
	struct json_array jar = {
		.value = (void*)arr, .count = 2, .type = t_json_string };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_json_array, },
		{ NULL }
	};
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_array_array()
{
	char *expected = "{\"array\": [[\"1\", \"2\", \"3\"], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_json_array_array";
	tell_single_test(test);

	char *arr[] = {"1", "2", "3"};
	struct json_array inner_jar_arr = {
		.value = (void*)arr, .count = 3, .type = t_json_string };
	struct json_array *inner_jar[] = { &inner_jar_arr, &inner_jar_arr };
	struct json_array jar = {
		.value = (void*)inner_jar, .count = 2, .type = t_json_array };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_json_array, },
		{ NULL }
	};

	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_array_empty()
{
	char *expected = "{\"array\": []}";
	char *test = "test_json_array_empty";
	tell_single_test(test);

	char *arr[1];
	struct json_array jar = {
		.value = (void*)arr, .count = 0, .type = t_json_string };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_json_array, },
		{ NULL }
	};

	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_array_empty_one()
{
	char *expected = "{\"array\": [[], [\"1\", \"2\", \"3\"]]}";
	char *test = "test_json_array_one_empty";
	tell_single_test(test);

	char *arr[] = {"1", "2", "3"};
	struct json_array inner_jar_arr = {
		.value = (void*)arr, .count = 3, .type = t_json_string };
	struct json_array inner_jar_empty = {
		.value = (void*)arr, .count = 0, .type = t_json_string };

	struct json_array *inner_jar[] = { &inner_jar_empty, &inner_jar_arr };
	struct json_array jar = {
		.value = (void*)inner_jar, .count = 2, .type = t_json_array };

	struct json_kv jkv[] = {
		{ .key = "array", .value = &jar, .type = t_json_array, },
		{ NULL }
	};

	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_object_empty()
{
	char *expected = "{}";
	char *test = "test_json_object_empty";
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ NULL },
	};
	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
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
	tell_single_test(test);

	char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	struct json_array addarr = {
		.value = (void*)addresses, .count = 3, .type = t_json_string };

	struct json_kv keys[] = {
		{ .key = "key_id", .value = "1",     .type = t_json_integer },
		{ .key = "count",   .value = "3",     .type = t_json_integer },
		{ .key = "values", .value = &addarr, .type = t_json_array },
		{ NULL }
	};

	struct json_kv jkv[] = {
		{ .key = "keys", .value = &keys, .type = t_json_object },
		{ .key = "number_of_keys", .value = "1", .type = t_json_integer},
		{ NULL }
	};

	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
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
	tell_single_test(test);

	char *addresses[] = {"DEADBEEF", "1337BEEF", "0000BEEF"};
	struct json_array addarr = {
		.value = (void*)addresses, .count = 3, .type = t_json_string };

	char *array2[] = {"DEADFEED"};
	struct json_array arr2 = {
		.value = (void*)array2, .count = 1, .type = t_json_string };

	struct json_kv keys_kv[][4] = {
		{
		{ .key = "key_id", .value = "1",     .type = t_json_integer },
		{ .key = "count",   .value = "3",     .type = t_json_integer },
		{ .key = "values", .value = &addarr, .type = t_json_array },
		{ NULL }
		}, {
		{ .key = "key_id", .value = "2",     .type = t_json_integer },
		{ .key = "count",   .value = "1",     .type = t_json_integer },
		{ .key = "values", .value = &arr2, .type = t_json_array },
		{ NULL }
		}
	};

	struct json_kv *keys_ptr[] = { keys_kv[0], keys_kv[1] };
	struct json_array keys = {
		.value = (void*)keys_ptr, .count = 2, .type = t_json_object };
	struct json_kv jkv[] = {
		{ .key = "keys", .value = &keys, .type = t_json_array },
		{ .key = "number_of_keys", .value = "2", .type = t_json_integer},
		{ NULL }
	};

	if (!generate_json(result, strlen(expected) + 1, jkv)) exit(-1);
	return check_result(test, expected, result);
}

int
test_json_integer_buffer_overflow()
{
	char *expected = "{\"key\": \"value\"}";
	char *test = "test_json_integer_buffer_overflow";
	tell_single_test(test);

	struct json_kv jkv[] = {
		{ .key = "key", .value = "value", .type = t_json_string, },
		{ NULL },
	};
	if (generate_json(result, strlen(expected) - 1, jkv))
		exit(-1);
	return 0;
}

int
exec_test(int i)
{
	switch (i){
	case 1:
		return test_json_string();
		break;
	case 2:
		return test_json_integer();
		break;
	case 3:
		return test_json_integer_two();
		break;
	case 4:
		return test_json_array_integer();
		break;
	case 5:
		return test_json_array_string();
		break;
	case 6:
		return test_json_array_array();
		break;
	case 7:
		return test_json_array_empty();
		break;
	case 8:
		return test_json_array_empty_one();
		break;
	case 9:
		return test_json_object();
		break;
	case 10:
		return test_json_array_object();
		break;
	case 11:
		return test_json_integer_buffer_overflow();
		break;
	case 12:
		return test_json_object_empty();
		break;
	default:
		fputs("No such test!\n", stderr);
		return 1;
	}
	return 1;
}
#define MAXTEST 12

int
main(int argc, char *argv[])
{
	int opt;
	int test = 0;
	int rv = 0;

	while ((opt = getopt(argc, argv, "hn:")) != -1){
		switch (opt){
		case 'n':
			single_test = 1;
			test = atoi(optarg);
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
			rv += exec_test(i);
		}
	}

	return rv;
}
