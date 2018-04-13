#this is CMakeLists.txt for Lchika_client

if(NOT ${use_mqtt})
    message(FATAL_ERROR "Lchika_client being generated without mqtt support")
endif()

compileAsC99()

set(Lchika_client_c_files
    Lchika_client.c
)

set(Lchika_client_h_files
)

include_directories(. "../../../serializer/inc")

add_executable(Lchika_client ${Lchika_client_c_files} ${Lchika_client_h_files})

target_link_libraries(Lchika_client  
    iothub_client_mqtt_transport
    iothub_client_amqp_transport
    uamqp
    iothub_client 
    serializer
)

