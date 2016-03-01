#include <sys/stat.h>
#include "oss_config.h"
#include "oss_api.h"
#include "oss_test_util.h"

void make_rand_string(aos_pool_t *p, int len, aos_string_t *data)
{
    char *str = NULL;
    int i = 0;
    str = (char *)aos_palloc(p, len + 1);
    for ( ; i < len; i++) {
        str[i] = rand() % 128;
    }
    str[len] = '\0';
    aos_str_set(data, str);
}

aos_buf_t *make_random_buf(aos_pool_t *p, int len)
{
    int bytes;
    aos_buf_t *b;
    aos_string_t str;

    make_rand_string(p, 16, &str);
    b = aos_create_buf(p, len);

    while (b->last < b->end) {
        bytes = b->end - b->last;
        bytes = aos_min(bytes, 16);
        memcpy(b->last, str.data, bytes);
        b->last += bytes;
    }

    return b;
}

void make_random_body(aos_pool_t *p, int count, aos_list_t *bc)
{
    int i = 0;
    int len;
    aos_buf_t *b;

    for (; i < count; ++i) {
        len = random() % 4096;
        b = make_random_buf(p, len);
        aos_list_add_tail(&b->node, bc);
    }
}

void init_test_config(oss_config_t *config, int is_oss_domain)
{
    aos_str_set(&config->host, TEST_OSS_HOST);
    config->port = 80; 
    aos_str_set(&config->id, TEST_ACCESS_ID);
    aos_str_set(&config->key, TEST_ACCESS_KEY);
    config->is_oss_domain = is_oss_domain;
}

void init_test_request_options(oss_request_options_t *options, int is_oss_domain)
{
    options->config = oss_config_create(options->pool);
    init_test_config(options->config, is_oss_domain);
    options->ctl = aos_http_controller_create(options->pool, 0);
}

aos_status_t * create_test_bucket(const oss_request_options_t *options,
    const char *bucket_name, oss_acl_e oss_acl)
{
    aos_string_t bucket;
    aos_table_t *resp_headers;
    aos_status_t * s;

    aos_str_set(&bucket, bucket_name);
    resp_headers = aos_table_make(options->pool, 5);

    s = aos_status_create(options->pool);
    s = oss_create_bucket(options, &bucket, oss_acl, &resp_headers);
    return s;
}

aos_status_t *create_test_object(const oss_request_options_t *options, const char *bucket_name, 
    const char *object_name, const char *data, aos_table_t *headers)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *resp_headers;
    aos_list_t buffer;
    aos_buf_t *content;
    aos_status_t * s;
    int ret;

    test_object_base();
    aos_list_init(&buffer);
    content = aos_buf_pack(options->pool, data, strlen(data));
    aos_list_add_tail(&content->node, &buffer);

    s = oss_put_object_from_buffer(options, &bucket, &object, &buffer, headers, &resp_headers);
    return s;
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

aos_status_t *delete_test_object(const oss_request_options_t *options, 
    const char *bucket_name, const char *object_name)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *resp_headers;
    aos_status_t * s;
    int ret;

    s = aos_status_create(options->pool);

    test_object_base();
    s = oss_delete_object(options, &bucket, &object, &resp_headers);
    return s;
}

aos_status_t *init_test_multipart_upload(const oss_request_options_t *options, 
    const char *bucket_name, const char *object_name, aos_string_t *upload_id)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_status_t *s;
    oss_acl_e oss_acl;
    int ret;

    test_object_base();
    headers = aos_table_make(options->pool, 5);
    oss_acl = OSS_ACL_PUBLIC_READ;

    s = oss_init_multipart_upload(options, &bucket, &object, headers, upload_id, &resp_headers);

    return s;
}

aos_status_t *abort_test_multipart_upload(const oss_request_options_t *options, 
    const char *bucket_name, const char *object_name, aos_string_t *upload_id)
{
    aos_string_t bucket;
    aos_string_t object;
    aos_table_t *resp_headers;
    aos_status_t *s;
    int ret;

    test_object_base();
    s = oss_abort_multipart_upload(options, &bucket, &object, upload_id, &resp_headers);

    return s;
}

char *gen_test_signed_url(const oss_request_options_t *options, const char *bucket_name,
    const char *object_name, int64_t expires, aos_http_request_t *req)
{
    aos_string_t bucket;
    aos_string_t object;
    char *signed_url = NULL;
    int ret;

    aos_str_set(&bucket, bucket_name);
    aos_str_set(&object, object_name);
    signed_url = oss_gen_signed_url(options, &bucket, &object, expires, req);
    return signed_url;
}

unsigned long get_file_size(const char *file_path)
{
    unsigned long filesize = -1; 
    struct stat statbuff;

    if(stat(file_path, &statbuff) < 0){
        return filesize;
    } else {
        filesize = statbuff.st_size;
    }

    return filesize;
}
