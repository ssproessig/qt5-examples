add_executable(SslUsage.RawEchoServer
    Server.cpp
)
target_compile_options(SslUsage.RawEchoServer PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(SslUsage.RawEchoServer Qt::Core Qt::Network)

add_executable(SslUsage.RawEchoClient
    Client.cpp
)
target_compile_options(SslUsage.RawEchoClient PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(SslUsage.RawEchoClient Qt::Core Qt::Network)
