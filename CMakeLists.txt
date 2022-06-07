cmake_minimum_required(VERSION 3.7.2)
project(BS_22_Gruppe_3 C)

set(CMAKE_C_STANDARD 99)

include_directories(server)
include_directories(key_val_store)
include_directories(helper_functions)

add_executable(BS_22_Gruppe_3
        main.c
        helper_functions/helper_functions.c
        helper_functions/helper_functions.h
        key_val_store/key_val_store.c
        key_val_store/key_val_store.h
        server/server.h
        server/server.c
        )
