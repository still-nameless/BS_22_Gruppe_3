cmake_minimum_required(VERSION 3.7.2)
project(BS_22_Gruppe_3 C)

set(CMAKE_C_STANDARD 99)

add_executable(BS_22_Gruppe_3
        main.c
        server/server.c
        server/server.h
        key_val_store/key_val_store.c
        key_val_store/key_val_store.h
        helper_functions/helper_functions.c
        helper_functions/helper_functions.h)
