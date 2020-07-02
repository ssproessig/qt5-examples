add_executable(SslUsage.SecureServer
    Server.cpp
    ServerCredentials.qrc
)
target_compile_options(SslUsage.SecureServer PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(SslUsage.SecureServer Qt::Core Qt::Network)

add_executable(SslUsage.SecureClient
    Client.cpp
    ClientCredentials.qrc
)
target_compile_options(SslUsage.SecureClient PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(SslUsage.SecureClient Qt::Core Qt::Network)
