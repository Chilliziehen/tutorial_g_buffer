cmake_minimum_required(VERSION 4.0)
project(tutorial_g_buffer)
message("C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_LINKER = ${CMAKE_LINKER}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LOGS_DIR ${CMAKE_SOURCE_DIR}/logs)
set(LOG_FILENAME "global.log")

set(CXX_LIB_ROOT ${CMAKE_SOURCE_DIR}/src/cxx_lib)
set(CXX_MAIN_ROOT ${CMAKE_SOURCE_DIR}/src/cxx_main)

set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)

set(LIBRARY_ROOT ${CMAKE_SOURCE_DIR}/libs)
set(GLEW_ROOT ${LIBRARY_ROOT}/external/glew)
set(GLFW_ROOT ${LIBRARY_ROOT}/external/glfw)
set(GLM_ROOT ${LIBRARY_ROOT}/external/glm)
set(STB_ROOT ${LIBRARY_ROOT}/stb)
set(STB_IMAGE_ROOT ${STB_ROOT}/stb_image)
set(TINYOBJLOADER_ROOT ${LIBRARY_ROOT}/external/tinyobj)
set(GLEW_INCLUDE_DIR ${GLEW_ROOT}/include)
set(GLFW_INCLUDE_DIR ${GLFW_ROOT}/include)
set(GLM_INCLUDE_DIR ${GLM_ROOT})
set(STB_IMAGE_INCLUDE_DIR ${STB_IMAGE_ROOT})
set(TINYOBJLOADER_INCLUDE_DIR ${TINYOBJLOADER_ROOT})

set(GLFW_LIB_DIR     "${GLFW_ROOT}/lib-vc2022")
set(GLEW_LIB_DIR     "${GLEW_ROOT}/lib")

#assimp library settings
#assimp官方没有提供二进制文件，需要手动编译后放到指定目录
#以下为assimp编译完成后的目录设置
set(ASSIMP_FALLBACK_DIR "${LIBRARY_ROOT}/external/assimp")
set(ASSIMP_DIR          "${ASSIMP_FALLBACK_DIR}")
set(ASSIMP_INCLUDE_DIR "${ASSIMP_DIR}/include")
set(ASSIMP_LIB_DIR     "${ASSIMP_DIR}/lib")
set(ASSIMP_BIN_DIR     "${ASSIMP_DIR}/bin")
set(LIB_INCLUDE_DIRS
    ${GLEW_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${STB_IMAGE_INCLUDE_DIR}
    ${TINYOBJLOADER_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIR}
)

add_subdirectory(src/cxx_main)
add_subdirectory(src/cxx_lib)
add_dependencies(tutorial_g_buffer cxx_library)
# --- Optional: build Assimp from source and install into libs/assimp ---
# This target is not built by default; invoke explicitly:
#   cmake --build <build_dir> --target compile_assimp --config Release
set(ASSIMP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/assimp")
set(ASSIMP_EXTERNAL_BUILD_DIR "${CMAKE_BINARY_DIR}/thirdparty-assimp-build")
set(ASSIMP_EXTERNAL_INSTALL_DIR "${ASSIMP_FALLBACK_DIR}")

if (EXISTS "${ASSIMP_SOURCE_DIR}/CMakeLists.txt")
    if (CMAKE_CONFIGURATION_TYPES)
        # Multi-config generators (Visual Studio, Xcode)
        add_custom_target(compile_assimp #EXCLUDE_FROM_ALL
                COMMAND ${CMAKE_COMMAND} -E make_directory "${ASSIMP_EXTERNAL_BUILD_DIR}"
                COMMAND ${CMAKE_COMMAND}
                -S "${ASSIMP_SOURCE_DIR}"
                -B "${ASSIMP_EXTERNAL_BUILD_DIR}"
                -G "${CMAKE_GENERATOR}"
                -A "${CMAKE_GENERATOR_PLATFORM}"
                -T "${CMAKE_GENERATOR_TOOLSET}"
                -DBUILD_SHARED_LIBS=ON
                -DASSIMP_BUILD_TESTS=OFF
                -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
                -DASSIMP_INSTALL=ON
                -DCMAKE_INSTALL_PREFIX="${ASSIMP_EXTERNAL_INSTALL_DIR}"
                COMMAND ${CMAKE_COMMAND} --build "${ASSIMP_EXTERNAL_BUILD_DIR}" --config $<CONFIG>
                COMMAND ${CMAKE_COMMAND} --install "${ASSIMP_EXTERNAL_BUILD_DIR}" --config $<CONFIG>
                BYPRODUCTS
                "${ASSIMP_EXTERNAL_INSTALL_DIR}/include/assimp/Importer.hpp"
                COMMENT "Building Assimp1 (shared) from thirdparty/assimp and installing into libs/assimp"
        )
    else()
        # Single-config generators (Ninja, Makefiles)
        add_custom_target(compile_assimp #EXCLUDE_FROM_ALL
                COMMAND ${CMAKE_COMMAND} -E make_directory "${ASSIMP_EXTERNAL_BUILD_DIR}"
                COMMAND ${CMAKE_COMMAND}
                -S "${ASSIMP_SOURCE_DIR}"
                -B "${ASSIMP_EXTERNAL_BUILD_DIR}"
                -G "${CMAKE_GENERATOR}"
                -DCMAKE_MAKE_PROGRAM="${CMAKE_MAKE_PROGRAM}"
                -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}"
                -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DBUILD_SHARED_LIBS=ON
                -DASSIMP_BUILD_TESTS=OFF
                -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
                -DASSIMP_INSTALL=ON
                -DCMAKE_INSTALL_PREFIX="${ASSIMP_EXTERNAL_INSTALL_DIR}"
                COMMAND ${CMAKE_COMMAND} --build "${ASSIMP_EXTERNAL_BUILD_DIR}"
                COMMAND ${CMAKE_COMMAND} --install "${ASSIMP_EXTERNAL_BUILD_DIR}"
                BYPRODUCTS
                "${ASSIMP_EXTERNAL_INSTALL_DIR}/include/assimp/Importer.hpp"
                COMMENT "Building Assimp2 (shared) from thirdparty/assimp and installing into libs/assimp"
        )
    endif()
endif()


