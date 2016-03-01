OSS C SDK provides an API for using OSS(Open Storage Service). It is intended to
be easy to use, comprehensive, and suitable for use from any C or C++ program.

#DEPENDENCE
OSS C SDK depends on third party libraries such as curl, apr, xml2.
Before you use OSS C sdk, please check you have installed the third party 
libraries. Here we refer third party libraries websites:

1. curl
website: http://curl.haxx.se
version: 7.42.1

2. apr
website: https://apr.apache.org
version: 1.5.2

3. apr-util
website: https://apr.apache.org
version: 1.5.4

4. xml2
website: http://xmlsoft.org
version: 2.9.1

#SETUP
OSS C SDK uses autoconf and automake to compile. compile_oss_sdk.sh is a tool 
which has basic setup steps. you can use compile_oss_sdk to get liboss_c_sdk.a

#NOTE
If you use OSS C SDK in embeded system, space may be considered. you could use 
the option CFLAGS='-Os' when you install the third party. Here we refer to 
some useful websites to minimize the space:

1. http://www.cokco.cn/thread-11777-1-1.html

2. http://curl.haxx.se/docs/install.html
