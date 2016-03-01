#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_util.h"

char useage[] = "oss_bucket [create/delete] [bucket]"; 

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

aos_status_t * do_create_bucket(const oss_request_options_t *options,
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

int create_bucket(char *bucketName)
{
	aos_pool_t *p;
    int is_oss_domain = 1;
    aos_status_t *s;
    oss_request_options_t *options;
    oss_acl_e oss_acl;

	//apr_initialize();
    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    oss_acl = OSS_ACL_PRIVATE;

    //create the same bucket twice with same bucket acl
    s = do_create_bucket(options, bucketName, oss_acl);

    if((200 != s->code) || (s->error_code != NULL)){
	printf("Error:create bucket(%s) failed!\n", bucketName);	
	return 1;
    }

    printf("create_bucket(%s) ok return code=%d\n", bucketName, s->code);

    aos_pool_destroy(p);
	return 0;
}

int delete_bucket(char *bucketName)
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
    aos_str_set(&bucket, bucketName);
    oss_acl = OSS_ACL_PRIVATE;
    s = create_test_bucket(options, bucketName, oss_acl);
    resp_headers = aos_table_make(p, 5);

    //delete bucket 
    s = oss_delete_bucket(options, &bucket, &resp_headers);
    if(204 != s->code){
    	aos_pool_destroy(p);
	return  1;
    }
    //CuAssertStrEquals(tc, "BucketNotEmpty", s->error_code);
    //CuAssertTrue(tc, s->req_id != NULL);
    printf("delete_bucket(%s) ok\n", bucketName);

    aos_pool_destroy(p);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	if(argc != 3){
		printf("%s\n", useage);
		return 1;		
	}

  	if (aos_http_io_initialize ("oss_autelan", 0) != AOSE_OK)
    	{
      		exit (1);
    	}
	if(!strcmp(argv[1], "create")){
		ret = create_bucket(argv[2]);
		if(ret != 0)
			goto out;
	}
	else if(!strcmp(argv[1], "delete")){
		ret = delete_bucket(argv[2]);
		if(ret != 0)
			goto out;
	}	
	else{
		printf("%s\n", useage);
		goto out;
	}

	aos_http_io_deinitialize ();
	return 0;
out:	
	aos_http_io_deinitialize ();
	return ret;
}
