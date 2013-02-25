#!/bin/bash

#Create a CA
echo "##> Creating a CA for autosigning..."
/usr/lib/ssl/misc/CA.pl -newca

#generate RSA private key for Apache
echo "##> Generating RSA private key..."
openssl genrsa -des3 -out server.key 1024
# openssl genrsa -out ldap1.example.com.key 1024

#Create a (CSR) Certificate Signing Request with the previous private key
echo "##> Creating a Certificate Signing Request (CSR)..."
openssl req -new -key server.key -out server.csr

#Sign the certificate
echo "##> Sign the certificate..."
openssl ca -in server.csr -extensions v3_ca -out server.pem -days 9000
#rm server.csr

#disable passphrase
mv server.key server.key.orig
openssl rsa -in server.key.orig -out server.key

#Show some infos:
echo "#######################################################"
echo "The certificate: server.pem"
echo "The certificate signing request: server.csr"
echo "The private key (keep it in a safe place): server.key"
echo "The CA certificate: ./demoCA/private/cakey.pem and ./demoCA/private/cacert.pem"
echo "#######################################################"