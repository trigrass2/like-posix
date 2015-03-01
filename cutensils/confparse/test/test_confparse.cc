
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "confparse.h"

//#define DEBGPRINTF(...)	printf(__VA_ARGS__)
#define DEBGPRINTF(...)

/**
 * fixture
 */
#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];
const char* filename = "test.conf";
const char* file_text_empty = "";
const char* file_text_invalid = "these\nlines\ncant\n\be\nparsed";
const char* file_text_valid_with_extra_newlines = "\n\nabc 123\nefg 456\n\n\n\n\nxyz 987\n\n";
const char* file_text_valid = "abc 123\n#hello\nefg 456\nxyz 987";
const char* valid_match_set[] = {
		"abc", "123",
		"efg", "456",
		"xyz", "987",
};
const char* file_text_valid_with_comments = "# comment1\nabc 123 #inlinecomment\n#efg 456\n#comment2\nxyz 987";
const char* valid_match_set_with_comments[] = {
		"abc", "123",
		"xyz", "987",
};
const char* new_entries[] = {
		"newkey1", "newvalue1",
		"newkey2", "newvalue2"
};
const char* valid_match_set_combined_new_entries[] = {
		"abc", "123",
		"efg", "456",
		"xyz", "987",
		"newkey1", "newvalue1",
		"newkey2", "newvalue2"
};
const char* valid_match_set_combined_new_entries_with_modifications[] = {
		"abc", "newvalue2",
		"efg", "456",
		"xyz", "newvalue2",
		"newkey1", "newvalue1",
};

const char* file_text_valid_with_comments_modified_retains_comments =
"# comment1\n"
"abc newvalue2 #inlinecomment\n"
"#efg 456\n"
"#comment2\n"
"xyz newvalue2\n"
"newkey1 newvalue1\n";

void create_file(const char* text)
{
	FILE* f = fopen(filename, "w");
	fprintf(f, text);
	fclose(f);
}

void delete_file()
{
	unlink(filename);
}

char* get_buffer()
{
	return buffer;
}

TEST(test_confparse, test_config_file_exists_no_file)
{
	bool ret = config_file_exists((const uint8_t*)filename);
	ASSERT_FALSE(ret);
}

TEST(test_confparse, test_config_file_exists_with_file)
{
	create_file(file_text_valid);
	bool ret = config_file_exists((const uint8_t*)filename);
	delete_file();
	ASSERT_TRUE(ret);
}

TEST(test_confparse, test_open_close_config_file_no_file)
{
	config_parser_t cfg;
	bool ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	ASSERT_FALSE(cfg.file);
	close_config_file(&cfg);
	ASSERT_FALSE(ret);
}

TEST(test_confparse, test_open_close_config_file_with_file)
{
	config_parser_t cfg;
	create_file(file_text_valid);
	bool ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	ASSERT_TRUE(cfg.file);
	close_config_file(&cfg);
	delete_file();
	ASSERT_TRUE(ret);
}

TEST(test_confparse, test_get_next_config_valid_data)
{
	int count = 0;
	int i = 0;
	config_parser_t cfg;
	create_file(file_text_valid);
	bool ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	if(ret)
	{
		while(get_next_config(&cfg))
		{
			ASSERT_STREQ((const char*)get_config_key(&cfg), valid_match_set[i]);
			ASSERT_TRUE(config_key_match(&cfg, (const uint8_t*)valid_match_set[i]));
			i++;
			ASSERT_STREQ((const char*)get_config_value(&cfg), valid_match_set[i]);
			ASSERT_TRUE(config_value_match(&cfg, (const uint8_t*)valid_match_set[i]));
			i++;
			count++;
		}
	}
	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 3);
}

TEST(test_confparse, test_get_next_config_empty_data)
{
	int count = 0;
	config_parser_t cfg;
	create_file(file_text_empty);
	bool ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	if(ret)
	{
		while(get_next_config(&cfg))
		{
			count++;
		}
	}
	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 0);
}

