#this is CMakeLists.txt for Lchika_client
cmake_minimum_required(VERSION 2.8.11)

set(AZUREIOT_INC_FOLDER ".." "/usr/include/azureiot" "/usr/include/azureiot/inc")

include_directories(${AZUREIOT_INC_FOLDER})

set(Lchika_client_c_files
    ../Lchika_client.c
)

set(Lchika_client_h_files
    ../Lchika_client.h
)

add_executable(Lchika_client ${Lchika_client_c_files} ${Lchika_client_h_files})

target_link_libraries(Lchika_client
    iothub_client
    iothub_client_mqtt_transport
    aziotsharedutil
    umqtt
    pthread
    curl
    ssl
    crypto
    m
    serializer
)
