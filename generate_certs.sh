#!/bin/bash

# Certs directory
CERTS_DIR=certs

# Create certs directory
mkdir -p $CERTS_DIR
mkdir -p data/certs

# Certs files
CA_CERT=$CERTS_DIR/ca.crt
CA_KEY=$CERTS_DIR/ca.key
SERVER_CERT=$CERTS_DIR/server.crt
SERVER_KEY=$CERTS_DIR/server.key
SERVER_CSR=$CERTS_DIR/server.csr
CLIENT_CERT=$CERTS_DIR/client.crt
CLIENT_KEY=$CERTS_DIR/client.key
CLIENT_CSR=$CERTS_DIR/client.csr

# Simple Certs files
SERVER_KEY_PEM=$CERTS_DIR/server_key.pem
SERVER_CERT_PEM=$CERTS_DIR/server_cert.pem

# Locale data
C=DO
ST=Santiago
L=Santiago
O=Example
OU=Example

# Days
DAYS=365

# Generate simple server key
openssl req -x509 -newkey rsa:2048 -keyout $SERVER_KEY_PEM -out $SERVER_CERT_PEM -days 365 -nodes -subj "/C=$C/ST=$ST/L=$L/O=$O/OU=$OU"

# Generate CA key
openssl genpkey -algorithm RSA -out $CA_KEY -pkeyopt rsa_keygen_bits:2048
openssl req -new -nodes -x509 -days $DAYS -key $CA_KEY -out $CA_CERT -subj "/C=$C/ST=$ST/L=$L/O=$O/OU=$OU"

# Generate server key
openssl genpkey -algorithm RSA -out $SERVER_KEY -pkeyopt rsa_keygen_bits:2048
openssl req -new -key $SERVER_KEY -out $SERVER_CSR -subj "/C=$C/ST=$ST/L=$L/O=$O/OU=$OU"
openssl x509 -req -days $DAYS -in $SERVER_CSR -CA $CA_CERT -CAkey $CA_KEY -CAcreateserial -out $SERVER_CERT -sha256

# Generate client (ESp32) key
openssl genpkey -algorithm RSA -out $CLIENT_KEY -pkeyopt rsa_keygen_bits:2048
openssl req -new -key $CLIENT_KEY -out $CLIENT_CSR -subj "/C=$C/ST=$ST/L=$L/O=$O/OU=$OU"
openssl x509 -req -days $DAYS -in $CLIENT_CSR -CA $CA_CERT -CAkey $CA_KEY -CAcreateserial -out $CLIENT_CERT -sha256

# Remove csr files
rm $SERVER_CSR $CLIENT_CSR 

# Convert to ESP32 format (PEM Without password)
openssl rsa -in $CLIENT_KEY -out data/certs/client.pem
openssl x509 -in $CLIENT_CERT -out data/certs/client_cert.pem 
openssl x509 -in $CA_CERT -out data/certs/ca.pem

# Show certs
echo "CA Cert:"
ls -l $CA_CERT