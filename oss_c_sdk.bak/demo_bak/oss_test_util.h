#ifndef OSS_TEST_UTIL_H
#define OSS_TEST_UTIL_H

#include "aos_http_io.h"
#include "aos_string.h"
#include "aos_transport.h"
#include "oss_define.h"

OSS_CPP_START

void make_rand_string(aos_pool_t *p, int len, aos_string_t *data);

aos_buf_t *make_random_buf(aos_pool_t *p, int len);

void make_random_body(aos_pool_t *p, int count, aos_list_t *bc);

void init_test_config(oss_config_t *config, int is_oss_domain);

void init_test_request_options(oss_request_options_t *options, int is_oss_domain);

OSS_CPP_END

#endif
