#!/bin/bash

log_path="/tmp/oss/post/"
host="https://120.26.47.127:8282/"
url="UMS/UpdateLog.do"

main(){
	/root/oss/project/post_to_server.sh "${log_path}" "${host}" "${url}" 
	return 
}

main "$@"

