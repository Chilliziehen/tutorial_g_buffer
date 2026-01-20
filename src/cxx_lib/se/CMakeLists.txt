#set(GENERATED_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/generated")
add_library(seTools
        Singleton.h
        GlobalLogger.cpp
        GlobalLogger.h
        IdentificationGenerator.cpp
        IdentificationGenerator.h
        Interface_Identification.h)

target_include_directories(seTools PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GENERATED_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/libs/external/cli
        ${LIB_INCLUDE_DIRS}
)
