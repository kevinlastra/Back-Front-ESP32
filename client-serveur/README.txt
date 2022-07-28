/etc/mosquitto.conf

-> listener 1883 0.0.0.0
-> allow_anonymous true


compile

-> gcc client.c -o client -pthread -lmosquitto -lsqlite3
