add_executable(StructSerializationTest WIN32
    StructSerializationTest.h
    StructSerializationTest.cpp
    TheStruct.h
    TheStructSerialization.cpp
)
target_compile_options(StructSerializationTest PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(StructSerializationTest Qt::Core Qt::Test)

add_test(NAME StructSerializationTest COMMAND StructSerializationTest)
