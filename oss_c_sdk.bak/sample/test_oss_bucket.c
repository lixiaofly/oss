#include "CuTest.h"
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_xml.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_test_util.h"

void test_bucket_setup(CuTest *tc)
{
    aos_pool_t *p;
    int is_oss_domain = 1;
    aos_status_t *s;
    oss_request_options_t *options;
    oss_acl_e oss_acl = OSS_ACL_PRIVATE;
    char *object_name1 = "oss_test_object1";
    char *object_name2 = "oss_test_object2";
    char *object_name3 = "oss_tmp1/";
    char *object_name4 = "oss_tmp2/";
    char *str = "test c oss sdk";
    aos_table_t *headers1;
    aos_table_t *headers2;
    aos_table_t *headers3;
    aos_table_t *headers4;

    //set log level, default AOS_LOG_WARN
    aos_log_set_level(AOS_LOG_WARN);

    //set log output, default stderr
    aos_log_set_output(NULL);

    //create test bucket
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);

    CuAssertIntEquals(tc, 200, s->code);

    //create test object
    headers1 = aos_table_make(p, 0);
    headers2 = aos_table_make(p, 0);
    headers3 = aos_table_make(p, 0);
    headers4 = aos_table_make(p, 0);
    create_test_object(options, TEST_BUCKET_NAME, object_name1, str, headers1);
    create_test_object(options, TEST_BUCKET_NAME, object_name2, str, headers2);
    create_test_object(options, TEST_BUCKET_NAME, object_name3, str, headers3);
    create_test_object(options, TEST_BUCKET_NAME, object_name4, str, headers4);

    aos_pool_destroy(p);
}

void test_bucket_cleanup(CuTest *tc)
{
    aos_pool_t *p;
    int is_oss_domain = 1;
    aos_string_t bucket;
    aos_status_t *s;
    oss_request_options_t *options;
    char *object_name1 = "oss_test_object1";
    char *object_name2 = "oss_test_object2";
    char *object_name3 = "oss_tmp1/";
    char *object_name4 = "oss_tmp2/";
    aos_table_t *resp_headers;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);

    //delete test object
    delete_test_object(options, TEST_BUCKET_NAME, object_name1);
    delete_test_object(options, TEST_BUCKET_NAME, object_name2);
    delete_test_object(options, TEST_BUCKET_NAME, object_name3);
    delete_test_object(options, TEST_BUCKET_NAME, object_name4);

    //delete test bucket
    resp_headers = aos_table_make(p, 5);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s= oss_delete_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code);

    aos_pool_destroy(p);
}

void test_create_bucket(CuTest *tc)
{
    aos_pool_t *p;
    int is_oss_domain = 1;
    aos_status_t *s;
    oss_request_options_t *options;
    oss_acl_e oss_acl;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    oss_acl = OSS_ACL_PRIVATE;

    //create the same bucket twice with same bucket acl
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, NULL, s->error_code);

    //create the same bucket with different bucket acl
    oss_acl = OSS_ACL_PUBLIC_READ;
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    CuAssertIntEquals(tc, 200, s->code);
    printf("test_create_bucket ok\n");

    aos_pool_destroy(p);
}

void test_delete_bucket(CuTest *tc)
{
    aos_pool_t *p;
    aos_status_t *s;
    aos_string_t bucket;
    oss_acl_e oss_acl;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *resp_headers;
    char *req_id;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    oss_acl = OSS_ACL_PUBLIC_READ;
    s = create_test_bucket(options, TEST_BUCKET_NAME, oss_acl);
    resp_headers = aos_table_make(p, 5);

    //delete bucket not empty
    s = oss_delete_bucket(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 409, s->code);
    CuAssertStrEquals(tc, "BucketNotEmpty", s->error_code);
    CuAssertTrue(tc, s->req_id != NULL);
    printf("test_delete_bucket ok\n");

    aos_pool_destroy(p);
}

