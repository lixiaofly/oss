SOURCE_FILES=\
    oss_test_util.h\
    oss_config.h\
    oss_test_util.c\
    oss_config.c\
    test_oss_object.c\

INCLUDES=\
    -I. -I..\
    -I/usr/local/include/\
	-I/root/oss/libxml-dest/include/libxml2\
	-I/root/oss/apr-dest/include/apr-1\
	-I/root/oss/apr-util-dest/include/apr-1\
	-I/root/oss/curl-7.45.0/include/\
	-I/root/oss/curl-dest/include/curl

CFLAGS=\
    -D_LARGEFILE64_SOURCE\
    -fPIC

LIBS_PATH=\
    -L../\
    -L/usr/local/lib\
	-L/usr/lib64\
	-L/root/oss/apr-dest/lib\
	-L/root/oss/apr-util-dest/lib\
	-L/root/oss/oss_c_sdk\
	-L/root/oss/libxml-dest/lib\
	-L/root/oss/curl-dest/lib\
    -Wl,--rpath=/usr/local/lib

LIBS=\
    -loss_c_sdk\
    -lxml2\
    -laprutil-1\
    -lapr-1\
    -lcurl\
    -lrt\
    -lm

all:
	$(CC) $(SOURCE_FILES) -s $(CFLAGS) -o oss_c_sdk_demo $(INCLUDES) $(LIBS_PATH) $(LIBS)

install: all
	@echo

clean:
	rm -f oss_c_sdk_demo
	rm -f *.o
	rm -f oss_test_get_object_to_file
	rm -f oss_test_object_by_url
	rm -f test_upload_part_copy_file_download

test: all
	./oss_c_sdk_demo
