#!/usr/bin/env node
/**
 * iopp - I/O Process Monitor (Node.js version)
 * Multi-language support for Linux I/O monitoring
 */

const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

const VERSION = '0.0.1';
const VERSION_DATE = '2026.06.29';
const PROC_PATH = '/proc';

// Language constants
const LANG_ZH_CN = 0;
const LANG_ZH_TW = 1;
const LANG_EN = 2;

// Format constants
const FMT_EMOJI = 0;
const FMT_TEXT = 1;

// Language strings
const LANG_STRINGS = [
  // Simplified Chinese
  {
    err_buf: '错误 - 值大于缓冲区',
    warn_path: '警告 - 文件名长度可能超出缓冲区',
    err_num: "错误：'%s' 不是有效的数字参数",
    label_pid: '进程ID',
    label_rchar: '读字符',
    label_wchar: '写字符',
    label_syscr: '系统读',
    label_syscw: '系统写',
    label_read: '读字节',
    label_write: '写字节',
    label_cancel: '取消写',
    label_cmd: '命令行',
    usage_title: '用法',
    opt_command: '-c, --command 显示完整命令行',
    opt_help: '-h, --help 显示帮助',
    opt_idle: '-i, --idle 隐藏空闲进程',
    opt_kilobytes: '-k, --kilobytes 以千字节显示数据',
    opt_megabytes: '-m, --megabytes 以兆字节显示数据',
    opt_human: '-u, --human-readable 自动调整显示单位',
    opt_version: '-v, --version 显示版本信息',
    opt_lang: '-L, --lang <zh|tw|en> 设置语言 (默认: zh)',
    opt_format: '-e, --emoji 使用表情符 (默认) | -t, --text 纯文本',
    ver_info: 'iopp 版本',
    lang_setting: '语言: 简体中文',
    fmt_setting: '格式: 表情符',
    read_kb: '读KB',
    write_kb: '写KB',
    cancel_kb: '取消KB',
    read_mb: '读MB',
    write_mb: '写MB',
    cancel_mb: '取消MB',
  },
  // Traditional Chinese
  {
    err_buf: '錯誤 - 值大於緩衝區',
    warn_path: '警告 - 檔案名稱長度可能超出緩衝區',
    err_num: "錯誤：'%s' 不是有效的數字參數",
    label_pid: '進程ID',
    label_rchar: '讀字符',
    label_wchar: '寫字符',
    label_syscr: '系統讀',
    label_syscw: '系統寫',
    label_read: '讀位元組',
    label_write: '寫位元組',
    label_cancel: '取消寫',
    label_cmd: '命令列',
    usage_title: '用法',
    opt_command: '-c, --command 顯示完整命令列',
    opt_help: '-h, --help 顯示說明',
    opt_idle: '-i, --idle 隱藏空閒進程',
    opt_kilobytes: '-k, --kilobytes 以千位元組顯示資料',
    opt_megabytes: '-m, --megabytes 以兆位元組顯示資料',
    opt_human: '-u, --human-readable 自動調整顯示單位',
    opt_version: '-v, --version 顯示版本資訊',
    opt_lang: '-L, --lang <zh|tw|en> 設定語言 (預設: zh)',
    opt_format: '-e, --emoji 使用表情符 (預設) | -t, --text 純文字',
    ver_info: 'iopp 版本',
    lang_setting: '語言: 繁體中文',
    fmt_setting: '格式: 表情符',
    read_kb: '讀KB',
    write_kb: '寫KB',
    cancel_kb: '取消KB',
    read_mb: '讀MB',
    write_mb: '寫MB',
    cancel_mb: '取消MB',
  },
  // English
  {
    err_buf: 'Error - value exceeds buffer',
    warn_path: 'Warning - filename may exceed buffer',
    err_num: "Error: '%s' is not a valid numeric argument",
    label_pid: 'PID',
    label_rchar: 'RCHAR',
    label_wchar: 'WCHAR',
    label_syscr: 'SYSCR',
    label_syscw: 'SYSCW',
    label_read: 'READ',
    label_write: 'WRITE',
    label_cancel: 'CANCEL',
    label_cmd: 'COMMAND',
    usage_title: 'Usage',
    opt_command: '-c, --command Show full command line',
    opt_help: '-h, --help Show this help message',
    opt_idle: '-i, --idle Hide idle processes',
    opt_kilobytes: '-k, --kilobytes Display data in kilobytes',
    opt_megabytes: '-m, --megabytes Display data in megabytes',
    opt_human: '-u, --human-readable Auto-scale display units',
    opt_version: '-v, --version Show version information',
    opt_lang: '-L, --lang <zh|tw|en> Set language (default: zh)',
    opt_format: '-e, --emoji Use emoji (default) | -t, --text Plain text',
    ver_info: 'iopp version',
    lang_setting: 'Language: English',
    fmt_setting: 'Format: Emoji',
    read_kb: 'READ_KB',
    write_kb: 'WRITE_KB',
    cancel_kb: 'CANCEL_KB',
    read_mb: 'READ_MB',
    write_mb: 'WRITE_MB',
    cancel_mb: 'CANCEL_MB',
  },
];