void test_get_bucket_acl(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *resp_headers;
    aos_status_t *s;
    aos_string_t oss_acl;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    resp_headers = aos_table_make(p, 5);
    s = oss_get_bucket_acl(options, &bucket, &oss_acl, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertStrEquals(tc, "public-read", oss_acl.data);
    printf("test_get_bucket_acl ok\n");

    aos_pool_destroy(p);
}

void test_list_object(CuTest *tc)
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
    params->max_ret = 1;
    params->truncated = 0;
    aos_str_set(&params->prefix, "oss_test");
    resp_headers = aos_table_make(p, 5);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 1, params->truncated);
    CuAssertStrEquals(tc, "oss_test_object1", params->next_marker.data);

    aos_list_for_each_entry(content, &params->object_list, node) {
        ++size;
        key = apr_psprintf(p, "%.*s", content->key.len, content->key.data);
    }
    CuAssertIntEquals(tc, 1 ,size);
    CuAssertStrEquals(tc, "oss_test_object1", key);
    
    size = 0;
    aos_list_init(&params->object_list);
    aos_str_set(&params->marker, params->next_marker.data);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);
    aos_list_for_each_entry(content, &params->object_list, node) {
        ++size;
        key = apr_psprintf(p, "%.*s", content->key.len, content->key.data);
    }
    CuAssertIntEquals(tc, 1 ,size);
    CuAssertStrEquals(tc, "oss_test_object2", key);
    printf("test_list_object ok\n");

    aos_pool_destroy(p);
}

void test_list_object_with_delimiter(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    oss_request_options_t *options;
    int is_oss_domain = 1;
    aos_table_t *resp_headers;
    aos_status_t *s;
    oss_list_object_params_t *params;
    oss_list_object_common_prefix_t *common_prefix;
    int size = 0;
    char *prefix = NULL;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    params = oss_create_list_object_params(p);
    params->max_ret = 5;
    params->truncated = 0;
    aos_str_set(&params->delimiter, "/");
    resp_headers = aos_table_make(p, 5);
    aos_str_set(&bucket, TEST_BUCKET_NAME);
    s = oss_list_object(options, &bucket, params, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);
    CuAssertIntEquals(tc, 0, params->truncated);

    aos_list_for_each_entry(common_prefix, &params->common_prefix_list, node) {
        ++size;
        prefix = apr_psprintf(p, "%.*s", common_prefix->prefix.len, common_prefix->prefix.data);
        if (size == 1) {
            CuAssertStrEquals(tc, "oss_tmp1/", prefix);
        } else if(size == 2) {
            CuAssertStrEquals(tc, "oss_tmp2/", prefix);
        }
    }
    CuAssertIntEquals(tc, 2, size);
    printf("test_list_object_with_delimiter ok\n");

    aos_pool_destroy(p);
}

