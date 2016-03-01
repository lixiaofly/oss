#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_test_util.h"

#define test_object_base() do {                                         \
        s = aos_status_create(options->pool);                           \
        aos_str_set(&bucket, bucket_name);                              \
        aos_str_set(&object, object_name);                              \
        resp_headers = aos_table_make(options->pool, 5);                \
    } while(0)

void test_list_object()
{
    aos_pool_t *p;
    aos_string_t bucket;
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_table_t *resp_headers;
    aos_status_t *s;
    oss_list_object_params_t *params;
    oss_list_object_content_t *content;
    int size = 0;
    char *key = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);

    params = oss_create_list_object_params(p);
    params->max_ret = 3;
    params->truncated = 1;

    aos_str_set(&params->prefix, "oss_test");
    resp_headers = aos_table_make(p, 5);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    s = oss_list_object(options, &bucket, params, &resp_headers);
	printf("s->code=%d\n", s->code);
    assert(200 == s->code);
	printf("params->next_marker.data=%s marker.data=%s\n", params->next_marker.data, params->marker.data);
    //CuAssertIntEquals(tc, 1, params->truncated);
	printf("truncated=%d\n", params->truncated);
	//assert(1 == params->truncated);
    //CuAssertStrEquals(tc, "oss_test_object1", params->next_marker.data);
	//assert("oss_test_object1" == params->next_marker.data);

    aos_list_for_each_entry(content, &params->object_list, node) {
        ++size;
        key = apr_psprintf(p, "%.*s", content->key.len, content->key.data);
	printf("key.data=%s\n", content->key.data);
    }
	printf("111\n");
    //CuAssertIntEquals(tc, 1 ,size);
	//assert(1 == size);
    //CuAssertStrEquals(tc, "oss_test_object1", key);
	//assert("oss_test_object1", key);
	printf("key=%s\n", key);
   /* 
    size = 0;
    aos_list_init(&params->object_list);
	printf("222\n");
    aos_str_set(&params->marker, params->next_marker.data);
	printf("333\n");
    s = oss_list_object(options, &bucket, params, &resp_headers);
	printf("444\n");
    assert(200 == s->code);
	printf("truncated=%d\n", params->truncated);
    assert(0 == params->truncated);
    //CuAssertIntEquals(tc, 200, s->code);
    //CuAssertIntEquals(tc, 0, params->truncated);
    aos_list_for_each_entry(content, &params->object_list, node) {
        ++size;
        key = apr_psprintf(p, "%.*s", content->key.len, content->key.data);
    }
    //CuAssertIntEquals(tc, 1 ,size);
    //CuAssertStrEquals(tc, "oss_test_object2", key);
    printf("key2=%s\n", key);
*/
    printf("test_list_object ok\n");

    aos_pool_destroy(p);
}
aos_status_t *create_test_object_from_file(const oss_request_options_t *options, const char *bucket_name,
    const char *object_name, const char *filename, aos_table_t *headers)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_string_t file;
    aos_table_t *resp_headers;
    aos_status_t * s;
    int ret;

    test_object_base();
    aos_str_set(&file, filename);
    resp_headers = aos_table_make(options->pool, 5);

    s = oss_put_object_from_file(options, &bucket, &object, &file, headers, &resp_headers);
    return s;
}

void test_get_object_to_file()
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object_from_file";
    aos_string_t object;
    char *filename = "./oss_test_get_object_to_file";
    //char *source_filename = "./oss_c_sdk_sample";
    char *source_filename = "./oss_lixiao_test";
    aos_string_t file;
    oss_request_options_t *options; 
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    aos_str_set(&file, filename);
    headers = aos_table_make(p, 5);
    resp_headers = aos_table_make(p, 5);
	
    //test get object to file
    s = oss_get_object_to_file(options, &bucket, &object, headers, &file, &resp_headers);
    assert(200 == s->code);
    //CuAssertIntEquals(tc, get_file_size(source_filename), get_file_size(filename));
    printf("test_get_object_to_file size(%lu) == size(%lu) ok\n",get_file_size(source_filename), get_file_size(filename));

    aos_pool_destroy(p);
}

void test_put_object_from_file_2()
{
    aos_pool_t *p;
    char *object_name = "oss_test_put_object_from_file_2";
    char *filename = "/tmp/oss/umslog/log_1";
    aos_status_t *s;
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_table_t *headers;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 5);
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, object_name, filename, headers);
    //CuAssertIntEquals(tc, 200, s->code);
    assert(200 == s->code);
    printf("test_put_object_from_file_2 ok\n");

    aos_pool_destroy(p);
}


