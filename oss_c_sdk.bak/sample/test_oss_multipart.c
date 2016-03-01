#include "CuTest.h"
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "aos_transport.h"
#include "aos_http_io.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_test_util.h"

void test_multipart_setup(CuTest *tc)
{
    aos_pool_t *p;
    int is_oss_domain = 1;
    aos_status_t *s;
    oss_request_options_t *options;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;

    //create test bucket
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);

    aos_pool_destroy(p);
}

void test_multipart_cleanup(CuTest *tc)
{
    aos_pool_t *p;
    int is_oss_domain = 1;
    aos_string_t bucket;
    aos_status_t *s;
    oss_request_options_t *options;
    char *object_name = "oss_test_multipart_upload";
    char *object_name1 = "oss_test_multipart_upload_from_file";
    char *object_name2 = "oss_test_upload_part_copy_dest_object";
    char *object_name3 = "oss_test_upload_part_copy_source_object";
    aos_table_t *resp_headers;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);

    //delete test object
    delete_test_object(options, TEST_BUCKET_NAME, object_name);
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);
    delete_test_object(options, TEST_BUCKET_NAME, object_name3);

    //delete test bucket
    resp_headers = aos_table_make(p, 5);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s= oss_delete_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);

    aos_pool_destroy(p);
}

void test_init_abort_multipart_upload(CuTest *tc)
{
    aos_pool_t *p;
    char *object_name = "oss_test_abort_multipart_upload";
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_string_t upload_id;
    aos_status_t *s;

    //test init multipart
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //abort multipart
    s = abort_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 204, s->code);
    printf("test_init_abort_multipart_upload ok\n");

    aos_pool_destroy(p);
}

void test_list_multipart_upload(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name1 = "oss_test_abort_multipart_upload1";
    char *object_name2 = "oss_test_abort_multipart_upload2";
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_string_t upload_id1;
    aos_string_t upload_id2;
    aos_status_t *s;
    aos_table_t *resp_headers;
    oss_list_multipart_upload_params_t *params;
    char *expect_next_key_marker = "oss_test_abort_multipart_upload1";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name1, &upload_id1);
    CuAssertIntEquals(tc, 200, s->code);
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name2, &upload_id2);
    CuAssertIntEquals(tc, 200, s->code);

    resp_headers = aos_table_make(p, 5);
    params = oss_create_list_multipart_upload_params(p);
    params->max_ret = 1;
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_multipart_upload(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 1, params->truncated);
    CuAssertStrEquals(tc, expect_next_key_marker, params->next_key_marker.data);

    aos_list_init(&params->upload_list);
    aos_str_set(&params->key_marker, params->next_key_marker.data);
    aos_str_set(&params->upload_id_marker, params->next_upload_id_marker.data);
    s = oss_list_multipart_upload(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);

    s = abort_test_multipart_upload(options, TEST_BUCKET_NAME, object_name1, &upload_id1);
    CuAssertIntEquals(tc, 204, s->code);
    s = abort_test_multipart_upload(options, TEST_BUCKET_NAME, object_name2, &upload_id2);
    CuAssertIntEquals(tc, 204, s->code);
    printf("test_list_multipart_upload ok\n");

    aos_pool_destroy(p);
}

