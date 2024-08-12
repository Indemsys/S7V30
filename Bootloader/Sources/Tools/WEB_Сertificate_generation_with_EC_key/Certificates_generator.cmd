rem Create CA Key
openssl ecparam -name secp256r1 -genkey -noout -out ca.key

rem Create selfsigned CA Certificate
openssl req -config CA.conf -new -x509 -sha256 -key ca.key  -days 3650 -out ca.crt

rem Create server Key 
openssl ecparam -name secp256r1 -genkey -noout -out srv.key

rem Create Certificate Request from CA certificate
openssl req -new -config Server.conf -key srv.key -out srv.csr 

rem Verify and Sign the Certificate Request
openssl x509 -req -in srv.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out srv.crt -days 3650 -sha256 -extfile Server.conf -extensions x509_ext

rem Convert PEM to DER
openssl x509 -in CA.crt -out CA.der -outform DER
openssl x509 -in srv.crt -out srv.der -outform DER
openssl ec -inform pem -in ca.key -outform der -out ca_key.der
openssl ec -inform pem -in srv.key -outform der -out srv_key.der

copy srv.key .\Output_for_Server\srv.key
copy srv.crt .\Output_for_Server\srv.crt
copy srv.der .\Output_for_Server\srv.der
copy ca.der .\Output_for_Server\ca.der
copy srv_key.der .\Output_for_Server\srv_key.der
copy ca_key.der .\Output_for_Server\ca_key.der
copy CA.crt  .\Output_for_Client\CA.crt
del  srv.key
del  srv.crt
del  srv.der
del  ca.der
del  srv_key.der
del  ca_key.der
del  ca.key
del  CA.crt 
del  srv.csr
del  ca.srl