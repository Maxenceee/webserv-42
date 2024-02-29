#Nginx container

#init the base environement
FROM debian:latest

RUN apt update -y && apt upgrade -y
RUN apt install nginx curl lsof -y

#copy the nginx config file
COPY ./confs/nginx.conf /etc/nginx/nginx.conf

#set the working directory
WORKDIR /etc/nginx/

COPY ./public /public

EXPOSE 3000 3001 3002

#launch
CMD ["nginx", "-g", "daemon off;"]