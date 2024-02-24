#!/bin/bash

port=3000

if [ "$#" -eq 1 ]; then
	port=$1
fi

for i in {1..1000}
do
	printf "Request $i\n"
	curl -iL -X GET http://localhost:$port/static/type2/obj.json
	printf "\n\n"
done

# for i in {1..100}
# do
# 	printf "Request $i\n"
# 	curl -iL -X GET http://localhost:$port/stylesheet/style.css
# 	printf "\n\n"
# done

# for i in {1..50}
# do
# 	printf "Request $i\n"
# 	curl -iL -X GET http://localhost:$port/static/type2/file.txt
# 	printf "\n\n"
# done

# for i in {1..10}
# do
# 	printf "Request $i\n"
# 	curl -iL -X GET http://localhost:$port/static/type3/server.log
# 	printf "\n\n"
# done

# for i in {1..5}
# do
# 	printf "Request $i\n"
# 	curl -iL -X GET http://localhost:$port/static/type2/48faec0cb53d.png
# 	printf "\n\n"
# done

# for i in {1..10}
# do
# 	printf "Request $i\n"
# 	curl -iL -X GET http://localhost:$port/static/type3/unmf.js
# 	printf "\n\n"
# done