class IOData {
  constructor(pid) {
    this.pid = pid;
    this.rchar = 0;
    this.wchar = 0;
    this.syscr = 0;
    this.syscw = 0;
    this.read_bytes = 0;
    this.write_bytes = 0;
    this.cancelled_write_bytes = 0;
    this.command = '';
  }
}

function parseLang(code) {
  switch (code) {
    case 'zh': return LANG_ZH_CN;
    case 'tw': return LANG_ZH_TW;
    case 'en': return LANG_EN;
    default: return LANG_ZH_CN;
  }
}

function readFile(filePath) {
  try {
    return fs.readFileSync(filePath, 'utf8');
  } catch (err) {
    return null;
  }
}

function getProcessIO(pid) {
  const ioPath = path.join(PROC_PATH, String(pid), 'io');
  const content = readFile(ioPath);
  if (!content) return null;

  const io = new IOData(pid);
  const lines = content.trim().split('\n');

  for (const line of lines) {
    const colonIdx = line.indexOf(':');
    if (colonIdx === -1) continue;

    const key = line.substring(0, colonIdx).trim();
    const val = parseInt(line.substring(colonIdx + 1).trim(), 10);

    switch (key) {
      case 'rchar': io.rchar = val; break;
      case 'wchar': io.wchar = val; break;
      case 'syscr': io.syscr = val; break;
      case 'syscw': io.syscw = val; break;
      case 'read_bytes': io.read_bytes = val; break;
      case 'write_bytes': io.write_bytes = val; break;
      case 'cancelled_write_bytes': io.cancelled_write_bytes = val; break;
    }
  }

  return io;
}

function getCommand(pid, fullCmd) {
  if (fullCmd) {
    const cmdPath = path.join(PROC_PATH, String(pid), 'cmdline');
    const content = readFile(cmdPath);
    return content ? content.replace(/\x00/g, ' ') : '?';
  }

  const statPath = path.join(PROC_PATH, String(pid), 'stat');
  const content = readFile(statPath);
  if (!content) return '?';

  const start = content.lastIndexOf('(');
  const end = content.lastIndexOf(')');
  if (start !== -1 && end !== -1 && end > start) {
    return content.substring(start + 1, end);
  }
  return '?';
}

function collectProcesses(command) {
  const results = [];

  try {
    const entries = fs.readdirSync(PROC_PATH, { withFileTypes: true });

    for (const entry of entries) {
      if (!entry.isDirectory()) continue;
      const pid = parseInt(entry.name, 10);
      if (isNaN(pid)) continue;

      const io = getProcessIO(pid);
      if (io) {
        io.command = getCommand(pid, command);
        results.push(io);
      }
    }
  } catch (err) {
    console.error('Error collecting processes:', err.message);
  }

  results.sort((a, b) => a.pid - b.pid);
  return results;
}

function formatBytes(amt) {
  if (amt >= 10000) {
    amt = Math.floor((amt + 512) / 1024);
    let tag = 'K';
    if (amt >= 10000) {
      amt = Math.floor((amt + 512) / 1024);
      tag = 'M';
      if (amt >= 10000) {
        amt = Math.floor((amt + 512) / 1024);
        tag = 'G';
      }
    }
    return `${amt.toString().padStart(5)}${tag}`;
  }
  return `${amt.toString().padStart(5)}B`;
}

