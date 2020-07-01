# A collection of Qt examples

## StructSerialization
...deals with serializing [`TheStruct.h`] to Qt's build serialization format with `QDataStream`,
especially for custom types and `enum`s in [`TheStructSerialization.cpp`].

As additional test, [`StructSerializationTest`] provides a custom templated `char* toString()` for
`QCOMPARE` to pretty print in case of difference.

[`TheStruct.h`]: StructSerialization/TheStruct.h
[`TheStructSerialization.cpp`]: StructSerialization/TheStructSerialization.cpp
[`StructSerializationTest`]: StructSerialization/StructSerializationTest.cpp
