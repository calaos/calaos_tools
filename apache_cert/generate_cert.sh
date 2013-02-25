#!/bin/bash


openssl req -new -outform PEM -config cert.cnf -out server.pem -newkey rsa:2048 -nodes -keyout server.key -keyform PEM -days 9999 -x509
#cat server.key server.cert > server.pem
