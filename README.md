# tutorial_g_buffer

这是一个基于 **CMake + C++20 + OpenGL** 的学习工程，包含：
- `forward_rendering`：前向渲染示例
- `tutorial_g_buffer`：G-Buffer / Deferred 相关示例
- `cxx_library`：场景/资源导入（Assimp）等通用库

---

## 1. 目录结构（和你需要关心的部分）

- `CMakeLists.txt`：顶层 CMake，定义依赖根目录、`RESOURCES_DIR`、以及可选的 `compile_assimp` 目标
- `src/cxx_main/`：可执行程序（`tutorial_g_buffer`、`forward_rendering`）
  - `src/cxx_main/CMakeLists.txt`：
    - 生成 `src/cxx_main/config/config.h`
    - 定义 `GLSL_ROOT` 为 `src/cxx_main/glsl`
    - 链接 `glfw/glew/assimp` 等
- `src/cxx_lib/`：静态库 `cxx_library`
  - `src/cxx_lib/CMakeLists.txt`：
    - 生成 `src/cxx_lib/config/config.h`
    - 链接 assimp，并区分 debug/optimized lib 名称
- `config/`：`config.h` 模板
  - `config/config_cxx_main.h.in`：生成资源路径宏
  - `config/config_cxx_lib.h.in`：生成日志相关宏
- `libs/`：**预编译第三方库**存放位置（本仓库 `.gitignore` 里忽略了它，所以你需要自己准备）
  - `libs/external/glfw`、`libs/external/glew`、`libs/external/glm`、`libs/external/tinyobj`、`libs/stb/stb_image`
  - `libs/external/assimp`：assimp install 目标目录（可手动放，也可从源码编译安装）
- `thirdparty/assimp`：assimp 源码目录（用于 `compile_assimp`）
- `resources/`：运行时资源（gltf/纹理等）；**同样被 `.gitignore` 忽略**，你需要自己准备

---

## 2. 先决条件（Windows）

- Visual Studio 2022（MSVC v143 toolset）
- CMake（仓库里显示使用 CMake 4.x；3.20+ 一般也够，但以你的环境为准）
- Ninja（可选，CLion 默认常用 Ninja；Visual Studio 生成器则不需要）
- OpenGL 可用显卡驱动

> 这个工程的 assimp lib 名称是 `assimp-vc143-*.lib`，强绑定 VS2022(v143)。如果你用 v142/v141，需要重新编 assimp 或修改链接库名。

---

## 3. 外部库布局约定（`libs/` 必须长这样）

顶层 `CMakeLists.txt` 里写死了这些目录：

- `libs/external/glfw/include` + `libs/external/glfw/lib-vc2022/*.lib`
- `libs/external/glew/include` + `libs/external/glew/lib/*.lib`
- `libs/external/glm/`（header-only）
- `libs/external/tinyobj/`（header-only 或按你现有结构）
- `libs/stb/stb_image/`
- `libs/external/assimp/include` + `libs/external/assimp/lib/*.lib` + `libs/external/assimp/bin/*.dll`

由于你的 `.gitignore` 忽略了 `/libs/`，所以这些内容不会随仓库提交。

---

## 4. Assimp：两种方式（二选一）

工程默认把 assimp 当成“外部预编译库”：

```cmake
set(ASSIMP_FALLBACK_DIR "${LIBRARY_ROOT}/external/assimp")
set(ASSIMP_DIR          "${ASSIMP_FALLBACK_DIR}")
set(ASSIMP_INCLUDE_DIR "${ASSIMP_DIR}/include")
set(ASSIMP_LIB_DIR     "${ASSIMP_DIR}/lib")
set(ASSIMP_BIN_DIR     "${ASSIMP_DIR}/bin")
```

### 方式 A：使用你自己编译好的 Assimp（推荐，最直接）

目标：把这些文件放到 `libs/external/assimp/` 下，使得 CMake 能找到：

- `libs/external/assimp/include/assimp/Importer.hpp`
- `libs/external/assimp/lib/assimp-vc143-mt.lib`（Release）
- `libs/external/assimp/lib/assimp-vc143-mtd.lib`（Debug）
- `libs/external/assimp/bin/assimp-vc143-mt.dll` / `assimp-vc143-mtd.dll`（运行时需要）

**如何得到这些文件：**
1. 从 assimp 官方仓库获取源码（或你已有源码）
2. 用 VS2022 编译（v143）
3. `cmake --install` 安装到 `libs/external/assimp`

> 注意：本工程在 `cxx_library` 里用的是 `debug ...mtd.lib / optimized ...mt.lib` 这种写法，所以 Debug/Release 两套库名需要都存在。

### 方式 B：使用工程自带的 `compile_assimp` 目标从源码编译并 install

顶层 `CMakeLists.txt` 已经定义了一个可选目标 `compile_assimp`：
- 源码：`thirdparty/assimp`
- build dir：`<build>/thirdparty-assimp-build`
- install dir：`libs/external/assimp`

**前提**：`thirdparty/assimp` 目录里确实存在 assimp 源码（至少要有 `thirdparty/assimp/CMakeLists.txt`）。

在你已经 CMake configure 过主工程后（生成过 build 目录），执行：

```powershell
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug --target compile_assimp --config Debug
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-release --target compile_assimp --config Release
```

