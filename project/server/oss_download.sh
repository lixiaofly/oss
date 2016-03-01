#!/bin/bash

oss_bucket="ums-autelan-com"

log_path="/tmp/oss/post/"
host="https://120.26.47.127:8282/"
url="UMS/UpdateLog.do"

downLoadPath="/tmp/oss/download"
postPath="/tmp/oss/post"

main(){
	#/root/oss/project/oss_server download ums-autelan-com oss ${downLoadPath} 1 > /root/oss/project/oss_server_log/server.log
	/root/oss/project/oss_server download "${oss_bucket}" oss ${downLoadPath} 1

	mv ${downLoadPath}/* ${postPath}/ 


	#/root/oss/project/post_to_server.sh "${log_path}" "${host}" "${url}" 
	return 
}

main "$@"

