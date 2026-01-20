set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config)
set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
set(WIN_SYS_LIBS
        opengl32.lib
        gdi32.lib
        user32.lib
        shell32.lib
        kernel32.lib
        winmm.lib
        ws2_32.lib
        bcrypt.lib
        ole32.lib
        uuid.lib
        advapi32.lib
)

set(GLSL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/glsl)

set(LINK_LIBS
        ${GLFW_LIB_DIR}/glfw3.lib
        ${GLEW_LIB_DIR}/glew32s.lib
        ${WIN_SYS_LIBS}
        ${ASSIMP_LIB_DIR}/assimp-vc143-mtd.lib
)

configure_file(
        ${CONFIG_DIR}/config_cxx_main.h.in
        ${GENERATED_INCLUDE_DIR}/config.h
)

set(COMMON_FILES common/light/LightManager.h
        common/shader/VertexShader.h
        common/light/LightManager.cpp
        common/shader/Shader.h
        common/shader/VertexShader.cpp
        common/program/Program.cpp
        common/shader/FragmentShader.h
        common/program/Program.h
        common/shader/FragmentShader.cpp
        common/light/Light.h
        common/light/Light.cpp
        common/camera/Camera.h
        common/camera/Camera.cpp)

set(CXX_LIB_INCLUDE_DIRS
        ${CXX_LIB_ROOT}/resources
        ${CXX_LIB_ROOT}/scene
        ${CXX_LIB_ROOT}/se
)

add_executable(forward_rendering forward_rendering.cpp
        ${COMMON_FILES}
        common/util/Utilities.cpp
        common/util/CLI_Color.h)
target_include_directories(forward_rendering PRIVATE
        ${LIB_INCLUDE_DIRS}
        config
        ${CXX_LIB_INCLUDE_DIRS}
)
target_link_libraries(
        forward_rendering PRIVATE
        cxx_library
)

add_executable(tutorial_g_buffer g_buffer_rendering.cpp
        ${COMMON_FILES}
        common/util/Utilities.cpp
        common/util/CLI_Color.h)

target_include_directories(tutorial_g_buffer PRIVATE
        ${LIB_INCLUDE_DIRS}
        config
        ${CXX_LIB_INCLUDE_DIRS}
)
target_link_libraries(
        tutorial_g_buffer PRIVATE
        cxx_library
        ${LINK_LIBS}
)
target_compile_definitions(tutorial_g_buffer PRIVATE GLEW_STATIC)
add_custom_command(TARGET tutorial_g_buffer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/Repositories/ComputerGraphics/tutorial_g_buffer/libs/external/assimp/bin/assimp-vc143-mtd.dll"
        "$<TARGET_FILE_DIR:tutorial_g_buffer>"
)

