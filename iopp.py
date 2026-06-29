#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
iopp - I/O process monitoring tool (Python version)
Compatible with Linux, macOS, and other Unix-like systems

Usage: iopp [-ciu] [-k|-m] [-L lang] [-e|-t] [interval [count]]
"""

import os
import sys
import time
import argparse
import platform

# Version info
VERSION = "0.0.1"
VERSION_DATE = "2026.06.29"

# Language options
LANG_ZH_CN = 0
LANG_ZH_TW = 1
LANG_EN = 2

# Format options
FMT_EMOJI = 0
FMT_TEXT = 1

# Language strings
LANG_STRINGS = {
    LANG_ZH_CN: {
        "err_buf": "错误 - 值大于缓冲区",
        "warn_path": "警告 - 文件名长度可能超出缓冲区",
        "err_num": "错误：'%s' 不是有效的数字参数",
        "label_pid": "进程ID",
        "label_rchar": "读字符",
        "label_wchar": "写字符",
        "label_syscr": "系统读",
        "label_syscw": "系统写",
        "label_read": "读字节",
        "label_write": "写字节",
        "label_cancel": "取消写",
        "label_cmd": "命令行",
        "usage_title": "用法",
        "opt_command": "-c, --command 显示完整命令行",
        "opt_help": "-h, --help 显示帮助",
        "opt_idle": "-i, --idle 隐藏空闲进程",
        "opt_kilobytes": "-k, --kilobytes 以千字节显示数据",
        "opt_megabytes": "-m, --megabytes 以兆字节显示数据",
        "opt_human": "-u, --human-readable 自动调整显示单位",
        "opt_version": "-v, --version 显示版本信息",
        "opt_lang": "-L, --lang <zh|tw|en> 设置语言 (默认: zh)",
        "opt_format": "-e, --emoji 使用表情符 (默认) | -t, --text 纯文本",
        "ver_info": "iopp 版本",
        "lang_setting": "语言: 简体中文",
        "fmt_setting": "格式: 表情符",
        "read_kb": "读KB",
        "write_kb": "写KB",
        "cancel_kb": "取消KB",
        "read_mb": "读MB",
        "write_mb": "写MB",
        "cancel_mb": "取消MB",
    },
    LANG_ZH_TW: {
        "err_buf": "錯誤 - 值大於緩衝區",
        "warn_path": "警告 - 檔案名稱長度可能超出緩衝區",
        "err_num": "錯誤：'%s' 不是有效的數字參數",
        "label_pid": "進程ID",
        "label_rchar": "讀字符",
        "label_wchar": "寫字符",
        "label_syscr": "系統讀",
        "label_syscw": "系統寫",
        "label_read": "讀位元組",
        "label_write": "寫位元組",
        "label_cancel": "取消寫",
        "label_cmd": "命令列",
        "usage_title": "用法",
        "opt_command": "-c, --command 顯示完整命令列",
        "opt_help": "-h, --help 顯示說明",
        "opt_idle": "-i, --idle 隱藏空閒進程",
        "opt_kilobytes": "-k, --kilobytes 以千位元組顯示資料",
        "opt_megabytes": "-m, --megabytes 以兆位元組顯示資料",
        "opt_human": "-u, --human-readable 自動調整顯示單位",
        "opt_version": "-v, --version 顯示版本資訊",
        "opt_lang": "-L, --lang <zh|tw|en> 設定語言 (預設: zh)",
        "opt_format": "-e, --emoji 使用表情符 (預設) | -t, --text 純文字",
        "ver_info": "iopp 版本",
        "lang_setting": "語言: 繁體中文",
        "fmt_setting": "格式: 表情符",
        "read_kb": "讀KB",
        "write_kb": "寫KB",
        "cancel_kb": "取消KB",
        "read_mb": "讀MB",
        "write_mb": "寫MB",
        "cancel_mb": "取消MB",
    },
    LANG_EN: {
        "err_buf": "Error - value exceeds buffer",
        "warn_path": "Warning - filename may exceed buffer",
        "err_num": "Error: '%s' is not a valid numeric argument",
        "label_pid": "PID",
        "label_rchar": "RCHAR",
        "label_wchar": "WCHAR",
        "label_syscr": "SYSCR",
        "label_syscw": "SYSCW",
        "label_read": "READ",
        "label_write": "WRITE",
        "label_cancel": "CANCEL",
        "label_cmd": "COMMAND",
        "usage_title": "Usage",
        "opt_command": "-c, --command Show full command line",
        "opt_help": "-h, --help Show this help message",
        "opt_idle": "-i, --idle Hide idle processes",
        "opt_kilobytes": "-k, --kilobytes Display data in kilobytes",
        "opt_megabytes": "-m, --megabytes Display data in megabytes",
        "opt_human": "-u, --human-readable Auto-scale display units",
        "opt_version": "-v, --version Show version information",
        "opt_lang": "-L, --lang <zh|tw|en> Set language (default: zh)",
        "opt_format": "-e, --emoji Use emoji (default) | -t, --text Plain text",
        "ver_info": "iopp version",
        "lang_setting": "Language: English",
        "fmt_setting": "Format: Emoji",
        "read_kb": "READ_KB",
        "write_kb": "WRITE_KB",
        "cancel_kb": "CANCEL_KB",
        "read_mb": "READ_MB",
        "write_mb": "WRITE_MB",
        "cancel_mb": "CANCEL_MB",
    },
}

# Emoji and text prefixes
EMOJI_ERR = "✖"
EMOJI_WARN = "⚠"
EMOJI_INFO = "ℹ"

TEXT_ERR = "[ERROR]"
TEXT_WARN = "[WARNING]"
TEXT_INFO = "[INFO]"


class IOData:
    """Store I/O data for a process"""
    def __init__(self, pid, rchar=0, wchar=0, syscr=0, syscw=0,
                 read_bytes=0, write_bytes=0, cancelled_write_bytes=0, command=""):
        self.pid = pid
        self.rchar = rchar
        self.wchar = wchar
        self.syscr = syscr
        self.syscw = syscw
        self.read_bytes = read_bytes
        self.write_bytes = write_bytes
        self.cancelled_write_bytes = cancelled_write_bytes
        self.command = command


class IOPP:
    """I/O Process Monitor"""

    def __init__(self, args):
        self.args = args
        self.lang = self._get_lang(args.lang)
        self.fmt = FMT_EMOJI if args.emoji else FMT_TEXT
        self.prev_data = {}  # Previous I/O data for delta calculation
        self.proc_path = "/proc"  # Linux

        # Detect platform and set appropriate proc path
        system = platform.system()
        if system == "Linux":
            self.proc_path = "/proc"
        elif system == "Darwin":  # macOS
            # macOS doesn't have /proc, use ps command
            self.use_ps = True
        else:
            self.use_ps = False

    def _get_lang(self, lang_code):
        """Convert language code to constant"""
        if lang_code == "zh":
            return LANG_ZH_CN
        elif lang_code == "tw":
            return LANG_ZH_TW
        elif lang_code == "en":
            return LANG_EN
        return LANG_ZH_CN

    def _t(self, key):
        """Get translated string"""
        return LANG_STRINGS[self.lang].get(key, key)

    def _err_prefix(self):
        return EMOJI_ERR if self.fmt == FMT_EMOJI else TEXT_ERR

    def _warn_prefix(self):
        return EMOJI_WARN if self.fmt == FMT_EMOJI else TEXT_WARN

    def _info_prefix(self):
        return EMOJI_INFO if self.fmt == FMT_EMOJI else TEXT_INFO

    def _format_bytes(self, amt):
        """Format bytes with automatic unit scaling"""
        if amt >= 10000:
            amt = (amt + 512) // 1024
            tag = 'K'
            if amt >= 10000:
                amt = (amt + 512) // 1024
                tag = 'M'
                if amt >= 10000:
                    amt = (amt + 512) // 1024
                    tag = 'G'
            return f"{amt:5d}{tag}"
        return f"{amt:5d}B"

    def _format_number(self, num, width=8):
        """Format number for display"""
        return f"{num:{width}d}"

    def get_process_io_linux(self, pid):
        """Get I/O stats for a process on Linux"""
        try:
            io_path = f"{self.proc_path}/{pid}/io"
            with open(io_path, 'r') as f:
                io_data = {}
                for line in f:
                    line = line.strip()
                    if ':' in line:
                        key, value = line.split(':', 1)
                        io_data[key.strip()] = int(value.strip())
                return io_data
        except (FileNotFoundError, PermissionError, ValueError):
            return None

    def get_command_linux(self, pid, full_cmd=False):
        """Get command name for a process on Linux"""
        try:
            if full_cmd:
                cmd_path = f"{self.proc_path}/{pid}/cmdline"
                with open(cmd_path, 'r') as f:
                    cmdline = f.read().replace('\x00', ' ').strip()
                    return cmdline if cmdline else "?"
            else:
                stat_path = f"{self.proc_path}/{pid}/stat"
                with open(stat_path, 'r') as f:
                    stat = f.read()
                    # Extract command name from stat (between parentheses)
                    start = stat.rfind('(')
                    end = stat.rfind(')')
                    if start != -1 and end != -1:
                        return stat[start+1:end]
            return "?"
        except (FileNotFoundError, PermissionError):
            return "?"

    def get_process_io_ps(self):
        """Get I/O stats using ps command (for macOS and other systems)"""
        try:
            import subprocess
            # Use iostat friendly format
            result = subprocess.run(
                ['ps', '-axo', 'pid,rchar,wchar,syscr,syscw,comm'],
                capture_output=True, text=True
            )
            processes = {}
            lines = result.stdout.strip().split('\n')
            if len(lines) > 1:
                # Skip header
                for line in lines[1:]:
                    parts = line.split(None, 5)
                    if len(parts) >= 6:
                        try:
                            pid = int(parts[0])
                            processes[pid] = {
                                'rchar': int(parts[1]),
                                'wchar': int(parts[2]),
                                'syscr': int(parts[3]),
                                'syscw': int(parts[4]),
                                'command': parts[5][:20]
                            }
                        except ValueError:
                            continue
            return processes
        except Exception:
            return {}

    def collect_processes_linux(self):
        """Collect I/O data for all processes on Linux"""
        processes = []
        try:
            for entry in os.listdir(self.proc_path):
                if entry.isdigit():
                    pid = int(entry)
                    io_data = self.get_process_io_linux(pid)
                    if io_data:
                        command = self.get_command_linux(pid, self.args.command)
                        processes.append(IOData(
                            pid=pid,
                            rchar=io_data.get('rchar', 0),
                            wchar=io_data.get('wchar', 0),
                            syscr=io_data.get('syscr', 0),
                            syscw=io_data.get('syscw', 0),
                            read_bytes=io_data.get('read_bytes', 0),
                            write_bytes=io_data.get('write_bytes', 0),
                            cancelled_write_bytes=io_data.get('cancelled_write_bytes', 0),
                            command=command
                        ))
        except PermissionError:
            pass
        return processes

    def print_header(self):
        """Print column headers"""
        t = self._t
        if self.args.human:
            print(f"{'PID':>5} {t('label_rchar'):>6} {t('label_wchar'):>6} {t('label_syscr'):>8} "
                  f"{t('label_syscw'):>8} {t('label_read'):>6} {t('label_write'):>6} "
                  f"{t('label_cancel'):>6} {t('label_cmd'):<20}")
        elif self.args.kilobytes:
            print(f"{'PID':>5} {t('label_rchar'):>8} {t('label_wchar'):>8} {t('label_syscr'):>8} "
                  f"{t('label_syscw'):>8} {t('read_kb'):>8} {t('write_kb'):>8} "
                  f"{t('cancel_kb'):>8} {t('label_cmd')}")
        elif self.args.megabytes:
            print(f"{'PID':>5} {t('label_rchar'):>8} {t('label_wchar'):>8} {t('label_syscr'):>8} "
                  f"{t('label_syscw'):>8} {t('read_mb'):>8} {t('write_mb'):>8} "
                  f"{t('cancel_mb'):>8} {t('label_cmd')}")
        else:
            print(f"{'PID':>5} {t('label_rchar'):>8} {t('label_wchar'):>8} {t('label_syscr'):>8} "
                  f"{t('label_syscw'):>8} {t('label_read'):>8} {t('label_write'):>8} "
                  f"{t('label_cancel'):>8} {t('label_cmd')}")

    def print_stats(self, processes):
        """Print I/O statistics for processes"""
        t = self._t

        for ion in processes:
            # Calculate delta from previous reading
            prev = self.prev_data.get(ion.pid)

            if prev:
                rchar = ion.rchar - prev.rchar
                wchar = ion.wchar - prev.wchar
                syscr = ion.syscr - prev.syscr
                syscw = ion.syscw - prev.syscw
                read_bytes = ion.read_bytes - prev.read_bytes
                write_bytes = ion.write_bytes - prev.write_bytes
                cancelled = ion.cancelled_write_bytes - prev.cancelled_write_bytes
            else:
                rchar = wchar = syscr = syscw = read_bytes = write_bytes = cancelled = 0

            # Apply unit conversion
            if self.args.kilobytes and not self.args.human:
                rchar //= 1024
                wchar //= 1024
                syscr //= 1024
                syscw //= 1024
                read_bytes //= 1024
                write_bytes //= 1024
                cancelled //= 1024
            elif self.args.megabytes and not self.args.human:
                rchar //= (1024 * 1024)
                wchar //= (1024 * 1024)
                syscr //= (1024 * 1024)
                syscw //= (1024 * 1024)
                read_bytes //= (1024 * 1024)
                write_bytes //= (1024 * 1024)
                cancelled //= (1024 * 1024)

            # Skip idle processes if requested
            if self.args.idle and rchar == 0 and wchar == 0 and syscr == 0 and \
               syscw == 0 and read_bytes == 0 and write_bytes == 0 and cancelled == 0:
                continue

            if self.args.human:
                print(f"{ion.pid:5d} {self._format_bytes(rchar):>6} {self._format_bytes(wchar):>6} "
                      f"{self._format_number(syscr):>8} {self._format_number(syscw):>8} "
                      f"{self._format_bytes(read_bytes):>6} {self._format_bytes(write_bytes):>6} "
                      f"{self._format_bytes(cancelled):>6} {ion.command:<20}")
            else:
                print(f"{ion.pid:5d} {self._format_number(rchar):>8} {self._format_number(wchar):>8} "
                      f"{self._format_number(syscr):>8} {self._format_number(syscw):>8} "
                      f"{self._format_number(read_bytes):>8} {self._format_number(write_bytes):>8} "
                      f"{self._format_number(cancelled):>8} {ion.command}")

            # Store current data for next iteration
            self.prev_data[ion.pid] = IOData(
                ion.pid, ion.rchar, ion.wchar, ion.syscr, ion.syscw,
                ion.read_bytes, ion.write_bytes, ion.cancelled_write_bytes
            )

    def run(self):
        """Main execution loop"""
        if self.args.version:
            print(f"{self._info_prefix()}  {self._t('ver_info')} {VERSION} - {VERSION_DATE}")
            print(f"    {self._t('lang_setting')} | {self._t('fmt_setting')}")
            return 0

        # Check if running on Linux
        if platform.system() != "Linux":
            print(f"{self._warn_prefix()}  Warning: This tool is optimized for Linux. "
                  f"Current platform: {platform.system()}", file=sys.stderr)
            if platform.system() == "Darwin":
                print(f"{self._info_prefix()}  On macOS, some metrics may be limited.", file=sys.stderr)

        self.print_header()

        count = 0
        max_count = self.args.count if self.args.count > 0 else -1

        while max_count == -1 or count < max_count:
            if platform.system() == "Linux":
                processes = self.collect_processes_linux()
            else:
                processes = self.get_process_io_ps()

            if count == 0:
                # First iteration - just store data, don't print
                for ion in processes:
                    self.prev_data[ion.pid] = IOData(
                        ion.pid, ion.rchar, ion.wchar, ion.syscr, ion.syscw,
                        ion.read_bytes, ion.write_bytes, ion.cancelled_write_bytes
                    )
            else:
                self.print_stats(processes)

            count += 1
            if count < max_count or max_count == -1:
                time.sleep(self.args.interval)

        return 0


def main():
    parser = argparse.ArgumentParser(
        description="I/O process monitoring tool",
        add_help=False,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument('-c', '--command', action='store_true',
                        help='Show full command line')
    parser.add_argument('-h', '--help', action='store_true',
                        help='Show this help message')
    parser.add_argument('-i', '--idle', action='store_true',
                        help='Hide idle processes')
    parser.add_argument('-k', '--kilobytes', action='store_true',
                        help='Display data in kilobytes')
    parser.add_argument('-m', '--megabytes', action='store_true',
                        help='Display data in megabytes')
    parser.add_argument('-u', '--human-readable', action='store_true',
                        help='Auto-scale display units')
    parser.add_argument('-v', '--version', action='store_true',
                        help='Show version information')
    parser.add_argument('-L', '--lang', choices=['zh', 'tw', 'en'], default='zh',
                        help='Set language (default: zh)')
    parser.add_argument('-e', '--emoji', action='store_true', default=True,
                        help='Use emoji format (default)')
    parser.add_argument('-t', '--text', action='store_true',
                        help='Use plain text format')
    parser.add_argument('interval', nargs='?', type=int, default=0,
                        help='Interval between updates (seconds)')
    parser.add_argument('count', nargs='?', type=int, default=1,
                        help='Number of updates to show')

    args = parser.parse_args()

    if args.help:
        t_demo = IOPP(args)
        print(f"{t_demo._t('usage_title')}: iopp -h|--help {t_demo._info_prefix()}")
        print(f"{t_demo._t('usage_title')}: iopp [-ci] [-k|-m] [-L lang] [-e|-t] [interval [count]]")
        print(f"            {t_demo._t('opt_command')}")
        print(f"            {t_demo._t('opt_help')}")
        print(f"            {t_demo._t('opt_idle')}")
        print(f"            {t_demo._t('opt_kilobytes')}")
        print(f"            {t_demo._t('opt_megabytes')}")
        print(f"            {t_demo._t('opt_human')}")
        print(f"            {t_demo._t('opt_version')}")
        print(f"            {t_demo._t('opt_lang')}")
        print(f"            {t_demo._t('opt_format')}")
        return 0

    # Set emoji default if neither -e nor -t specified
    if not args.emoji and not args.text:
        args.emoji = True

    app = IOPP(args)
    return app.run()


if __name__ == '__main__':
    sys.exit(main())
