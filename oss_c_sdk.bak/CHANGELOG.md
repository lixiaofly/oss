v0.0.1
2015-05-28
    基于OSS API文档，提供OSS bucket、object以及multipart相关的常见操作API
    提供基于CuTest的sample

v0.0.2
2015-06-10
    增加oss_upload_part_copy，支持Upload Part Copy方式拷贝
    增加sts服务临时授权方式访问OSS

v0.0.3
2015-07-08
    增加oss_append_object_from_buffer接口，支持追加上传buffer中的内容到object
    增加oss_append_object_from_file接口，支持追加上传文件中的内容到object

v0.0.4
2015-07-27
    增加生命周期相关的接口oss_put_bucket_lifecycle、oss_get_bucket_lifecycle以及oss_delete_bucket_lifecycle
    OSS C SDK支持长连接，默认使用连接池支持keep alive功能
    oss_list_object增加子目录的输出

v0.0.5
2015-09-10
    调整OSS C SDK获取GMT时间的方式，解决LOCALE变化可能导致签名出错的问题
    aos_status_t结构体增加req_id字段，方便定位请求出错问题

v0.0.6
2015-10-29
    OSS C SDK签名时请求头支持x-oss-date，允许用户指定签名时间，解决系统时间偏差导致签名出错的问题
    OSS C SDK支持CNAME方式访问OSS，CNAME方式请求时指定is_oss_domain值为0
    新增OSS C SDK demo,提供简单的接口调用示例，方便用户快速入门
    OSS C SDK sample示例中去除对utf8第三方库的依赖
