# wagaLauncher
>《明日方舟》艾丽妮的键盘音效播放器

> 一个轻量的键盘音效播放器 —— 按键即响，打断式播放，多套音效方案随心切换。

![Version](https://img.shields.io/badge/version-2.0.2-blue)
![Platform](https://img.shields.io/badge/platform-Windows%2010%2B-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)
![C++](https://img.shields.io/badge/C%2B%2B-14-00599C)
[![Downloads](https://img.shields.io/github/downloads/Cudny10D/wagaLauncher/total)](https://github.com/Cudny10D/wagaLauncher/releases)

---

## 目录

- [简介](#简介)
- [功能特性](#功能特性)
- [界面预览](#界面预览)
- [下载与安装](#下载与安装)
- [使用说明](#使用说明)
- [音效目录结构](#音效目录结构)
- [从源码构建](#从源码构建)
- [打包安装程序](#打包安装程序)
- [常见问题](#常见问题)
- [项目结构](#项目结构)
- [免责声明](#免责声明)
- [开源协议](#开源协议)
- [致谢](#致谢)

---

## 简介

**wagaLauncher** 是一个 Windows 平台的按键音效播放器。按下键盘任意键，即可随机播放一个自定义音效，支持**打断式播放**（新音效立即掐掉上一个）和**多套音效方案**快速切换。

程序使用免费开源的 [miniaudio](https://github.com/mackron/miniaudio) 作为音频引擎，界面基于 Win32 + GDI+ 手绘实现，**无外部运行时依赖**，单文件绿色运行。

- **作者**：Cudny（使用 DeepSeek 辅助开发）
- **项目主页**：https://github.com/Cudny10D/wagaLauncher
- **当前版本**：2.0.2
- **最后修改**：2026-09-25

---

## 功能特性

| 特性 | 说明 |
|---|---|
| **全局键盘监听** | 使用 Win32 低级键盘钩子，任意窗口下按键都能触发 |
| **打断式播放** | 新音效立即掐掉上一个，快速打字时声音干净利落 |
| **多套音效方案** | 按子文件夹分组，设置页一键切换，无需重启 |
| **支持格式** | `.wav` / `.mp3` / `.flac` |
| **现代扁平 GUI** | 无边框窗口 + 亚克力卡片 + 滑页动画 |
| **深色/浅色自适应** | 系统标题栏自动跟随主题 |
| **音量调节** | 0 ~ 100% 平滑拖动 |
| **系统托盘** | 关闭窗口最小化到托盘，右键菜单快速控制 |
| **开机自启** | 设置页勾选即可，自启时静默运行不弹窗 |
| **单实例保护** | 避免重复启动，第二次启动静默退出 |

---

## 界面预览

**主页**
<img width="380" height="500" alt="image" src="https://github.com/user-attachments/assets/4818c8dc-d636-4545-ad35-3672506fbeb6" />



**设置页**
<img width="380" height="500" alt="image" src="https://github.com/user-attachments/assets/5d2e693b-c041-413f-b5c2-862717da1ff8" />

---

## 下载与安装

### 系统要求

- Windows 10 1809 或更高版本（Windows 11 推荐）
- x64 架构
- 无外部运行时依赖

### 安装步骤

1. 从 [Releases](https://github.com/Cudny10D/wagaLauncher/releases) 下载最新的 `wagaLauncher_Setup.exe`
2. 双击运行，按向导完成安装
3. 安装时可勾选：
   - ☑ 创建桌面快捷方式
   - ☑ 创建开始菜单快捷方式
4. 安装完成后可勾选"立即启动"

默认安装位置：`C:\Program Files\wagaLauncher\`

### 卸载

控制面板 → 程序和功能 → **Key Sound** → 卸载

卸载时会自动：
- 结束正在运行的程序
- 清除开机自启注册表项
- 删除安装目录

---

## 使用说明

### 主页

| 元素 | 操作 |
|---|---|
| **开关**（右下角） | 点击开/关音效，关闭后按键无声音 |
| **设置** | 进入设置页 |
| **退出** | 完全退出程序（悬停变红） |
| **标题栏** | 按住可拖动窗口 |
| **最小化** | 最小化到任务栏 |
| **关闭 ✕** | 隐藏到系统托盘（不退出） |

### 设置页

| 元素 | 操作 |
|---|---|
| **← 返回** | 回到主页 |
| **音效方案** | 点击芯片切换方案，立即生效 |
| **音量** | 拖动滑块调节，0~100% |
| **打开音效文件夹** | 打开程序同目录的 `sounds\` |
| **开机自动启动** | 勾选后下次开机静默启动 |

### 系统托盘

- **双击托盘图标**：恢复主窗口
- **右键托盘图标**：
  - 显示主窗口
  - 启用音效（勾选）
  - 退出

---

## 音效目录结构

程序会读取 **exe 同目录** 下的 `sounds\` 文件夹。

### 单套音效

如果 `sounds\` 下没有子文件夹，所有音效当作"方案1"：

```
sounds\
├── click1.wav
├── click2.wav
└── click3.mp3
```

### 多套音效

如果 `sounds\` 下有子文件夹，**每个子文件夹是一套方案**，文件夹名即方案名：

```
sounds\
├── 方案1\
│   ├── click1.wav
│   ├── click2.wav
│   └── click3.mp3
├── 方案2\
│   ├── type1.wav
│   └── type2.flac
└── 机械键盘\
    ├── key1.wav
    ├── key2.wav
    └── key3.wav
```

**注意**：
- 子文件夹不支持嵌套（只扫描一层）
- 文件夹名可以是中文、英文、数字
- 新增方案后需**重启程序**生效
- 单套方案最多 128 个音效

---

## 从源码构建

### 依赖

| 工具 | 版本 | 用途 |
|---|---|---|
| Visual Studio 2022 | 含"使用 C++ 的桌面开发" | 编译 |
| [miniaudio.h](https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h) | 最新 | 音频引擎（单头文件） |

### 目录结构

```
key_sounds_gui_dev\
├── key_sound.cpp        ← 主程序
├── miniaudio.h          ← 下载后放这里
├── app.ico              ← 可选，程序图标
├── app.rc               ← 可选，图标资源描述
└── sounds\              ← 音效目录
    ├── 方案1\
    └── 方案2\
```

### `app.rc` 内容（有图标时）

```rc
IDI_APPICON ICON "app.ico"
```

保存为 **ANSI** 编码。

### 编译命令

打开 **"x64 Native Tools Command Prompt for VS 2022"**：

```bat
cd /d C:\path\to\key_sounds_gui_dev

rc app.rc

cl /utf-8 /EHsc /O2 /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE ^
   key_sound.cpp app.res ^
   /link /SUBSYSTEM:WINDOWS ^
   user32.lib gdi32.lib gdiplus.lib shell32.lib ^
   ole32.lib winmm.lib dwmapi.lib advapi32.lib
```

**没有 `app.rc`** 时去掉 `rc app.rc` 和命令里的 `app.res`。

编译成功 → 当前目录出现 `key_sound.exe`。

### 编译参数说明

| 参数 | 作用 |
|---|---|
| `/utf-8` | 源文件按 UTF-8 解释（关键，含中文必须） |
| `/EHsc` | 标准 C++ 异常处理 |
| `/O2` | 优化 |
| `/D_CRT_SECURE_NO_WARNINGS` | 屏蔽 `strcpy` 等安全警告 |
| `/DUNICODE /D_UNICODE` | 使用宽字符版 Win32 API |
| `/SUBSYSTEM:WINDOWS` | 无控制台窗口 |

### 链接的系统库

| 库 | 用途 |
|---|---|
| `user32.lib` | 窗口、消息、键盘钩子 |
| `gdi32.lib` | 底层绘图（BitBlt 等） |
| `gdiplus.lib` | GDI+ 绘制 |
| `shell32.lib` | 托盘图标、ShellExecute |
| `ole32.lib` | COM 初始化 |
| `winmm.lib` | 多媒体定时器 |
| `dwmapi.lib` | 暗色标题栏、圆角窗口 |
| `advapi32.lib` | 注册表操作（开机自启） |

---

## 打包安装程序

使用 [Inno Setup](https://jrsoftware.org/isdl.php)（6.2+ 版本）。

### 1. 建发布目录

```
key_sounds_gui_dev\
├── dist\
│   ├── key_sound.exe
│   ├── app.ico
│   └── sounds\
│       ├── 默认、哇嘎小鸟\
│       ├── 重岳形不成形\
│       └── 方案2\
└── installer.iss
```

### 2. `installer.iss` 模板

```ini
#define MyAppName      "Key Sound"
#define MyAppVersion   "2.0.2"
#define MyAppPublisher "Cudny"
#define MyAppExeName   "key_sound.exe"

[Setup]
AppId={{8F3A9C21-7B4E-4D2A-9E5F-1A2B3C4D5E6F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
OutputDir=Output
OutputBaseFilename=KeySound_Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupIconFile=dist\app.ico

[Languages]
Name: "chinese"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon";   Description: "创建桌面快捷方式";     GroupDescription: "附加任务:"; Flags: checkedonce
Name: "startmenuicon"; Description: "创建开始菜单快捷方式"; GroupDescription: "附加任务:"; Flags: checkedonce

[Files]
Source: "dist\key_sound.exe";  DestDir: "{app}"; Flags: ignoreversion
Source: "dist\app.ico";        DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "dist\sounds\*";       DestDir: "{app}\sounds"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}";       Filename: "{app}\{#MyAppExeName}"; Tasks: startmenuicon
Name: "{group}\卸载 {#MyAppName}";   Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{cmd}"; Parameters: "/C reg delete HKCU\Software\Microsoft\Windows\CurrentVersion\Run /v KeySound /f >nul 2>&1"; Flags: runhidden
Filename: "{app}\{#MyAppExeName}"; Description: "立即启动 {#MyAppName}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{cmd}"; Parameters: "/C taskkill /F /IM {#MyAppExeName} >nul 2>&1"; Flags: runhidden
Filename: "{cmd}"; Parameters: "/C reg delete HKCU\Software\Microsoft\Windows\CurrentVersion\Run /v KeySound /f >nul 2>&1"; Flags: runhidden

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
```

### 3. 编译安装包

1. Inno Setup Compiler → **File → Open** → 选 `installer.iss`
2. 按 **F9**
3. 输出：`Output\KeySound_Setup.exe`

---

## 常见问题

<details>
<summary><b>Q1：按键没有声音</b></summary>

检查：
- 程序主界面显示"已加载 N 个音效"，N 是否为 0
- `sounds\` 目录是否在 exe 同目录
- 音效文件格式是否为 `.wav` / `.mp3` / `.flac`
- 设置页开关是否处于"已开启"
- 音量是否调到了 0
- 杀毒软件是否拦截了全局键盘钩子
</details>

<details>
<summary><b>Q2：切换方案后没反应</b></summary>

新方案文件夹里可能没有音效文件。检查子文件夹里是否有支持格式的音频。
</details>

<details>
<summary><b>Q3：显示"已加载 0 个音效"</b></summary>

`sounds\` 里没东西，或者文件格式不支持。用播放器双击验证一下音频文件是否正常。
</details>

<details>
<summary><b>Q4：编译报"字符串字面量中的换行符"</b></summary>

源文件编码问题。确认：
1. 源文件保存为 **UTF-8**
2. 编译命令里有 `/utf-8`
</details>

<details>
<summary><b>Q5：编译报"无法解析的外部符号 BitBlt"</b></summary>

链接命令缺 `gdi32.lib`。检查 cl 命令里的库列表。
</details>

<details>
<summary><b>Q6：开机自启时窗口弹出</b></summary>

只有通过"设置页勾选"写入的注册表项才带 `--startup` 参数，程序会静默启动。如果你手动往注册表加过项，删掉重新从程序内勾选。
</details>

<details>
<summary><b>Q7：卸载后开机还报错</b></summary>

旧版自启项残留。手动打开注册表编辑器：
```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
```
删除 `KeySound` 项。
</details>

<details>
<summary><b>Q8：杀毒软件报警</b></summary>

全局键盘钩子是很多输入法、快捷键工具的标准技术，会被部分杀软标记为可疑行为。将程序加入信任列表即可。
</details>

---

## 项目结构

```
wagaLauncher/
└── key_sounds_gui_dev/
    ├── key_sound.cpp        主程序（单文件，约 1800 行）
    ├── miniaudio.h          音频引擎单头文件
    ├── app.ico              程序图标（可选）
    ├── app.rc               图标资源描述（可选）
    ├── sounds/              音效根目录
    │   ├── 默认-哇嘎小鸟/
    │   ├── 重岳形不成形/
    │   └── 自定义/
    ├── dist/                发布目录
    │   ├── key_sound.exe
    │   ├── app.ico
    │   └── sounds/
    ├── installer.iss        Inno Setup 脚本
    └── Output/              安装包输出目录
        └── KeySound_Setup.exe
```

### 主要代码模块

| 模块 | 说明 |
|---|---|
| `SoundPack` | 一套音效方案的数据结构 |
| `LoadAllPacks` | 扫描 `sounds\` 下所有子文件夹，构建方案列表 |
| `PlayInterrupting` | 打断式播放：停掉上一个，从头播放新的音效 |
| `LowLevelKeyboardProc` | 全局键盘钩子回调 |
| `DrawUI` / `DrawMainPage` / `DrawSettingsPage` | GDI+ 双缓冲绘制 |
| `DrawAcrylicCard` | 亚克力卡片（半透明白 + 白边 + 投影） |
| `WndProc` | 主窗口消息处理 |
| `IsAutoStartEnabled` / `SetAutoStart` | 注册表开机自启读写 |

---

## 免责声明

本程序是由**《明日方舟》游戏爱好者**，使用免费开源的 miniaudio 制作。

- 程序所涉及的公司名称、商标、产品等均为其各自所有者的资产，仅供识别。
- 程序内使用的游戏图片、音频、文本原文，仅用于更好地表现游戏资料，其版权属于**上海鹰角网络科技有限公司**及其关联公司。
- 除非另有声明，本仓库其他内容采用**知识共享署名-非商业性使用-相同方式共享**授权。

---

## 开源协议

本程序使用 **MIT 开源协议**。

允许任何人自由使用、复制、修改、合并、发布、分发、再许可和/或出售本软件的副本，但必须包含原作者的版权声明和许可声明。

```
MIT License

Copyright (c) 2026 Cudny

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 致谢

- 音频引擎：[miniaudio](https://github.com/mackron/miniaudio) by David Reid (MIT / public domain)
- 界面绘制：Windows GDI+ / Win32 API
- 开发辅助：DeepSeek
- 灵感来源：《明日方舟》游戏爱好者社区

---

<div align="center">

**wagaLauncher** · v2.0.2 · © 2026 Cudny

如果这个程序让你的打字变得更有趣，欢迎点个 ⭐

</div>
