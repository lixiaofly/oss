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

#define DSTFILEPATH "/tmp/oss/"

int
get_object_to_file (char *bucketName, char *ossFile, char *savePath)
{
  aos_pool_t *p;
  aos_string_t bucket;
  char *object_name = ossFile;
  aos_string_t object;
  char *filename = savePath;
  //char *source_filename = "./oss_lixiao_test";
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
  s =
    oss_get_object_to_file (options, &bucket, &object, headers, &file,
			    &resp_headers);
  assert (200 == s->code);
  printf("Saved %s to %s\n", ossFile, savePath);
  //CuAssertIntEquals(tc, get_file_size(source_filename), get_file_size(filename));
  //printf("test_get_object_to_file size(%lu) == size(%lu) ok\n",get_file_size(source_filename), get_file_size(filename));

  aos_pool_destroy (p);

  return 0;
}


int
scan_oss (char *bucketName)
{
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

  aos_str_set (&params->prefix, "oss_test");
  resp_headers = aos_table_make (p, 5);
  aos_str_set (&bucket, bucketName);

  do {
      s = oss_list_object (options, &bucket, params, &resp_headers);
      assert (200 == s->code);
      printf ("params->next_marker.data=[%s] marker.data=[%s]\n",
	      params->next_marker.data, params->marker.data);
      printf ("truncated=%d\n", params->truncated);

      aos_list_for_each_entry (content, &params->object_list, node)
      {
	++size;
	key = apr_psprintf (p, "%.*s", content->key.len, content->key.data);
	printf ("key.data=%s\n", content->key.data);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s%s", DSTFILEPATH, key);
	get_object_to_file (bucketName, key, buf);
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


int
main (int argc, char *argv[])
{
  char bucket_1[64];

  if (aos_http_io_initialize ("oss_test", 0) != AOSE_OK)
    {
      exit (1);
    }
  strcpy (bucket_1, TEST_BUCKET_NAME);
  printf ("bucket=%s\n", bucket_1);
  scan_oss (bucket_1);

  aos_http_io_deinitialize ();
  return 0;
}