function printHeader(args, lang) {
  if (args.human) {
    console.log(
      `${lang.label_pid.padStart(5)} ${lang.label_rchar.padStart(6)} ${lang.label_wchar.padStart(6)} ${lang.label_syscr.padStart(8)} ${lang.label_syscw.padStart(8)} ${lang.label_read.padStart(6)} ${lang.label_write.padStart(6)} ${lang.label_cancel.padStart(6)} ${lang.label_cmd.padEnd(20)}`
    );
  } else if (args.kilobytes) {
    console.log(
      `${lang.label_pid.padStart(5)} ${lang.label_rchar.padStart(8)} ${lang.label_wchar.padStart(8)} ${lang.label_syscr.padStart(8)} ${lang.label_syscw.padStart(8)} ${lang.read_kb.padStart(8)} ${lang.write_kb.padStart(8)} ${lang.cancel_kb.padStart(8)} ${lang.label_cmd}`
    );
  } else if (args.megabytes) {
    console.log(
      `${lang.label_pid.padStart(5)} ${lang.label_rchar.padStart(8)} ${lang.label_wchar.padStart(8)} ${lang.label_syscr.padStart(8)} ${lang.label_syscw.padStart(8)} ${lang.read_mb.padStart(8)} ${lang.write_mb.padStart(8)} ${lang.cancel_mb.padStart(8)} ${lang.label_cmd}`
    );
  } else {
    console.log(
      `${lang.label_pid.padStart(5)} ${lang.label_rchar.padStart(8)} ${lang.label_wchar.padStart(8)} ${lang.label_syscr.padStart(8)} ${lang.label_syscw.padStart(8)} ${lang.label_read.padStart(8)} ${lang.label_write.padStart(8)} ${lang.label_cancel.padStart(8)} ${lang.label_cmd}`
    );
  }
}

function printStats(args, processes, prevData, lang) {
  for (const io of processes) {
    const prev = prevData.get(io.pid);

    let rchar = 0, wchar = 0, syscr = 0, syscw = 0;
    let read_bytes = 0, write_bytes = 0, cancelled = 0;

    if (prev) {
      rchar = io.rchar - prev.rchar;
      wchar = io.wchar - prev.wchar;
      syscr = io.syscr - prev.syscr;
      syscw = io.syscw - prev.syscw;
      read_bytes = io.read_bytes - prev.read_bytes;
      write_bytes = io.write_bytes - prev.write_bytes;
      cancelled = io.cancelled_write_bytes - prev.cancelled_write_bytes;
    }

    // Apply unit conversion
    if (args.kilobytes && !args.human) {
      rchar = Math.floor(rchar / 1024);
      wchar = Math.floor(wchar / 1024);
      syscr = Math.floor(syscr / 1024);
      syscw = Math.floor(syscw / 1024);
      read_bytes = Math.floor(read_bytes / 1024);
      write_bytes = Math.floor(write_bytes / 1024);
      cancelled = Math.floor(cancelled / 1024);
    } else if (args.megabytes && !args.human) {
      rchar = Math.floor(rchar / (1024 * 1024));
      wchar = Math.floor(wchar / (1024 * 1024));
      syscr = Math.floor(syscr / (1024 * 1024));
      syscw = Math.floor(syscw / (1024 * 1024));
      read_bytes = Math.floor(read_bytes / (1024 * 1024));
      write_bytes = Math.floor(write_bytes / (1024 * 1024));
      cancelled = Math.floor(cancelled / (1024 * 1024));
    }

    // Skip idle processes
    if (args.idle && rchar === 0 && wchar === 0 && syscr === 0 &&
        syscw === 0 && read_bytes === 0 && write_bytes === 0 && cancelled === 0) {
      prevData.set(io.pid, io);
      continue;
    }

    if (args.human) {
      console.log(
        `${String(io.pid).padStart(5)} ${formatBytes(rchar).padStart(6)} ${formatBytes(wchar).padStart(6)} ${String(syscr).padStart(8)} ${String(syscw).padStart(8)} ${formatBytes(read_bytes).padStart(6)} ${formatBytes(write_bytes).padStart(6)} ${formatBytes(cancelled).padStart(6)} ${String(io.command).padEnd(20)}`
      );
    } else {
      console.log(
        `${String(io.pid).padStart(5)} ${String(rchar).padStart(8)} ${String(wchar).padStart(8)} ${String(syscr).padStart(8)} ${String(syscw).padStart(8)} ${String(read_bytes).padStart(8)} ${String(write_bytes).padStart(8)} ${String(cancelled).padStart(8)} ${io.command}`
      );
    }

    prevData.set(io.pid, io);
  }
}

