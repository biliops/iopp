# iopp - 多语言进程IO监控工具

iopp 是一个基于 Linux `/proc` 文件系统的进程文件IO监控工具，支持 **C、Python、Go、Rust、Node.js** 五种编程语言实现。

## 功能特性

- 多语言实现：C、Python、Go、Rust、Node.js
- 实时监控进程的IO统计信息
- 多语言界面：简体中文、繁体中文、English
- 多种显示格式：表情符/纯文本
- 支持多种显示格式（字节、KB、MB、自动单位）
- 支持隐藏空闲进程
- 支持显示完整命令行
- 可配置监控间隔和次数

## 项目结构

```
iopp/
├── src/
│   ├── c/           # C 语言实现
│   │   ├── iopp-c.c
│   │   └── Makefile
│   ├── python/      # Python 实现
│   │   └── iopp.py
│   ├── go/          # Go 实现
│   │   ├── main.go
│   │   └── go.mod
│   ├── rust/        # Rust 实现
│   │   ├── src/main.rs
│   │   └── Cargo.toml
│   └── node/        # Node.js 实现
│       ├── iopp.js
│       └── package.json
├── docs/            # 文档
├── tests/           # 测试用例
└── Makefile         # 顶层构建文件
```

## 支持的操作系统

**仅支持 Linux 系统**

该程序依赖 Linux 特有的 `/proc` 文件系统获取进程文件IO统计信息，其他系统（如 macOS、Windows）不支持。

## 安装方法

### 快速安装

```bash
# 克隆仓库
git clone --depth 1 -b python git@github.com:biliops/iopp.git ./iopp
cd ./iopp

# 构建所有版本
make all

# 安装所有版本
sudo make install
```

### 按语言安装

**C 版本**
```bash
cd src/c
make
sudo make install
```

**Go 版本**
```bash
cd src/go
go build -o iopp-go .
sudo install iopp-go /usr/local/bin/
```

**Rust 版本**
```bash
cd src/rust
cargo build --release
sudo install target/release/iopp /usr/local/bin/
```

**Python 版本**
```bash
chmod +x src/python/iopp.py
sudo cp src/python/iopp.py /usr/local/bin/iopp
```

**Node.js 版本**
```bash
cd src/node
npm install -g
```

## 使用方法

### 基本使用

```bash
# 显示一次所有进程的IO信息
iopp

# 每秒刷新一次，持续监控
iopp 1

# 每2秒刷新一次，共显示5次
iopp 2 5
```

### 过滤和显示选项

```bash
# 仅显示活跃进程
iopp -i

# 以人类可读格式显示（自动选择单位）
iopp -u

# 显示完整命令行
iopp -c

# 以千字节为单位
iopp -k

# 以兆字节为单位
iopp -m
```

### 多语言界面

```bash
# 使用简体中文 (默认)
iopp -L zh

# 使用繁体中文
iopp -L tw

# 使用英文
iopp -L en
```

### 输出格式

```bash
# 使用表情符 (默认)
iopp -e

# 使用纯文本
iopp -t
```

## 命令行选项

| 选项 | 长选项 | 说明 |
|------|--------|------|
| `-c` | `--command` | 显示完整命令行 |
| `-h` | `--help` | 显示帮助信息 |
| `-i` | `--idle` | 隐藏空闲进程 |
| `-k` | `--kilobytes` | 以千字节显示数据 |
| `-m` | `--megabytes` | 以兆字节显示数据 |
| `-u` | `--human-readable` | 自动选择合适的单位（B/K/M/G） |
| `-v` | `--version` | 显示版本信息 |
| `-L` | `--lang <zh\|tw\|en>` | 设置界面语言 |
| `-e` | `--emoji` | 使用表情符格式 |
| `-t` | `--text` | 使用纯文本格式 |

## 输出字段说明

| 字段 | 说明 |
|------|------|
| 进程ID (PID) | 进程标识符 |
| 读字符 (RCHAR) | 从文件读取的字符数 |
| 写字符 (WCHAR) | 写入文件的字符数 |
| 系统读 (SYSCR) | 系统调用读次数 |
| 系统写 (SYSCW) | 系统调用写次数 |
| 读字节 (READ) | 实际磁盘读取字节数 |
| 写字节 (WRITE) | 实际磁盘写入字节数 |
| 取消写 (CANCEL) | 被取消的写入字节数 |
| 命令行 (COMMAND) | 完整的命令行信息 |

## 示例

```bash
# 每1秒刷新，显示活跃进程，使用人类可读格式
iopp -i -u 1

# 显示一次，以兆字节为单位
iopp -m

# 完整命令行模式，每2秒刷新
iopp -icu 2

# 使用英文界面
iopp -L en -u

# 使用纯文本格式
iopp -t
```

## 卸载

```bash
sudo make uninstall
```

## 添加新语言实现

如需添加新的语言实现，请参考以下步骤：

1. 在 `src/` 下创建新的语言目录
2. 实现与现有版本相同的CLI接口
3. 确保输出格式与原版本一致
4. 更新顶层 `Makefile`
5. 更新本 README

## 许可证

MIT License
