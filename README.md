# tutorial_g_buffer（G-Buffer / Deferred Rendering 实验：构建与运行指南）

本仓库是一个基于 **CMake + C++20 + OpenGL** 的实验工程。
你将编译并运行两个可执行程序：

- `tutorial_g_buffer`：G-Buffer / Deferred Rendering 实验主程序（`src/cxx_main/g_buffer_rendering.cpp`）
- `forward_rendering`：前向渲染对照程序（`src/cxx_main/forward_rendering.cpp`）

> 重要：本仓库通常会在 `.gitignore` 中忽略 `libs/` 与 `resources/`，所以你本地必须准备好第三方库与运行资源，否则会**编译失败**或**运行黑屏/闪退**。

---

## 0. 你需要准备什么（Windows）

- Windows 10/11
- Visual Studio 2022（MSVC v143，x64）
- CMake（工程 `cmake_minimum_required(VERSION 4.0)`，建议你使用仓库/学校提供的版本；若版本不够会直接 configure 失败）
- （可选）Ninja：CLion 常用
- 支持 OpenGL 的显卡驱动

---

## 1. 必须的目录与文件（请先检查）

顶层 `CMakeLists.txt` **写死**了第三方库与资源目录位置：

- 运行资源目录：`<repo>/resources/`
- 第三方库目录：`<repo>/libs/`

请确保目录结构至少满足下面这些“关键路径”。（没有就创建目录并把实验材料拷进去。）

### 1.1 `libs/` 目录布局（必须）

工程会从这些路径找头文件与库：

- `libs/external/glfw/include/` + `libs/external/glfw/lib-vc2022/*.lib`
- `libs/external/glew/include/` + `libs/external/glew/lib/*.lib`
- `libs/external/glm/`（header-only）
- `libs/external/tinyobj/`（header-only）
- `libs/stb/stb_image/`
- `libs/external/assimp/include/` + `libs/external/assimp/lib/*.lib` + `libs/external/assimp/bin/*.dll`

> 说明：
> - `src/cxx_main/CMakeLists.txt` 里会链接 `${GLFW_LIB_DIR}/glfw3.lib`、`${GLEW_LIB_DIR}/glew32s.lib`。
> - `src/cxx_lib/CMakeLists.txt` 里会链接 `${GLFW_LIB_DIR}/glfw3dll.lib`，以及 Assimp 的 Debug/Release 两套库。

### 1.2 `resources/`（必须，否则运行时加载模型/纹理会失败）

把实验发放的模型/贴图等资源放到：

- `resources/`

---

## 2. Assimp（两种方式，二选一）

工程默认把 Assimp 当成“外部预编译库/已安装库”，位置固定为：

- `libs/external/assimp/`

### 方式 A：使用你已有的 Assimp（推荐）

确保以下文件存在（名称与 VS2022 v143 强绑定）：

- `libs/external/assimp/include/assimp/Importer.hpp`
- `libs/external/assimp/lib/assimp-vc143-mtd.lib`（Debug）
- `libs/external/assimp/lib/assimp-vc143-mt.lib`（Release）
- `libs/external/assimp/bin/assimp-vc143-mtd.dll`（Debug 运行时）
- `libs/external/assimp/bin/assimp-vc143-mt.dll`（Release 运行时）

> 注意：如果你用的不是 VS2022(v143)，库名很可能不同，会导致链接失败；要么重新编译 Assimp，要么修改工程的链接库名。

### 方式 B：使用工程自带目标 `compile_assimp` 从源码编译并 install

如果 `thirdparty/assimp/` 下确实有 Assimp 源码（至少存在 `thirdparty/assimp/CMakeLists.txt`），则顶层 CMake 会生成一个目标：

- `compile_assimp`

它会把 Assimp（shared）编译并安装到：

- `libs/external/assimp/`

> 你需要分别为 Debug/Release 构建一次，才能得到两套 `.lib/.dll`。

---

## 3. 关键路径宏：资源与 Shader 从哪里读？

工程不是手写路径，而是 CMake 在编译期生成头文件：

- `src/cxx_main/config/config.h`（由 `config/config_cxx_main.h.in` 生成）

其中包含：

- `RESOURCES_DIR`：固定为 `<repo>/resources`
- `GLSL_ROOT`：固定为 `<repo>/src/cxx_main/glsl`

因此：

- 你的 shader 放在 `src/cxx_main/glsl/`
- 你的模型/纹理放在 `resources/`

