bin_PROGRAMS=oss_c_sdk_sample
oss_c_sdk_sample_SOURCES=\
    CuTest.h\
    oss_test_util.h\
    oss_config.h\
    CuTest.c\
    oss_test_util.c\
    oss_config.c\
    test_oss_bucket.c\
    test_oss_object.c\
    test_oss_multipart.c\
    test_all.c

oss_c_sdk_sample_LDADD=\
    -loss_c_sdk\
    -laprutil-1\
    -lapr-1\
    -lcurl\
    -lxml2\
    -lm

oss_c_sdk_sample_LDFLAGS=\
    -L../\
    -L/usr/local/lib\
    -L/usr/local/apr/lib\
    -L/usr/local/apr-util/lib\
    -Wl,--rpath=/usr/local/lib

oss_c_sdk_sample_CFLAGS=\
    -I. -I../\
    -I/usr/local/include/\
    -I/usr/include/libxml2\
    -I/usr/local/apr/include/apr-1\
    -I/usr/local/apr-util/include/apr-1\
    -D_LARGEFILE64_SOURCE\
    -fPIC
