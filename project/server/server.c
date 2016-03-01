#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_util.h"

#define test_object_base() do {                                         \
        s = aos_status_create(options->pool);                           \
        aos_str_set(&bucket, bucket_name);                              \
        aos_str_set(&object, object_name);                              \
        resp_headers = aos_table_make(options->pool, 5);                \
    } while(0)

#define EXIT_PRINT_USEAGE(x) {printf("%s\n", useage); exit(x);}

int list_flag=0;
char useage[]="useage1: oss_server download [bucket] [prefix_objcet] [download_path] [delete_form_bucket[0/1]]\nuseage2: oss_server list [bucket] [prefix_objcet]";

char *get_time()
{
	static char buf[64];
	time_t timep;
	time (&timep);
	sprintf(buf, "%s",asctime(gmtime(&timep)));

	buf[strlen(buf)-1]=0;
	return buf;
}

int
get_object_to_file (char *bucketName, char *objectName, char *savePath)
{
  aos_pool_t *p;
  aos_string_t bucket;
  char *object_name = objectName;
  aos_string_t object;
  char *filename = savePath;
  aos_string_t file;
  oss_request_options_t *options;
  int is_oss_domain = 1;
  aos_table_t *headers;
  aos_table_t *resp_headers;
  aos_status_t *s;

  aos_pool_create (&p, NULL);
  options = oss_request_options_create (p);
  init_test_request_options (options, is_oss_domain);
  aos_str_set (&bucket, bucketName);
  aos_str_set (&object, object_name);
  aos_str_set (&file, filename);
  headers = aos_table_make (p, 5);
  resp_headers = aos_table_make (p, 5);

  //test get object to file
  s = oss_get_object_to_file (options, &bucket, &object, headers, &file,
			    &resp_headers);
  //printf("%s: Saved %s to %s\n", get_time(), objectName, savePath);

  aos_pool_destroy (p);
 
  if(s->code != 200){
	printf("%s oss_get_object_to_file failed s_code=%d\n", get_time(), s->code);
	return 1;
  }

  return 0;
}

int delete_object(char *bucketName, char* objectName)
{
    aos_pool_t *p;
    aos_string_t bucket;
    char *object_name = objectName;
    aos_string_t object;
    int is_oss_domain = 1;
    oss_request_options_t *options;
    aos_table_t *resp_headers;
    aos_status_t *s;

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_test_request_options(options, is_oss_domain);
    aos_str_set(&bucket, bucketName);
    aos_str_set(&object, object_name);
    resp_headers = aos_table_make(p, 5);
 
    //test delete object
    s = oss_delete_object(options, &bucket, &object, &resp_headers);
	
    aos_pool_destroy(p);
    if(204 != s->code){
	printf("%s delete object(%s) form bucket failed!\n", get_time(), objectName);
	return 1;
    }
    //printf("Deleted object:%s\n", objectName);	
    return 0;
}

int
scan_oss (char *bucketName, char* prefixObject, char* downloadPath, int doDelete)
{

  //[bucket] [prefix_objcet] [download_path] [delete_form_bucket] 
  int ret;
  char buf[128];
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

  aos_pool_create (&p, NULL);
  options = oss_request_options_create (p);
  init_test_request_options (options, is_oss_domain);

  params = oss_create_list_object_params (p);
  params->max_ret = 5;
  params->truncated = 0;

  aos_str_set (&params->prefix, prefixObject);
  resp_headers = aos_table_make (p, 5);
  aos_str_set (&bucket, bucketName);

  do {
      s = oss_list_object (options, &bucket, params, &resp_headers);
      if(200 != s->code){
	printf("%s Error:list object failed! code=%d\n", get_time(), s->code);
	return 1;
	}
      //printf ("truncated=%d\n", params->truncated);

      aos_list_for_each_entry (content, &params->object_list, node)
      {
	++size;
	key = apr_psprintf (p, "%.*s", content->key.len, content->key.data);
	//printf ("key.data=%s\n", content->key.data);
	    if(list_flag != 1){
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s%s%s", downloadPath,"/", key);
		ret = get_object_to_file (bucketName, key, buf);
		if((ret == 0) && (doDelete == 1)){
		delete_object(bucketName, key);
		}
	    }else{
		printf("obj:%s\n", key);
	    }
      }
      if(size == 0){
		if(list_flag == 1){
			printf("There list nothing in bucket!\n");
		}
       }

      //init params 
      if (params->truncated == 1)
	{
	  aos_list_init (&params->object_list);
	  aos_str_set (&params->marker, params->next_marker.data);
	}
    } while (params->truncated);

  aos_pool_destroy (p);
  return 0;
}

int list_object(char *bucket, char *preobj)
{
  if (aos_http_io_initialize ("oss_autelan", 0) != AOSE_OK)
  {
      exit (1);
  }

  scan_oss(bucket, preobj, NULL, 0);

  aos_http_io_deinitialize ();

	return 0;
}

int down_object(char *bucket, char *preobj, char *downpath, int dodel)
{
  if (aos_http_io_initialize ("oss_autelan", 0) != AOSE_OK)
  {
      exit (1);
  }

  scan_oss(bucket, preobj, downpath, dodel);

  aos_http_io_deinitialize ();
  return 0;

}
int
main (int argc, char *argv[])
{
  //useage: oss_server [download/list] [bucket] [prefix_objcet] [download_path] [delete_form_bucket[1/0]] 

   char* bucket=NULL, *preobj=NULL, *downpath=NULL;
   int dodel=0;	

	if(argc == 2){
		if(!strcmp(argv[1], "--help")){
			EXIT_PRINT_USEAGE(1)
		}
		EXIT_PRINT_USEAGE(1)
	}
	else if(argc == 4){
		if(strcmp(argv[1], "list")){
			EXIT_PRINT_USEAGE(1)
			
		}	
		bucket = argv[2];
		preobj = argv[3];
		
		list_flag=1;

		list_object(bucket, preobj);
		return 0;

	}
	else if(argc == 6){
		if(strcmp(argv[1], "download")){
			EXIT_PRINT_USEAGE(1);
		}
		bucket = argv[2];
		preobj = argv[3];	
		downpath = argv[4];	
		if(!strcmp(argv[5], "1")){
			dodel = 1;
  		 }else if(!strcmp(argv[5], "0")){
			dodel = 0;
   		}else{
			EXIT_PRINT_USEAGE(1)
   		}	
		down_object(bucket, preobj, downpath, dodel);
		return 0;
	}
	else{
		EXIT_PRINT_USEAGE(1)
	}

  return 0;
}
