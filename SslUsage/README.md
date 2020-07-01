# Qt TLS usage
## Raw TCP server and client
A raw TCP server and client w/o SSL can be found in [SslUsage/RawEchoService](RawEchoService). The
example uses a command-line parser to configure `interface` and `port` to listen to and can also be
connected using any TCP client, e.g. `netcat`. ![raw_client_to_server](raw_client_to_server.png "Raw
TCP client to server communication")

### Server
The server uses the signals
- `&QTcpServer::newConnection()` to acquire the `QTcpSocket*` of an incoming client connection and
- consecutively `&QTcpSocket::readyRead()` on the socket when the TCP stack signals new data is
  available.

Shortened example
```c++
QTcpServer srv;

QObject::connect(&srv, &QTcpServer::newConnection, [&srv]() {
    auto* const client = srv.nextPendingConnection();

    QObject::connect(client, &QTcpSocket::readyRead, [client]() {
        auto const& data = client->readAll();
        client->write(data);
    });
});

srv.listen(QHostAddress::Any, 9876);
```

### Client
The client uses the signals
- `&QTcpSocket::connected()` emitted when the TCP connection to the server was established and
- `&QTcpSocket::readyRead()` as well to read new data received from the server.

Shortened example
```c++
QTcpSocket s;

QObject::connect(&s, &QTcpSocket::connected, [&s]() {
    s.write("hello from client!");
});

QObject::connect(&s, &QTcpSocket::readyRead, [&s]() {
    auto const data = s.readAll();
    // do something with "data"
});

s.connectToHost("localhost", 9876);
```

## Securing the communication
The previous example has several problems with security:

1. data is transported unencrypted as _plain text_, hence everybody on the machines and on the route
   between client and server could read the data
2. data could easily be altered on the route, without any partner knowing
3. client does not know if it speaks to the correct "server" or a man-in-the-middle attacker
4. server does not know if the actual client speaks to it or not

The standard for solving this problem is
[**TLS** (_Transport Layer Security_)](https://en.wikipedia.org/wiki/Transport_Layer_Security).

We will concentrate on the latest version (as of 2020-07) TLS v1.3 here.

### Finding a solution for 1.
Obviously, the solution for 1. is to use TLS, as this will introduce symmetric encryption between
the communication peers. Anyway, all peers must somehow agree on a common secret when performing the
_Client Hello_, simplest solution would be the so-called _pre-shared key_ **PSK**.

![tls_client_server_with_psk](tls_client_server_with_psk.png)

The problems with this solution are
- you need to securely roll-out the PSK to all clients
- you need to securely store the PSK on the client
- you only have **one** PSK for the whole set-up, so every client will use the same PSK
- if it is compromised for one client (e.g. laptop stolen, phishing attack), it is compromised for
  all and the roll-out needs to start over again

Overall, this is not a feasible solution, as it also does not solve
- identification of server and client

Nevertheless, Qt supports `QSslPreSharedKeyAuthenticator` for this

```c++
QSslSocket s;
connect(s, &QSslSocket::preSharedKeyAuthenticationRequired, this, [](QSslPreSharedKeyAuthenticator* authenticator) {
    authenticator->setIdentity("client");

    // get thePSK from somewhere, e.g. derive it from authenticator->identityHint()...

    authenticator->setPreSharedKey(thePSK);
});
```

But, because of the disadvantages already mentioned above, we SHALL rely on a key exchange that
works asymmetric and over an open network. We advocate for
[Diffie-Hellman](https://en.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange) here.

### Finding a solution for 2.
TLS brings **DSA** (_Digital Signature Algorithm_) for that. We'll see this in the sample below.

### Find a solution for 3. and 4.
We need to find a way that the server and client can identify themselves. This is done using
so-called **certificates**. Follow-up with
[Wikipedia](https://en.wikipedia.org/wiki/Public_key_certificate) for more details.

![tls_client_server_certificates](tls_client_server_certificates.png)

So, basically the server presents its (orange) certificate to the client while handshaking, the
client presents it (green) certificate to the server.

Both can have a fixed list of certificates they except and hence we solved the problem of knowing
who we talk to. But: this would again not scale if would need to

Hence, client and server need to agree on a shared authority that both trust, a so-called
_Certificate Authority_ **CA**. This CA digitally signs those certificates and hence both, client
and server, only need to check if a certificate presented is valid. Certificates can easily be added
(new clients) or revoked (upon compromise).

![tls_client_server_certificate_with_ca](tls_client_server_certificate_with_ca.png)

### Putting everything together
In order to make this work with Qt we need to

1. become a _Certificate Authority_ and
   1. create a CA certificate
   2. create a server certificate and sign it with the CA
   3. create a client certificate and sign it with the CA
   4. create another CA certificate to test the bad case (presented certificates not signed by this
      CA certificate)
2. use `QSslSocket` on top of `QTcpServer` in the server, `QSslSocket` in the client
3. setup the connection and implement all checks as stated above

### Creating certificates
We will use the **OpenSSL** binary to create them (`1.1.1g` is latest as of writing), e.g.
[from here](https://bintray.com/vszakats/generic/openssl):

```shell
> openssl version

OpenSSL 1.1.1g  21 Apr 2020
```

#### for the CA
- create the **CA's private key** (in the example I'm using `CA_password` as pass phrase)
```
>openssl genrsa -aes256 -out CA.key 4096

Generating RSA private key, 4096 bit long modulus (2 primes)
.......................++++
................++++
e is 65537 (0x010001)
Enter pass phrase for CA.key:
Verifying - Enter pass phrase for CA.key:
```

- create the **CA certificate**

```
g:\Code\github\qt5-examples>openssl req -x509 -new -nodes -key CA.key -sha256 -days 365 -out CA.pem

Enter pass phrase for CA.key:
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:DE
State or Province Name (full name) [Some-State]:Berlin
Locality Name (eg, city) []:Berlin
Organization Name (eg, company) [Internet Widgits Pty Ltd]:github.com/ssproessig
Organizational Unit Name (eg, section) []:
Common Name (e.g. server FQDN or YOUR name) []:CA
Email Address []:ssproessig@gmail.com
```

- repeat those steps to create `Other_CA.key` and `Other_CA.pem`

#### for the server
-  create a private key for the server (using `server` as pass phrase this time)
```
>openssl genrsa -aes256 -out server.key 4096

Generating RSA private key, 4096 bit long modulus (2 primes)
............................................................................++++...++++
e is 65537 (0x010001)
Enter pass phrase for server.key:
Verifying - Enter pass phrase for server.key:
```

- create a _certificate request_ for the server
```
>openssl req -key server.key -new -out server.req

Enter pass phrase for server.key:
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:DE
State or Province Name (full name) [Some-State]:Berlin
Locality Name (eg, city) []:Berlin
Organization Name (eg, company) [Internet Widgits Pty Ltd]:github.com/ssproessig
Organizational Unit Name (eg, section) []:
Common Name (e.g. server FQDN or YOUR name) []:server
Email Address []:

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
An optional company name []:
```

- use the CA to create a signed certificate for the server's certificate request
```
> openssl x509 -req -in server.req -CA CA.pem -CAkey CA.key -CAserial CA_serial -CAcreateserial -out server.pem

Signature ok subject=C = DE, ST = Berlin, L = Berlin, O = github.com/ssproessig, CN = > server Getting CA Private Key Enter pass phrase for CA.key:

```

- have a look at the certificate
```
g:\Code\github\qt5-examples\SslUsage\Credentials>openssl x509 -in server.pem -text -noout
Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number:
            5c:57:ab:ab:07:39:b2:23:37:f7:55:36:a8:9c:2c:50:0a:7a:67:4c
        Signature Algorithm: sha256WithRSAEncryption
        Issuer: C = DE, ST = Berlin, L = Berlin, O = github.com/ssproessig, CN = CA, emailAddress = ssproessig@gmail.com
        Validity
            Not Before: Jul  1 19:20:05 2020 GMT
            Not After : Jul 31 19:20:05 2020 GMT
        Subject: C = DE, ST = Berlin, L = Berlin, O = github.com/ssproessig, CN = server, emailAddress = ssproessig@gmail.com
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                RSA Public-Key: (4096 bit)
                Modulus:
....
                Exponent: 65537 (0x10001)
    Signature Algorithm: sha256WithRSAEncryption
 ...
```

#### for the first client
- repeat the server steps as above, but
  - use `client_01.key` as file name of the private key, use `client_01` as passphrase
  - use `client_01.req` as file name for certificate request, `client_01` as CN
  - use `client_01.pem` as certificate name - and omit `-CAcreateserial`, as the serial file already
    exists

#### for the other CA's client
- repeat the server steps as above, but
  - use `Other_client_01.key` as file name of the private key, use `client_01` as passphrase
  - use `Other_client_01.req` as file name for certificate request, `client_01` as CN
  - use `Other_client_01.pem` as certificate name - and omit `-CAcreateserial`, as the serial file
    already exists, and use the `Other_CA` to sign

### Caveats
- `openssl` may try to use a not existing `openssl.conf` and fail - use environment variable
  `OPENSSL_CONF` to point to the correct one ... and think about to things like _Country Name_,
  _Locality Name_ etc. there directly to avoid typing it over and over again
- Qt probably uses `openssl` as SSL/TLS plugin library - hence the applications will compile and
  run, but SSL will not work, unless `libcrypto-1_1-x64.dll` and `libssl-1_1-x64.dll` are on the
  `PATH` - hint: if the applications fail to load the `key` and `cert` at startup this is probably
  what's hapenning

### Putting it all together - SecureEchoService
TODO:

### More Information on SSL/TLS
- read the [excellent TLS v1.3 slides by Andy Brodie from the OWASP London 2018 summit][slides]

[slides]: https://owasp.org/www-chapter-london/assets/slides/OWASPLondon20180125_TLSv1.3_Andy_Brodie.pdf
