CC = gcc # 编译器
CFLAGS = -v -Wall -Wextra -O3 # 编译选项
TARGET = iopp # 目标文件
SOURCES = iopp.c # 源文件

OS := $(shell uname -s)

.PHONY: all clean install uninstall help

all: check_os $(TARGET) # 默认目标

check_os:
	@if [ "$(OS)" != "Linux" ]; then \
		echo "❌  错误：iopp 仅支持 Linux 操作系统\n\n"; \
		echo "\t  当前系统: $(OS)"; \
		echo "ℹ️  此程序依赖 Linux /proc 文件系统获取进程 IO 统计信息"; \
		echo "\t  其他系统（如 macOS、Windows）暂不支持\n\n"; \
		false; \
	fi

.PHONY: $(TARGET)
$(TARGET): # 编译规则
	@if [ -f $(TARGET) ] && [ $(TARGET) -nt $(SOURCES) ]; then \
		echo "ℹ️  编译产物 $(TARGET) 已存在，跳过编译"; \
	else \
		$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) && echo "编译成功 ✅ $(TARGET)" || echo "编译出错 ❌"; \
	fi

clean: # 清理规则
	rm -fv $(TARGET)

install: $(TARGET) # 安装规则
	install -m 755 $(TARGET) /usr/local/bin/

uninstall: # 卸载规则
	rm -fv /usr/local/bin/$(TARGET)

help: # 帮助信息
	@echo "可用的 make 目标：ℹ️"
	@echo "  make           - 编译 iopp"
	@echo "  make clean     - 清理编译文件"
	@echo "  make install   - 安装到 /usr/local/bin"
	@echo "  make uninstall - 卸载 iopp"
	@echo "  make help      - 显示此帮助信息"