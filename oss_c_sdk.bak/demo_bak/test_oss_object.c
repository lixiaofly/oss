#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_test_util.h"

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

    test_put_object_from_buffer();
    test_get_object_to_buffer();
    test_head_object();
    test_append_object_from_buffer();

    //aos_http_io_deinitialize last
    aos_http_io_deinitialize();

    return 0;
}