TEST(test_confparse, test_get_next_config_valid_commented_data)
{
	int count = 0;
	int i = 0;
	config_parser_t cfg;
	create_file(file_text_valid_with_comments);
	bool ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	if(ret)
	{
		while(get_next_config(&cfg))
		{
			ASSERT_STREQ((const char*)get_config_key(&cfg), valid_match_set_with_comments[i]);
			ASSERT_TRUE(config_key_match(&cfg, (const uint8_t*)valid_match_set_with_comments[i]));
			i++;
			ASSERT_STREQ((const char*)get_config_value(&cfg), valid_match_set_with_comments[i]);
			ASSERT_TRUE(config_value_match(&cfg, (const uint8_t*)valid_match_set_with_comments[i]));
			i++;
			count++;
		}
	}
	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 2);
}

TEST(test_confparse, test_get_next_config_valid_data_extra_newlines)
{
	int count = 0;
	int i = 0;
	config_parser_t cfg;
	create_file(file_text_valid_with_extra_newlines);
	bool ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	if(ret)
	{
		while(get_next_config(&cfg))
		{
			ASSERT_STREQ((const char*)get_config_key(&cfg), valid_match_set[i]);
			ASSERT_TRUE(config_key_match(&cfg, (const uint8_t*)valid_match_set[i]));
			i++;
			ASSERT_STREQ((const char*)get_config_value(&cfg), valid_match_set[i]);
			ASSERT_TRUE(config_value_match(&cfg, (const uint8_t*)valid_match_set[i]));
			i++;
			count++;
		}
	}
	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 3);
}

TEST(test_confparse, test_get_config_value_by_key_valid_data_valid_key1)
{
	config_parser_t cfg;
	create_file(file_text_valid);
	const uint8_t* value = get_config_value_by_key((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)"abc");
	delete_file();
	ASSERT_STREQ((const char*)value, "123");
}


TEST(test_confparse, test_get_config_value_by_key_valid_data_valid_key2)
{
	config_parser_t cfg;
	create_file(file_text_valid);
	const uint8_t* value = get_config_value_by_key((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)"efg");
	delete_file();
	ASSERT_STREQ((const char*)value, "456");
}

TEST(test_confparse, test_get_config_value_by_key_valid_data_valid_key3)
{
	config_parser_t cfg;
	create_file(file_text_valid);
	const uint8_t* value = get_config_value_by_key((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)"xyz");
	delete_file();
	ASSERT_STREQ((const char*)value, "987");
}

TEST(test_confparse, test_get_config_value_by_key_valid_data_invalid_key)
{
	config_parser_t cfg;
	create_file(file_text_valid);
	const uint8_t* value = get_config_value_by_key((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)"gfd");
	delete_file();
	ASSERT_STREQ((const char*)value, NULL);
}

TEST(test_confparse, test_config_key_in_list)
{
	bool ret = config_key_in_list((const uint8_t*)"abc", 10, (const uint8_t**)valid_match_set);
	ASSERT_TRUE(ret);
	ret = config_key_in_list((const uint8_t*)"efg", 10, (const uint8_t**)valid_match_set);
	ASSERT_TRUE(ret);
	ret = config_key_in_list((const uint8_t*)"xyz", 10, (const uint8_t**)valid_match_set);
	ASSERT_TRUE(ret);
	ret = config_key_in_list((const uint8_t*)"gfd", 10, (const uint8_t**)valid_match_set);
	ASSERT_FALSE(ret);
	ret = config_key_in_list((const uint8_t*)"ab", 10, (const uint8_t**)valid_match_set);
	ASSERT_FALSE(ret);
	ret = config_key_in_list((const uint8_t*)"zy", 10, (const uint8_t**)valid_match_set);
	ASSERT_FALSE(ret);
}

TEST(test_confparse, test_string_match)
{
	bool ret = string_match("abc", (const uint8_t*)"abc");
	ASSERT_TRUE(ret);
	ret = string_match("abc", (const uint8_t*)"abcd");
	ASSERT_FALSE(ret);
	ret = string_match("abcd", (const uint8_t*)"abc");
	ASSERT_FALSE(ret);
	ret = string_match(NULL, (const uint8_t*)"abc");
	ASSERT_FALSE(ret);
	ret = string_match("abcd", NULL);
	ASSERT_FALSE(ret);
}

