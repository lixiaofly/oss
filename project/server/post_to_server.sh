#!/bin/bash

#LogPath="/tmp/oss/post/"
#host="https://120.26.47.127:8282/"
#url="UMS/UpdateLog.do"

LogPath="$1"
host="$2"
url="$3"


post_to_ums() {
	local json=$1
	json=`echo ${json} | jq -c .`
	local result=$(curl -k -cert /root/oss/project/server.cer -H "Content-type: application/json" -X POST  -d "${json}" "${host}""${url}" "-s")	
	
	echo "result=${result}"

	local result_code=`echo "${result}" | jq -c ".code"`

	case "${result_code}" in
                0 )
                        echo "Update success!" 
                        echo "0"
                        ;;
                * )
                        echo "Update error: ${result_code}" 
                        echo "${result_code}"
                        ;;
        esac
        return 0
}

update_log_to_ums() {

	local file=$1
	local ret

	while read LINE
	do
       		echo "[$LINE]" 
		post_to_ums "${LINE}" 
		
	done < ${file} 
	rm ${file}
	return 0
}

main() {

	for file in `ls "${LogPath}"`
	do
		echo "${file}"

 		if [ -f ${LogPath}$file ]; then
   			update_log_to_ums ${LogPath}${file}

 		fi
	done
	return 0

}
main "$@" 
