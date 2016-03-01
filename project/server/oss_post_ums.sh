#!/bin/bash

log_path="/tmp/oss/post/"
host="http://121.43.231.237:8181/"
url="UMS/UpdateLog.do"

main(){
	/root/oss/project/server/post_to_server.sh "${log_path}" "${host}" "${url}" 
	return 
}

main "$@"