/**
 * start with empty config file
 *
 * add 2 entries:
 * newkey1 newvalue1
 * newkey2 newvalue2
 *
 * expect result:
 * newkey1 newvalue1
 * newkey2 newvalue2
 *
 * expect iterator to cycle 2 times
 */
TEST(test_confparse, test_add_config_entry_empty_file_add_twice)
{
	config_parser_t cfg;
	int count = 0;
	int i = 0;
	create_file(file_text_empty);
	bool ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[0], (const uint8_t*)new_entries[1]);
	ASSERT_TRUE(ret);
	ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[2], (const uint8_t*)new_entries[3]);
	ASSERT_TRUE(ret);

	ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	if(ret)
	{
		while(get_next_config(&cfg))
		{
			DEBGPRINTF("%d %s %s %s\n", count, (const char*)get_config_key(&cfg),(const char*)get_config_value(&cfg),(const char*)get_config_comment(&cfg));
			ASSERT_STREQ((const char*)get_config_key(&cfg), new_entries[i]);
			ASSERT_TRUE(config_key_match(&cfg, (const uint8_t*)new_entries[i]));
			i++;
			ASSERT_STREQ((const char*)get_config_value(&cfg), new_entries[i]);
			ASSERT_TRUE(config_value_match(&cfg, (const uint8_t*)new_entries[i]));
			i++;
			count++;
		}
	}
	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 2);
}

/**
 * start with empty config file
 *
 * add 1 entry:
 * newkey1 newvalue1
 * modify that entry:
 * newkey1 newvalue2
 *
 * expect result:
 * newkey1 newvalue2
 *
 * expect iterator to cycle 1 times
 */
TEST(test_confparse, test_add_config_entry_empty_file_add_and_modify)
{
	config_parser_t cfg;
	int count = 0;
	int i = 0;
	create_file(file_text_empty);
	bool ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[0], (const uint8_t*)new_entries[1]);
	ASSERT_TRUE(ret);
	ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[0], (const uint8_t*)new_entries[3]);
	ASSERT_TRUE(ret);

	ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);

	if(ret)
	{
		while(get_next_config(&cfg))
		{
			DEBGPRINTF("%d %s %s %s\n", count, (const char*)get_config_key(&cfg),(const char*)get_config_value(&cfg),(const char*)get_config_comment(&cfg));
			ASSERT_STREQ((const char*)get_config_key(&cfg), new_entries[0]);
			ASSERT_TRUE(config_key_match(&cfg, (const uint8_t*)new_entries[0]));
			ASSERT_STREQ((const char*)get_config_value(&cfg), new_entries[3]);
			ASSERT_TRUE(config_value_match(&cfg, (const uint8_t*)new_entries[3]));
			count++;
		}
	}

	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 1);
}

/**
 * start with config file:
 * abc 123
 * efg 456
 * xyz 987
 *
 * add 2 entries:
 * newkey1 newvalue1
 * newkey2 newvalue2
 *
 * expect result:
 * abc 123
 * efg 456
 * xyz 987
 * newkey1 newvalue1
 * newkey2 newvalue2
 *
 *
 * expect iterator to cycle 5 times
 */
TEST(test_confparse, test_add_config_entry_file_already_has_content_add_twice)
{
	config_parser_t cfg;
	int count = 0;
	int i = 0;
	create_file(file_text_valid);
	bool ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[0], (const uint8_t*)new_entries[1]);
	ASSERT_TRUE(ret);
	ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[2], (const uint8_t*)new_entries[3]);
	ASSERT_TRUE(ret);

	ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	if(ret)
	{
		while(get_next_config(&cfg))
		{
			DEBGPRINTF("%d %s %s %s\n", count, (const char*)get_config_key(&cfg),(const char*)get_config_value(&cfg),(const char*)get_config_comment(&cfg));
			ASSERT_STREQ((const char*)get_config_key(&cfg), valid_match_set_combined_new_entries[i]);
			ASSERT_TRUE(config_key_match(&cfg, (const uint8_t*)valid_match_set_combined_new_entries[i]));
			i++;
			ASSERT_STREQ((const char*)get_config_value(&cfg), valid_match_set_combined_new_entries[i]);
			ASSERT_TRUE(config_value_match(&cfg, (const uint8_t*)valid_match_set_combined_new_entries[i]));
			i++;
			count++;
		}
	}
	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 5);
}