> 说明：
> - 如果你用的是 Ninja 单配置生成器，就不要传 `--config`，而是靠 `-DCMAKE_BUILD_TYPE=Debug/Release` 决定。
> - 这个目标会把 assimp 安装到 `libs/external/assimp`，符合工程默认路径。

---

## 5. 关于 `config.h`（资源路径/Shader 路径宏的正确来源）

你的工程不是手写资源路径，而是用 `configure_file()` 生成头文件：

- `src/cxx_main/config/config.h` 由 `config/config_cxx_main.h.in` 生成：

```c
#define RESOURCES_DIR "@RESOURCES_DIR@"
#define GLSL_ROOT "@GLSL_ROOT@"
```

其中：
- `RESOURCES_DIR` 来自顶层 `CMakeLists.txt`：`set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)`
- `GLSL_ROOT` 来自 `src/cxx_main/CMakeLists.txt`：`set(GLSL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/glsl)`

也就是说：
- 资源应该放在 `<repo>/resources`
- shader 应该放在 `src/cxx_main/glsl`

> 你的 `.gitignore` 忽略了 `/resources/`，所以你本地看得到，仓库里可能没有；这不是路径宏的问题。

---

## 6. libs：如何添加 Release 库 / Debug 库（以 Assimp 为例）

你现在的链接方式在 `src/cxx_lib/CMakeLists.txt`：

```cmake
target_link_libraries( cxx_library PRIVATE
    ${GLFW_LIB_DIR}/glfw3dll.lib
    debug ${ASSIMP_LIB_DIR}/assimp-vc143-mtd.lib
    optimized ${ASSIMP_LIB_DIR}/assimp-vc143-mt.lib
    seTools
)
```

含义：
- Debug 配置会链接 `assimp-vc143-mtd.lib`
- Release/RelWithDebInfo 会链接 `assimp-vc143-mt.lib`

因此你把第三方库放进 `libs/external/<name>/lib` 时，建议遵循：
- 明确区分 Debug/Release 两套库（名字可以像 assimp 这样用 `d` 后缀；或者你也可以改 CMake 去适配你现有命名）
- 如果是动态库（dll），还要把 dll 放到 `bin`，并确保运行时能找到（见下一节）

---

## 7. 构建与运行（CLion / Visual Studio）

### 7.1 使用 CLion（常见 Ninja 单配置）

要点：
- 配置 CMake Profile 为 Debug/Release（对应 `CMAKE_BUILD_TYPE`）
- 先确保 `libs/` 下的 glfw/glew/assimp 都已经就位

如果你想完全走命令行（可选）：

```powershell
# Debug
cmake -S D:\Repositories\ComputerGraphics\tutorial_g_buffer -B D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug

# Release
cmake -S D:\Repositories\ComputerGraphics\tutorial_g_buffer -B D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-release
```

### 7.2 使用 Visual Studio 生成器（多配置）

```powershell
cmake -S D:\Repositories\ComputerGraphics\tutorial_g_buffer -B D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug-visual-studio -G "Visual Studio 17 2022" -A x64
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug-visual-studio --config Debug
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug-visual-studio --config Release
```

---

## 8. 运行时 DLL（非常关键）

### 8.1 当前工程的现状（你需要知道的坑）

`src/cxx_main/CMakeLists.txt` 里对 `tutorial_g_buffer` 有一个 POST_BUILD：

```cmake
add_custom_command(TARGET tutorial_g_buffer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "D:/Repositories/ComputerGraphics/tutorial_g_buffer/libs/external/assimp/bin/assimp-vc143-mtd.dll"
    "$<TARGET_FILE_DIR:tutorial_g_buffer>"
)
```

这行 **硬编码了一个绝对路径**。如果你的本机路径不同，或者你在 Release 下运行，它会导致：
- DLL 没拷贝过去（运行失败或找不到依赖）
- 或者 Release 用了 Debug DLL（行为不一致）

### 8.2 推荐做法（不改工程结构，只修正路径逻辑）

你有两种选择：

1) **手动处理 DLL**：
- Debug：把 `libs/external/assimp/bin/assimp-vc143-mtd.dll` 复制到可执行文件目录
- Release：把 `libs/external/assimp/bin/assimp-vc143-mt.dll` 复制到可执行文件目录

2) **改 CMake**：把上面的硬编码路径改成使用 `ASSIMP_BIN_DIR`，并按配置选择 dll 名称。

> 如果你希望我直接把这段 CMake 改好，我可以按你现有命名（`mt/mtd`）写成生成器表达式，保证 Debug/Release 都自动 copy。

---

## 9. 常见问题排查

### 9.1 “能编译但黑屏/啥也看不到”

优先检查：
- 你的 shader 是否真的从 `GLSL_ROOT` 指向的目录读取（`src/cxx_main/glsl`）
- 你的模型/纹理是否真的在 `RESOURCES_DIR`（`<repo>/resources`）
- OpenGL context 是否创建成功（glfw 初始化、窗口创建、glew init）
- 深度测试/背面剔除等状态是否按预期

### 9.2 “运行时找不到 assimp*.dll / 程序启动即闪退”

- 检查可执行程序目录中是否存在正确的 assimp dll
- Debug/Release 对应 dll 是否匹配
- 确保是 VS2022(v143) 编译出的 assimp

### 9.3 `libs/` / `resources/` 为什么仓库里没有？

`.gitignore` 明确忽略了：
- `/libs/`
- `/resources/`

你需要自行准备这些目录和内容。
已经放在实验材料中。