function main() {
  const args = parseArgs();
  const lang = LANG_STRINGS[args.lang];
  const fmt = args.text ? FMT_TEXT : FMT_EMOJI;

  const errPrefix = fmt === FMT_EMOJI ? '✖' : '[ERROR]';
  const warnPrefix = fmt === FMT_EMOJI ? '⚠' : '[WARNING]';
  const infoPrefix = fmt === FMT_EMOJI ? 'ℹ' : '[INFO]';

  if (args.version) {
    console.log(`${infoPrefix}  ${lang.ver_info} ${VERSION} - ${VERSION_DATE}`);
    console.log(`    ${lang.lang_setting} | ${lang.fmt_setting}`);
    return 0;
  }

  if (args.help) {
    console.log(`${lang.usage_title}: iopp -h|--help ${infoPrefix}`);
    console.log(`${lang.usage_title}: iopp [-ci] [-k|-m] [-L lang] [-e|-t] [interval [count]]`);
    console.log(`            ${lang.opt_command}`);
    console.log(`            ${lang.opt_help}`);
    console.log(`            ${lang.opt_idle}`);
    console.log(`            ${lang.opt_kilobytes}`);
    console.log(`            ${lang.opt_megabytes}`);
    console.log(`            ${lang.opt_human}`);
    console.log(`            ${lang.opt_version}`);
    console.log(`            ${lang.opt_lang}`);
    console.log(`            ${lang.opt_format}`);
    return 0;
  }

  const interval = args.interval || 0;
  const count = args.count || 1;

  printHeader(args, lang);

  const prevData = new Map();
  let iteration = 0;

  while (iteration < count) {
    const processes = collectProcesses(args.command);

    if (iteration > 0) {
      printStats(args, processes, prevData, lang);
    } else {
      for (const io of processes) {
        prevData.set(io.pid, io);
      }
    }

    iteration++;
    if (iteration < count && interval > 0) {
      setTimeout(() => {}, interval * 1000);
    }
  }

  return 0;
}

function parseArgs() {
  const args = {
    command: false,
    idle: false,
    kilobytes: false,
    megabytes: false,
    human: false,
    version: false,
    help: false,
    lang: 'zh',
    emoji: false,
    text: false,
    interval: 0,
    count: 1,
  };

  const argv = process.argv.slice(2);
  let i = 0;

  while (i < argv.length) {
    const arg = argv[i];

    switch (arg) {
      case '-c':
      case '--command':
        args.command = true;
        break;
      case '-i':
      case '--idle':
        args.idle = true;
        break;
      case '-k':
      case '--kilobytes':
        args.kilobytes = true;
        break;
      case '-m':
      case '--megabytes':
        args.megabytes = true;
        break;
      case '-u':
      case '--human-readable':
        args.human = true;
        break;
      case '-v':
      case '--version':
        args.version = true;
        break;
      case '-h':
      case '--help':
        args.help = true;
        break;
      case '-L':
      case '--lang':
        i++;
        if (i < argv.length) {
          args.lang = argv[i];
        }
        break;
      case '-e':
      case '--emoji':
        args.emoji = true;
        break;
      case '-t':
      case '--text':
        args.text = true;
        break;
      default:
        if (!arg.startsWith('-')) {
          if (args.interval === 0) {
            args.interval = parseInt(arg, 10) || 0;
          } else {
            args.count = parseInt(arg, 10) || 1;
          }
        }
    }
    i++;
  }

  return args;
}

main();