void test_multipart_upload(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_multipart_upload";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_status_t *s;
    aos_list_t buffer;
    aos_table_t *upload_part_resp_headers;
    oss_list_upload_part_params_t *params;
    aos_table_t *list_part_resp_headers;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content1;
    oss_list_part_content_t *part_content2;
    oss_complete_part_content_t *complete_content1;
    oss_complete_part_content_t *complete_content2;
    aos_table_t *complete_resp_headers;
    int part_num = 1;
    int part_num1 = 2;
    char *expect_part_num_marker = "1";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part
    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);

    upload_part_resp_headers = aos_table_make(p, 5);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num, &buffer, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_list_init(&buffer);
    make_random_body(p, 200, &buffer);
    s = oss_upload_part_from_buffer(options, &bucket, &object, &upload_id,
        part_num1, &buffer, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //list part
    list_part_resp_headers = aos_table_make(p, 5);
    params = oss_create_list_upload_part_params(p);
    params->max_ret = 1;
    aos_list_init(&complete_part_list);

    s = oss_list_upload_part(options, &bucket, &object, &upload_id, params, &list_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 1, params->truncated);
    CuAssertStrEquals(tc, expect_part_num_marker, params->next_part_number_marker.data);

    aos_list_for_each_entry(part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    aos_list_init(&params->part_list);
    aos_str_set(&params->part_number_marker, params->next_part_number_marker.data);
    s = oss_list_upload_part(options, &bucket, &object, &upload_id, params, &list_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);

    aos_list_for_each_entry(part_content2, &params->part_list, node) {
        complete_content2 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content2->part_number, part_content2->part_number.data);
        aos_str_set(&complete_content2->etag, part_content2->etag.data);
        aos_list_add_tail(&complete_content2->node, &complete_part_list);
    }

    //complete multipart
    complete_resp_headers = aos_table_make(p, 5);
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
        &complete_part_list, &complete_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_multipart_upload ok\n");

    aos_pool_destroy(p);
}

