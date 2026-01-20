#设置一个config文件, 方便修改参数
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config)
set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
configure_file(
        ${CONFIG_DIR}/config_cxx_lib.h.in
        ${GENERATED_INCLUDE_DIR}/config.h
)
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

set(SE_INCLUDE_DIR se)

set(LIB_INCLUDE_DIRS
        ${GLEW_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${STB_IMAGE_INCLUDE_DIR}
        ${TINYOBJLOADER_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIR}
)
add_library(cxx_library
        scene/data_structures/Material.h
        scene/data_structures/Mesh.h
        scene/data_structures/Node.h
        scene/data_structures/RenderScene.h
        scene/data_structures/TextureInfo.h
        scene/data_structures/Transform.h
        scene/data_structures/Vertex.h
        scene/Interface_SceneImporter.h
        scene/SceneImporter_Assimp.cpp
        scene/SceneImporter_Assimp.h
        resources/Interface_ExternalResourcePoolAccessor.h
        resources/VertexPoolAccessor.cpp
        resources/VertexPoolAccessor.h
        resources/IndexPoolAccessor.cpp
        resources/IndexPoolAccessor.h

)

target_include_directories(cxx_library PRIVATE
        ${LIB_INCLUDE_DIRS}
        ${SE_INCLUDE_DIR}
        se
)

target_link_libraries( cxx_library PRIVATE
        ${GLFW_LIB_DIR}/glfw3dll.lib
        debug ${ASSIMP_LIB_DIR}/assimp-vc143-mtd.lib
        optimized ${ASSIMP_LIB_DIR}/assimp-vc143-mt.lib
        seTools
)

target_compile_definitions(cxx_library PRIVATE 
        GLM_ENABLE_EXPERIMENTAL #启用glm的实验特性
)
add_subdirectory(se)