/**
 * start with config file:
 * abc 123
 * efg 456
 * xyz 987
 *
 * add 1 entries:
 * newkey1 newvalue1
 * modify 1 entry:
 * abc newvalue2
 * modify 1 entry:
 * xyz newvalue2
 *
 * expect result:
 * abc newvalue2
 * efg 456
 * xyz newvalue2
 * newkey1 newvalue1
 *
 * expect iterator to cycle 4 times
 */
TEST(test_confparse, test_add_config_entry_file_already_has_content_add_and_modify)
{
	config_parser_t cfg;
	int count = 0;
	int i = 0;
	create_file(file_text_valid);
	bool ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[0], (const uint8_t*)new_entries[1]);
	ASSERT_TRUE(ret);
	ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)valid_match_set[0], (const uint8_t*)new_entries[3]);
	ASSERT_TRUE(ret);
	ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)valid_match_set[4], (const uint8_t*)new_entries[3]);
	ASSERT_TRUE(ret);

	ret = open_config_file(&cfg, (uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename);
	if(ret)
	{
		while(get_next_config(&cfg))
		{
			DEBGPRINTF("%d %s %s %s\n", count, (const char*)get_config_key(&cfg),(const char*)get_config_value(&cfg),(const char*)get_config_comment(&cfg));
			ASSERT_STREQ((const char*)get_config_key(&cfg), valid_match_set_combined_new_entries_with_modifications[i]);
			ASSERT_TRUE(config_key_match(&cfg, (const uint8_t*)valid_match_set_combined_new_entries_with_modifications[i]));
			i++;
			ASSERT_STREQ((const char*)get_config_value(&cfg), valid_match_set_combined_new_entries_with_modifications[i]);
			ASSERT_TRUE(config_value_match(&cfg, (const uint8_t*)valid_match_set_combined_new_entries_with_modifications[i]));
			i++;
			count++;
		}
	}
	close_config_file(&cfg);
	delete_file();
	ASSERT_EQ(count, 4);
}

/**
 * start with config file:
 *
 * # comment1
 * abc 123
 * #efg 456
 * #comment2
 * xyz 987"
 *
 * add 1 entries:
 * newkey1 newvalue1
 * modify 1 entry:
 * abc newvalue2
 * modify 1 entry:
 * xyz newvalue2
 *
 * expect result:
 * # comment1
 * abc newvalue2
 * #efg 456
 * #comment2
 * xyz newvalue2
 * newkey1 newvalue1
 *
 * expect iterator to cycle 4 times
 */
TEST(test_confparse, test_add_config_entry_file_already_has_content_comments_retained)
{
	create_file(file_text_valid_with_comments);
	bool ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)new_entries[0], (const uint8_t*)new_entries[1]);
	ASSERT_TRUE(ret);
	ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)valid_match_set[0], (const uint8_t*)new_entries[3]);
	ASSERT_TRUE(ret);
	ret = add_config_entry((uint8_t*)get_buffer(), BUFFER_SIZE, (const uint8_t*)filename, (const uint8_t*)valid_match_set[4], (const uint8_t*)new_entries[3]);
	ASSERT_TRUE(ret);

	int res = -1;
	char buf[1024] = {0};
	FILE* f = fopen(filename, "r");
	if(f)
	{
		fread(buf, 1, 1024, f);
		fclose(f);
	}

	DEBGPRINTF("expect:\n%s\n\n", file_text_valid_with_comments_modified_retains_comments);
	DEBGPRINTF("got:\n%s\n\n", buf);

	ASSERT_STREQ(buf, file_text_valid_with_comments_modified_retains_comments);

	delete_file();
}