---

## 4. 构建（两种常见方式：CLion / Visual Studio）

下面命令都以你的仓库路径为例：

- `D:\Repositories\ComputerGraphics\tutorial_g_buffer`

### 4.1 CLion（常见：Ninja 单配置）

要点：

- Debug/Release 由 `-DCMAKE_BUILD_TYPE=...` 决定
- 先把 `libs/` 与 `resources/` 放好

命令行等价流程：

```powershell
# Debug
cmake -S D:\Repositories\ComputerGraphics\tutorial_g_buffer `
      -B D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug `
      -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug

# 如需：编译并安装 Assimp（可选）
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-debug --target compile_assimp

# Release
cmake -S D:\Repositories\ComputerGraphics\tutorial_g_buffer `
      -B D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-release `
      -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-release

# 如需：编译并安装 Assimp（可选）
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-release --target compile_assimp
```

### 4.2 Visual Studio 2022（多配置生成器）

要点：

- Debug/Release 由 `--config Debug|Release` 决定

```powershell
cmake -S D:\Repositories\ComputerGraphics\tutorial_g_buffer `
      -B D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-vs2022 `
      -G "Visual Studio 17 2022" -A x64

cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-vs2022 --config Debug
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-vs2022 --config Release

# 如需：分别编译并安装 Assimp（可选，但通常你需要两次）
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-vs2022 --target compile_assimp --config Debug
cmake --build D:\Repositories\ComputerGraphics\tutorial_g_buffer\cmake-build-vs2022 --target compile_assimp --config Release
```

---

## 5. 运行（以及最容易踩的坑：DLL）

### 5.1 运行哪个程序？

- G-Buffer 实验：运行 `tutorial_g_buffer`
- 对照：运行 `forward_rendering`

### 5.2 assimp DLL（运行时必须存在）

当前工程在 `src/cxx_main/CMakeLists.txt` 中写了 POST_BUILD：

- 把 `${ASSIMP_DIR}/bin/assimp-vc143-mtd.dll` 复制到 `tutorial_g_buffer` 和 `forward_rendering` 的输出目录

这意味着：

- **Debug**：通常没问题（会拷贝 `assimp-vc143-mtd.dll`）
- **Release**：你很可能需要手动把 `assimp-vc143-mt.dll` 放到可执行文件目录（否则可能找不到 DLL 或配置不匹配）

你可以按配置手动处理：

- Debug：复制 `libs/external/assimp/bin/assimp-vc143-mtd.dll`
- Release：复制 `libs/external/assimp/bin/assimp-vc143-mt.dll`

---

## 6. 常见问题排查（按优先级）

### 6.1 CMake configure 报错：找不到 glfw/glew/glm/assimp

- 检查 `libs/` 是否存在且目录是否符合第 1 节布局
- 检查是否是 VS2022(v143) 对应的 Assimp 库名（`assimp-vc143-*.lib`）

### 6.2 链接报错：`assimp-vc143-mtd.lib` 或 `assimp-vc143-mt.lib` 找不到

- 说明你没有准备对应配置的 `.lib`
- 如果你只装了 Release 的 Assimp，却在 Debug 编译，会失败（反之亦然）

### 6.3 运行时报错：缺少 `assimp-*.dll` / 启动即闪退

- 确保可执行文件目录中存在对应配置的 DLL
- Debug/Release 的 DLL 不要混用

### 6.4 能运行但黑屏/没有模型

优先检查：

- `resources/` 是否放了实验要求的模型/贴图
- shader 是否确实位于 `src/cxx_main/glsl/`
- OpenGL context 是否创建成功（glfw / glew 初始化）

---

## 7. （给助教/进阶）实验环境自检清单

- [ ] `libs/external/glfw/lib-vc2022/glfw3.lib` 与 `glfw3dll.lib` 都存在
- [ ] `libs/external/glew/lib/glew32s.lib` 存在（工程使用静态 GLEW：`GLEW_STATIC`）
- [ ] `libs/external/assimp/include/assimp/Importer.hpp` 存在
- [ ] Debug：`assimp-vc143-mtd.lib` + `assimp-vc143-mtd.dll`
- [ ] Release：`assimp-vc143-mt.lib` + `assimp-vc143-mt.dll`
- [ ] `resources/` 已放置实验资源
- [ ] `src/cxx_main/glsl/` 中 shader 文件完整
