FROM nginx:latest

RUN rm /etc/nginx/conf.d/default.conf

COPY conf/nginx.conf.template /etc/nginx/conf.d

CMD /bin/sh -c "envsubst '\$PORT' < /etc/nginx/conf.d/nginx.conf.template > /etc/nginx/conf.d/nginx.conf" && nginx -g 'daemon off;'
