# iopp 文档

## 概述

iopp 是一个跨语言的 Linux 进程 I/O 监控工具，支持多种编程语言实现。

## 架构设计

### 多语言实现原则

1. **统一的 CLI 接口**：所有语言版本保持相同的命令行参数
2. **一致的输出格式**：确保各版本输出格式完全一致
3. **独立开发**：各语言版本可独立编译和运行

### 目录结构

```
src/
├── c/           # C 语言实现 (编译型)
├── python/      # Python 实现 (解释型)
├── go/          # Go 实现 (编译型)
├── rust/        # Rust 实现 (编译型)
└── node/        # Node.js 实现 (解释型)
```

## 技术实现

### 数据采集

所有语言版本都从 Linux 的 `/proc/[pid]/io` 文件读取 I/O 统计数据：

- `rchar`: 读取的字符数
- `wchar`: 写入的字符数
- `syscr`: 读系统调用次数
- `syscw`: 写系统调用次数
- `read_bytes`: 实际磁盘读取字节
- `write_bytes`: 实际磁盘写入字节
- `cancelled_write_bytes`: 被取消的写入字节

### 多语言界面

使用结构化字符串表实现国际化：

```c
// C 语言示例
typedef struct {
    const char *label_pid;
    const char *label_rchar;
    // ...
} lang_strings_t;
```

### 输出格式

支持两种输出格式：

- **表情符格式** (默认): `✖ 错误信息`
- **纯文本格式**: `[ERROR] 错误信息`

## 构建说明

### 依赖要求

| 语言 | 依赖 |
|------|------|
| C | gcc, make |
| Go | Go 1.21+ |
| Rust | Rust 2021 edition, cargo |
| Python | Python 3.x |
| Node.js | Node.js 16+ |

### 构建步骤

1. 克隆仓库
2. 运行 `make all` 构建所有版本
3. 运行 `sudo make install` 安装

## API 参考

### 命令行参数

| 参数 | 说明 | 类型 |
|------|------|------|
| `-c, --command` | 显示完整命令行 | 布尔 |
| `-i, --idle` | 隐藏空闲进程 | 布尔 |
| `-k, --kilobytes` | 以 KB 显示 | 布尔 |
| `-m, --megabytes` | 以 MB 显示 | 布尔 |
| `-u, --human-readable` | 自动单位 | 布尔 |
| `-L, --lang` | 设置语言 | 字符串 |
| `-e, --emoji` | 表情符格式 | 布尔 |
| `-t, --text` | 纯文本格式 | 布尔 |
| `interval` | 刷新间隔 | 整数 |
| `count` | 刷新次数 | 整数 |

## 扩展指南

### 添加新语言实现

1. 在 `src/` 下创建新目录
2. 实现核心功能函数：
   - `collect_processes()` - 采集进程 I/O 数据
   - `print_header()` - 打印表头
   - `print_stats()` - 打印统计数据
3. 实现命令行参数解析
4. 实现多语言字符串表
5. 更新 Makefile
6. 更新 README.md

### 参考实现

推荐按照以下顺序参考实现：

1. **Python 版本** (`src/python/iopp.py`) - 最易读
2. **Go 版本** (`src/go/main.go`) - 结构清晰
3. **Rust 版本** (`src/rust/src/main.rs`) - 类型安全
4. **C 版本** (`src/c/iopp-c.c`) - 性能最优
