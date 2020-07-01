# A collection of Qt examples

## StructSerialization
...deals with serializing [`TheStruct.h`] to Qt's build serialization format with `QDataStream`,
especially for custom types and `enum`s in [`TheStructSerialization.cpp`].

As additional test, [`StructSerializationTest`] provides a custom templated `char* toString()` for
`QCOMPARE` to pretty print in case of difference.

[`TheStruct.h`]: StructSerialization/TheStruct.h
[`TheStructSerialization.cpp`]: StructSerialization/TheStructSerialization.cpp
[`StructSerializationTest`]: StructSerialization/StructSerializationTest.cpp

## SslUsage
...shows how to implement a secure communication with Qt's `QSsl*` classes.

Although the classes still use the name **SSL** (_Secure Socket Layers_), the samples will focus on
**TLS** (_Transport Layer Security_) >= v1.2, as TLS v1.1 and below are deprecated from 2020 on.

For more information read [SslUsage/README.md](SslUsage/README.md).