void test_lifecycle(CuTest *tc)
{
    aos_pool_t *p;
    aos_string_t bucket;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *resp_headers;
    aos_status_t *s;
    aos_list_t lifecycle_rule_list;
    oss_lifecycle_rule_content_t *invalid_rule_content;    
    oss_lifecycle_rule_content_t *rule_content;
    oss_lifecycle_rule_content_t *rule_content1;
    oss_lifecycle_rule_content_t *rule_content2;
    int size = 0;
    char *rule_id;
    char *prefix;
    char *status;
    int days = INT_MAX;
    char* date = "";

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, TEST_BUCKET_NAME);

    //put invalid lifecycle rule
    resp_headers = aos_table_make(p, 5);
    aos_list_init(&lifecycle_rule_list);
    invalid_rule_content = oss_create_lifecycle_rule_content(p);
    aos_str_set(&invalid_rule_content->id, "");
    aos_str_set(&invalid_rule_content->prefix, "pre");
    aos_list_add_tail(&invalid_rule_content->node, &lifecycle_rule_list);
    s = oss_put_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, &resp_headers);
    CuAssertIntEquals(tc, 400, s->code);

    //put lifecycle
    resp_headers = aos_table_make(p, 5); 
    aos_list_init(&lifecycle_rule_list);
    rule_content1 = oss_create_lifecycle_rule_content(p);
    aos_str_set(&rule_content1->id, "1");
    aos_str_set(&rule_content1->prefix, "pre1");
    aos_str_set(&rule_content1->status, "Enabled");
    rule_content1->days = 1;
    rule_content2 = oss_create_lifecycle_rule_content(p);
    aos_str_set(&rule_content2->id, "2");
    aos_str_set(&rule_content2->prefix, "pre2");
    aos_str_set(&rule_content2->status, "Enabled");
    aos_str_set(&rule_content2->date, "2022-10-11T00:00:00.000Z");
    aos_list_add_tail(&rule_content1->node, &lifecycle_rule_list);
    aos_list_add_tail(&rule_content2->node, &lifecycle_rule_list);

    s = oss_put_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    //get lifecycle
    aos_list_init(&lifecycle_rule_list);
    resp_headers = aos_table_make(p, 5);
    s = oss_get_bucket_lifecycle(options, &bucket, &lifecycle_rule_list, &resp_headers);
    CuAssertIntEquals(tc, 200, s->code);

    aos_list_for_each_entry(rule_content, &lifecycle_rule_list, node) {
        if (size == 0) {
            rule_id = apr_psprintf(p, "%.*s", rule_content->id.len, rule_content->id.data);
            CuAssertStrEquals(tc, "1", rule_id);
            prefix = apr_psprintf(p, "%.*s", rule_content->prefix.len, rule_content->prefix.data);
            CuAssertStrEquals(tc, "pre1", prefix);
            date = apr_psprintf(p, "%.*s", rule_content->date.len, rule_content->date.data);
            CuAssertStrEquals(tc, "", date);
            status = apr_psprintf(p, "%.*s", rule_content->status.len, rule_content->status.data);
            CuAssertStrEquals(tc, "Enabled", status);
            days = rule_content->days;
            CuAssertIntEquals(tc, 1, days);
        }
        else if (size == 1){
            rule_id = apr_psprintf(p, "%.*s", rule_content->id.len, rule_content->id.data);
            CuAssertStrEquals(tc, "2", rule_id);
            prefix = apr_psprintf(p, "%.*s", rule_content->prefix.len, rule_content->prefix.data);
            CuAssertStrEquals(tc, "pre2", prefix);
            date = apr_psprintf(p, "%.*s", rule_content->date.len, rule_content->date.data);
            CuAssertStrEquals(tc, "2022-10-11T00:00:00.000Z", date);
            status = apr_psprintf(p, "%.*s", rule_content->status.len, rule_content->status.data);
            CuAssertStrEquals(tc, "Enabled", status);
            days = rule_content->days;
            CuAssertIntEquals(tc, INT_MAX, days);
        }
        ++size;
    }
    CuAssertIntEquals(tc, 2 ,size);

    //delete lifecycle
    resp_headers = aos_table_make(p, 5);
    s = oss_delete_bucket_lifecycle(options, &bucket, &resp_headers);
    CuAssertIntEquals(tc, 204, s->code); 
    printf("test_lifecycle ok\n");

    aos_pool_destroy(p);
}

CuSuite *test_oss_bucket()
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, test_bucket_setup);
    SUITE_ADD_TEST(suite, test_create_bucket);
    SUITE_ADD_TEST(suite, test_get_bucket_acl);
    SUITE_ADD_TEST(suite, test_list_object);
    SUITE_ADD_TEST(suite, test_list_object_with_delimiter);
    SUITE_ADD_TEST(suite, test_delete_bucket);
    SUITE_ADD_TEST(suite, test_lifecycle);
    SUITE_ADD_TEST(suite, test_bucket_cleanup);

    return suite;
}