void test_multipart_upload_from_file(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = "oss_test_multipart_upload_from_file";
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_status_t *s;
    oss_upload_file_t *upload_file;
    aos_table_t *upload_part_resp_headers;
    oss_list_upload_part_params_t *params;
    aos_table_t *list_part_resp_headers;
    aos_string_t upload_id;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content1;
    oss_complete_part_content_t *complete_content1;
    aos_table_t *complete_resp_headers;
    int part_num = 1;
    int part_num1 = 2;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    aos_str_set(&object, object_name);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part from file
    upload_file = oss_create_upload_file(p); 
    aos_str_set(&upload_file->filename, "./oss_c_sdk_sample");
    upload_file->file_pos = 0;
    upload_file->file_last = 200 * 1024; //200k
    
    upload_part_resp_headers = aos_table_make(p, 5);
    s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num, upload_file, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    
    upload_file->file_pos = 200 *1024;//remain content start pos
    upload_file->file_last = get_file_size("./oss_c_sdk_sample");
    
    s = oss_upload_part_from_file(options, &bucket, &object, &upload_id,
        part_num1, upload_file, &upload_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    
    //list part
    list_part_resp_headers = aos_table_make(p, 5);
    params = oss_create_list_upload_part_params(p);
    aos_str_set(&params->part_number_marker, "");
    params->max_ret = 10;
    params->truncated = 0;
    aos_list_init(&complete_part_list);
    
    s = oss_list_upload_part(options, &bucket, &object, &upload_id, params, &list_part_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);

    aos_list_for_each_entry(part_content1, &params->part_list, node) {
        complete_content1 = oss_create_complete_part_content(p);
        aos_str_set(&complete_content1->part_number, part_content1->part_number.data);
        aos_str_set(&complete_content1->etag, part_content1->etag.data);
        aos_list_add_tail(&complete_content1->node, &complete_part_list);
    }

    //complete multipart
    complete_resp_headers = aos_table_make(p, 5);
    s = oss_complete_multipart_upload(options, &bucket, &object, &upload_id,
        &complete_part_list, &complete_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_multipart_upload_from_file ok\n");

    aos_pool_destroy(p);
}

void test_upload_part_copy(CuTest *tc)
{
    aos_pool_t *p;
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_string_t upload_id;
    oss_list_upload_part_params_t *list_upload_part_params;
    oss_upload_part_copy_params_t *upload_part_copy_params1;
    oss_upload_part_copy_params_t *upload_part_copy_params2;
    aos_table_t *headers;
    aos_table_t *resp_headers;
    aos_table_t *list_part_resp_headers;
    aos_list_t complete_part_list;
    oss_list_part_content_t *part_content;
    oss_complete_part_content_t *complete_content;
    aos_table_t *complete_resp_headers;
    aos_status_t *s;
    int part1 = 1;
    int part2 = 2;
    char *local_filename = "./test_upload_part_copy_file";
    char *download_filename = "./test_upload_part_copy_file_download";
    char *source_object_name = "oss_test_upload_part_copy_source_object";
    char *dest_object_name = "oss_test_upload_part_copy_dest_object";
    aos_string_t download_file;
    aos_string_t dest_bucket;
    aos_string_t dest_object;
    aos_string_t source_object_content;
    int64_t range_start1 = 0;
    int64_t range_end1 = 6000000;
    int64_t range_start2 = 6000001;
    int64_t range_end2;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);

    init_test_request_options(options, is_oss_domain);
    headers = aos_table_make(p, 0);
    s = create_test_object_from_file(options, TEST_BUCKET_NAME, source_object_name, 
        local_filename, headers);
    CuAssertIntEquals(tc, 200, s->code);

    //init mulitipart
    s = init_test_multipart_upload(options, TEST_BUCKET_NAME, dest_object_name, &upload_id);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part copy 1
    upload_part_copy_params1 = oss_create_upload_part_copy_params(p);
    aos_str_set(&upload_part_copy_params1->source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params1->source_object, source_object_name);
    aos_str_set(&upload_part_copy_params1->dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params1->dest_object, dest_object_name);
    aos_str_set(&upload_part_copy_params1->upload_id, upload_id.data);
    upload_part_copy_params1->part_num = part1;
    upload_part_copy_params1->range_start = range_start1;
    upload_part_copy_params1->range_end = range_end1;

    headers = aos_table_make(p, 0);
    resp_headers = aos_table_make(p, 5);
    s = oss_upload_part_copy(options, upload_part_copy_params1, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //upload part copy 2
    range_end2 = get_file_size(local_filename) - 1;
    upload_part_copy_params2 = oss_create_upload_part_copy_params(p);
    aos_str_set(&upload_part_copy_params2->source_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params2->source_object, source_object_name);
    aos_str_set(&upload_part_copy_params2->dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&upload_part_copy_params2->dest_object, dest_object_name);
    aos_str_set(&upload_part_copy_params2->upload_id, upload_id.data);
    upload_part_copy_params2->part_num = part2;
    upload_part_copy_params2->range_start = range_start2;
    upload_part_copy_params2->range_end = range_end2;

    headers = aos_table_make(p, 0);
    resp_headers = aos_table_make(p, 5);
    s = oss_upload_part_copy(options, upload_part_copy_params2, headers, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //list part
    list_part_resp_headers = aos_table_make(p, 5);
    list_upload_part_params = oss_create_list_upload_part_params(p);
    list_upload_part_params->max_ret = 10;
    aos_list_init(&complete_part_list);
        
    aos_str_set(&dest_bucket, TEST_BUCKET_NAME);
    aos_str_set(&dest_object, dest_object_name);
    s = oss_list_upload_part(options, &dest_bucket, &dest_object, &upload_id, 
        list_upload_part_params, &list_part_resp_headers);

    aos_list_for_each_entry(part_content, &list_upload_part_params->part_list, node) {
        complete_content = oss_create_complete_part_content(p);
        aos_str_set(&complete_content->part_number, part_content->part_number.data);
        aos_str_set(&complete_content->etag, part_content->etag.data);
        aos_list_add_tail(&complete_content->node, &complete_part_list);
    }
     
    //complete multipart
    complete_resp_headers = aos_table_make(p, 5);
    s = oss_complete_multipart_upload(options, &dest_bucket, &dest_object, &upload_id,
        &complete_part_list, &complete_resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //check upload copy part content equal to local file
    headers = aos_table_make(p, 0);
    resp_headers = aos_table_make(p, 5);
    aos_str_set(&download_file, download_filename);
    s = oss_get_object_to_file(options, &dest_bucket, &dest_object, headers, &download_file, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, get_file_size(local_filename), get_file_size(download_filename));
    printf("test_upload_part_copy ok\n");

    aos_pool_destroy(p);
}

CuSuite *test_oss_multipart()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_multipart_setup);
    SUITE_ADD_TEST(suite, test_init_abort_multipart_upload);
    SUITE_ADD_TEST(suite, test_list_multipart_upload);
    SUITE_ADD_TEST(suite, test_multipart_upload);
    SUITE_ADD_TEST(suite, test_multipart_upload_from_file);
    SUITE_ADD_TEST(suite, test_upload_part_copy);
    SUITE_ADD_TEST(suite, test_multipart_cleanup);

    return suite;
}