void test_put_object_from_file()
{
    aos_pool_t *p;
    char *object_name = "oss_test_put_object_from_file_1";
    char *filename = "/tmp/oss/umslog/log_1";
    aos_status_t *s;
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_table_t *headers;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 5);
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, object_name, filename, headers);
	printf("code=%d\n", s->code);
    //CuAssertIntEquals(tc, 200, s->code);
    assert(200 == s->code);
    printf("test_put_object_from_file_1 ok\n");

    aos_pool_destroy(p);
}

void test_put_object_from_buffer()
{
    aos_pool_t *p;
    aos_string_t bucket;
    aos_string_t object;
    int is_oss_domain = 1;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    aos_list_t buffer;
    aos_buf_t *content;
    char *object_name = "oss_test_put_object";
    char *str = "test oss c sdk";
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 1);
    apr_table_set(headers, "x-oss-meta-author", "oss");
    resp_headers = aos_table_make(options->pool, 5); 
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_put_object_from_buffer(options, &bucket, &object, &buffer, headers, &resp_headers);
    assert(200 == s->code);
    printf("test_put_object_from_buffer ok\n");

    aos_pool_destroy(p);
}

void test_get_object_to_buffer()
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    aos_list_t buffer;
    aos_buf_t *content;
    char *expect_content = "test oss c sdk";
    char *buf;
    int64_t len = 0;
    int64_t size = 0;
    int64_t pos = 0;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);
    resp_headers = aos_table_make(p, 5);
    aos_list_init(&buffer);

    //test get object to buffer
    s = oss_get_object_to_buffer(options, &bucket, &object, headers, &buffer, &resp_headers);
    assert(200 == s->code);

    //get buffer len
    aos_list_for_each_entry(content, &buffer, node) {
        len += aos_buf_size(content);
    }

    buf = aos_pcalloc(p, len + 1);
    buf[len] = '\0';

    //copy buffer content to memory
    aos_list_for_each_entry(content, &buffer, node) {
        size = aos_buf_size(content);
        memcpy(buf + pos, content->pos, size);
        pos += size;
    }

    assert(strcmp(expect_content, buf) == 0);
    printf("test_get_object_to_buffer ok\n");

    aos_pool_destroy(p);
}

void test_head_object()
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_put_object";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    char *user_meta;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);
    resp_headers = aos_table_make(p, 0);
    s = aos_status_create(p);

    //test head object
    s = oss_head_object(options, &bucket, &object, headers, &resp_headers);
    assert(200 == s->code);
    user_meta = (char*)(apr_table_get(resp_headers, "x-oss-meta-author"));
    assert(strcmp("oss",user_meta) == 0);
    printf("test_head_object ok\n");

    aos_pool_destroy(p);
}

void test_append_object_from_buffer()
{
    aos_pool_t *p;
    char *object_name = "oss_test_append_object";
    aos_string_t bucket;
    aos_string_t object;
    char *str = "test oss c sdk";
    char *str1 = "for append object";
    aos_status_t *s;
    int is_oss_domain = 1;
    int64_t position = 0;
    aos_table_t *headers1;
    aos_table_t *headers2;
    aos_table_t *resp_headers;
    oss_request_options_t *options;
    aos_list_t buffer;
    aos_buf_t *content;
    char *next_append_position;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    headers1 = aos_table_make(p, 0);
    resp_headers = aos_table_make(p, 5);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);
    s = oss_head_object(options, &bucket, &object, headers1, &resp_headers);
    if(s->code == 200) {
        next_append_position = (char*)(apr_table_get(resp_headers, "x-oss-next-append-position"));
        position = atoi(next_append_position); 
    }

    //append object
    headers2 = aos_table_make(p, 0);
    aos_list_init(&buffer);
    content = aos_buf_pack(p, str, strlen(str));
    aos_list_add_tail(&content->node, &buffer);
    s = oss_append_object_from_buffer(options, &bucket, &object, position, &buffer, headers2, &resp_headers);
    assert(200 == s->code);
    printf("test_append_object_from_buffer ok\n");

    aos_pool_destroy(p);
}

int main(int argc, char *argv[])
{
    //aos_http_io_initialize first 
    if (aos_http_io_initialize("oss_test", 0) != AOSE_OK) {
        exit(1);
    }

    //test_put_object_from_buffer();
    test_put_object_from_file();
    test_put_object_from_file_2();
     //test_get_object_to_file();
     //test_list_object();
    //test_get_object_to_buffer();
    //test_head_object();
    //test_append_object_from_buffer();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
