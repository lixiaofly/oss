#!/bin/bash

oss_bucket="sensor-autelan-com"
prefix_obj=""

downLoadPath="/tmp/oss/download/"
postPath="/tmp/oss/post"

main(){
	#/root/oss/project/oss_server download ums-autelan-com oss ${downLoadPath} 1 > /root/oss/project/oss_server_log/server.log
	/root/oss/project/server/oss_server download "${oss_bucket}" "${prefix_obj}" "${downLoadPath}" 0 

	mv ${downLoadPath}/* ${postPath}/ 
	return 
}

main "$@"

