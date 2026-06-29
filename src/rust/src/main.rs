// iopp - I/O Process Monitor (Rust version)
// Multi-language support for Linux I/O monitoring

use clap::Parser;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use std::time::Duration;

const VERSION: &str = "0.0.1";
const VERSION_DATE: &str = "2026.06.29";
const PROC_PATH: &str = "/proc";

// Language constants
const LANG_ZH_CN: usize = 0;
const LANG_ZH_TW: usize = 1;
const LANG_EN: usize = 2;

// Format constants
const FMT_EMOJI: usize = 0;
const FMT_TEXT: usize = 1;

// Language strings struct
#[derive(Debug)]
struct LangStrings {
    err_buf: &'static str,
    warn_path: &'static str,
    err_num: &'static str,
    label_pid: &'static str,
    label_rchar: &'static str,
    label_wchar: &'static str,
    label_syscr: &'static str,
    label_syscw: &'static str,
    label_read: &'static str,
    label_write: &'static str,
    label_cancel: &'static str,
    label_cmd: &'static str,
    usage_title: &'static str,
    opt_command: &'static str,
    opt_help: &'static str,
    opt_idle: &'static str,
    opt_kilobytes: &'static str,
    opt_megabytes: &'static str,
    opt_human: &'static str,
    opt_version: &'static str,
    opt_lang: &'static str,
    opt_format: &'static str,
    ver_info: &'static str,
    lang_setting: &'static str,
    fmt_setting: &'static str,
    read_kb: &'static str,
    write_kb: &'static str,
    cancel_kb: &'static str,
    read_mb: &'static str,
    write_mb: &'static str,
    cancel_mb: &'static str,
}

const LANG_STRINGS: [LangStrings; 3] = [
    // Simplified Chinese
    LangStrings {
        err_buf: "错误 - 值大于缓冲区",
        warn_path: "警告 - 文件名长度可能超出缓冲区",
        err_num: "错误：'%s' 不是有效的数字参数",
        label_pid: "进程ID",
        label_rchar: "读字符",
        label_wchar: "写字符",
        label_syscr: "系统读",
        label_syscw: "系统写",
        label_read: "读字节",
        label_write: "写字节",
        label_cancel: "取消写",
        label_cmd: "命令行",
        usage_title: "用法",
        opt_command: "-c, --command 显示完整命令行",
        opt_help: "-h, --help 显示帮助",
        opt_idle: "-i, --idle 隐藏空闲进程",
        opt_kilobytes: "-k, --kilobytes 以千字节显示数据",
        opt_megabytes: "-m, --megabytes 以兆字节显示数据",
        opt_human: "-u, --human-readable 自动调整显示单位",
        opt_version: "-v, --version 显示版本信息",
        opt_lang: "-L, --lang <zh|tw|en> 设置语言 (默认: zh)",
        opt_format: "-e, --emoji 使用表情符 (默认) | -t, --text 纯文本",
        ver_info: "iopp 版本",
        lang_setting: "语言: 简体中文",
        fmt_setting: "格式: 表情符",
        read_kb: "读KB",
        write_kb: "写KB",
        cancel_kb: "取消KB",
        read_mb: "读MB",
        write_mb: "写MB",
        cancel_mb: "取消MB",
    },
    // Traditional Chinese
    LangStrings {
        err_buf: "錯誤 - 值大於緩衝區",
        warn_path: "警告 - 檔案名稱長度可能超出緩衝區",
        err_num: "錯誤：'%s' 不是有效的數字參數",
        label_pid: "進程ID",
        label_rchar: "讀字符",
        label_wchar: "寫字符",
        label_syscr: "系統讀",
        label_syscw: "系統寫",
        label_read: "讀位元組",
        label_write: "寫位元組",
        label_cancel: "取消寫",
        label_cmd: "命令列",
        usage_title: "用法",
        opt_command: "-c, --command 顯示完整命令列",
        opt_help: "-h, --help 顯示說明",
        opt_idle: "-i, --idle 隱藏空閒進程",
        opt_kilobytes: "-k, --kilobytes 以千位元組顯示資料",
        opt_megabytes: "-m, --megabytes 以兆位元組顯示資料",
        opt_human: "-u, --human-readable 自動調整顯示單位",
        opt_version: "-v, --version 顯示版本資訊",
        opt_lang: "-L, --lang <zh|tw|en> 設定語言 (預設: zh)",
        opt_format: "-e, --emoji 使用表情符 (預設) | -t, --text 純文字",
        ver_info: "iopp 版本",
        lang_setting: "語言: 繁體中文",
        fmt_setting: "格式: 表情符",
        read_kb: "讀KB",
        write_kb: "寫KB",
        cancel_kb: "取消KB",
        read_mb: "讀MB",
        write_mb: "寫MB",
        cancel_mb: "取消MB",
    },
    // English
    LangStrings {
        err_buf: "Error - value exceeds buffer",
        warn_path: "Warning - filename may exceed buffer",
        err_num: "Error: '%s' is not a valid numeric argument",
        label_pid: "PID",
        label_rchar: "RCHAR",
        label_wchar: "WCHAR",
        label_syscr: "SYSCR",
        label_syscw: "SYSCW",
        label_read: "READ",
        label_write: "WRITE",
        label_cancel: "CANCEL",
        label_cmd: "COMMAND",
        usage_title: "Usage",
        opt_command: "-c, --command Show full command line",
        opt_help: "-h, --help Show this help message",
        opt_idle: "-i, --idle Hide idle processes",
        opt_kilobytes: "-k, --kilobytes Display data in kilobytes",
        opt_megabytes: "-m, --megabytes Display data in megabytes",
        opt_human: "-u, --human-readable Auto-scale display units",
        opt_version: "-v, --version Show version information",
        opt_lang: "-L, --lang <zh|tw|en> Set language (default: zh)",
        opt_format: "-e, --emoji Use emoji (default) | -t, --text Plain text",
        ver_info: "iopp version",
        lang_setting: "Language: English",
        fmt_setting: "Format: Emoji",
        read_kb: "READ_KB",
        write_kb: "WRITE_KB",
        cancel_kb: "CANCEL_KB",
        read_mb: "READ_MB",
        write_mb: "WRITE_MB",
        cancel_mb: "CANCEL_MB",
    },
];

#[derive(Debug, Clone)]
struct IOData {
    pid: i32,
    rchar: i64,
    wchar: i64,
    syscr: i64,
    syscw: i64,
    read_bytes: i64,
    write_bytes: i64,
    cancelled_write_bytes: i64,
    command: String,
}

#[derive(Parser, Debug)]
#[command(name = "iopp")]
#[command(version = "0.0.1")]
struct Args {
    /// Show full command line
    #[arg(short, long)]
    command: bool,

    /// Hide idle processes
    #[arg(short, long)]
    idle: bool,

    /// Display data in kilobytes
    #[arg(short, long)]
    kilobytes: bool,

    /// Display data in megabytes
    #[arg(short, long)]
    megabytes: bool,

    /// Auto-scale display units
    #[arg(short, long)]
    human: bool,

    /// Show version information
    #[arg(short, long)]
    version: bool,

    /// Show help message
    #[arg(short, long)]
    help: bool,

    /// Set language (zh|tw|en)
    #[arg(short, long, default_value = "zh")]
    lang: String,

    /// Use emoji format (default)
    #[arg(short, long)]
    emoji: bool,

    /// Use plain text format
    #[arg(short, long)]
    text: bool,

    /// Interval between updates (seconds)
    interval: Option<u64>,

    /// Number of updates to show
    count: Option<i32>,
}

fn parse_lang(code: &str) -> usize {
    match code {
        "zh" => LANG_ZH_CN,
        "tw" => LANG_ZH_TW,
        "en" => LANG_EN,
        _ => LANG_ZH_CN,
    }
}

fn read_file(path: &str) -> Option<String> {
    fs::read_to_string(path).ok()
}

fn get_process_io(pid: i32) -> Option<IOData> {
    let io_path = format!("{}/{}/io", PROC_PATH, pid);
    let content = read_file(&io_path)?;

    let mut io = IOData {
        pid,
        rchar: 0,
        wchar: 0,
        syscr: 0,
        syscw: 0,
        read_bytes: 0,
        write_bytes: 0,
        cancelled_write_bytes: 0,
        command: String::new(),
    };

    for line in content.lines() {
        let parts: Vec<&str> = line.splitn(2, ':').collect();
        if parts.len() != 2 {
            continue;
        }
        let key = parts[0].trim();
        let val: i64 = parts[1].trim().parse().unwrap_or(0);

        match key {
            "rchar" => io.rchar = val,
            "wchar" => io.wchar = val,
            "syscr" => io.syscr = val,
            "syscw" => io.syscw = val,
            "read_bytes" => io.read_bytes = val,
            "write_bytes" => io.write_bytes = val,
            "cancelled_write_bytes" => io.cancelled_write_bytes = val,
            _ => {}
        }
    }

    Some(io)
}

fn get_command(pid: i32, full_cmd: bool) -> String {
    if full_cmd {
        let cmd_path = format!("{}/{}/cmdline", PROC_PATH, pid);
        if let Some(content) = read_file(&cmd_path) {
            return content.replace('\0', " ");
        }
    } else {
        let stat_path = format!("{}/{}/stat", PROC_PATH, pid);
        if let Some(content) = read_file(&stat_path) {
            // Extract command from between parentheses
            if let Some(start) = content.rfind('(') {
                if let Some(end) = content.rfind(')') {
                    if end > start {
                        return content[start + 1..end].to_string();
                    }
                }
            }
        }
    }
    "?".to_string()
}

fn collect_processes(command: bool) -> Vec<IOData> {
    let mut results = Vec::new();

    if let Ok(entries) = fs::read_dir(PROC_PATH) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_dir() {
                if let Some(name) = path.file_name().and_then(|n| n.to_str()) {
                    if let Ok(pid) = name.parse::<i32>() {
                        if let Some(mut io) = get_process_io(pid) {
                            io.command = get_command(pid, command);
                            results.push(io);
                        }
                    }
                }
            }
        }
    }

    results.sort_by_key(|io| io.pid);
    results
}

fn format_bytes(amt: i64) -> String {
    if amt >= 10000 {
        let mut amt = (amt + 512) / 1024;
        let mut tag = 'K';
        if amt >= 10000 {
            amt = (amt + 512) / 1024;
            tag = 'M';
            if amt >= 10000 {
                amt = (amt + 512) / 1024;
                tag = 'G';
            }
        }
        format!("{:5}{}", amt, tag)
    } else {
        format!("{:5}B", amt)
    }
}

fn print_header(args: &Args, lang: &LangStrings) {
    if args.human {
        println!(
            "{:>5} {:>6} {:>6} {:>8} {:>8} {:>6} {:>6} {:>6} {:<20}",
            lang.label_pid, lang.label_rchar, lang.label_wchar,
            lang.label_syscr, lang.label_syscw,
            lang.label_read, lang.label_write, lang.label_cancel,
            lang.label_cmd
        );
    } else if args.kilobytes {
        println!(
            "{:>5} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {}",
            lang.label_pid, lang.label_rchar, lang.label_wchar,
            lang.label_syscr, lang.label_syscw,
            lang.read_kb, lang.write_kb, lang.cancel_kb,
            lang.label_cmd
        );
    } else if args.megabytes {
        println!(
            "{:>5} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {}",
            lang.label_pid, lang.label_rchar, lang.label_wchar,
            lang.label_syscr, lang.label_syscw,
            lang.read_mb, lang.write_mb, lang.cancel_mb,
            lang.label_cmd
        );
    } else {
        println!(
            "{:>5} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {}",
            lang.label_pid, lang.label_rchar, lang.label_wchar,
            lang.label_syscr, lang.label_syscw,
            lang.label_read, lang.label_write, lang.label_cancel,
            lang.label_cmd
        );
    }
}

fn print_stats(args: &Args, processes: &[IOData], prev_data: &mut HashMap<i32, IOData>, lang: &LangStrings) {
    for io in processes {
        let prev = prev_data.get(&io.pid);

        let mut rchar = 0i64;
        let mut wchar = 0i64;
        let mut syscr = 0i64;
        let mut syscw = 0i64;
        let mut read_bytes = 0i64;
        let mut write_bytes = 0i64;
        let mut cancelled = 0i64;

        if let Some(p) = prev {
            rchar = io.rchar - p.rchar;
            wchar = io.wchar - p.wchar;
            syscr = io.syscr - p.syscr;
            syscw = io.syscw - p.syscw;
            read_bytes = io.read_bytes - p.read_bytes;
            write_bytes = io.write_bytes - p.write_bytes;
            cancelled = io.cancelled_write_bytes - p.cancelled_write_bytes;
        }

        // Apply unit conversion
        if args.kilobytes && !args.human {
            rchar /= 1024;
            wchar /= 1024;
            syscr /= 1024;
            syscw /= 1024;
            read_bytes /= 1024;
            write_bytes /= 1024;
            cancelled /= 1024;
        } else if args.megabytes && !args.human {
            rchar /= 1024 * 1024;
            wchar /= 1024 * 1024;
            syscr /= 1024 * 1024;
            syscw /= 1024 * 1024;
            read_bytes /= 1024 * 1024;
            write_bytes /= 1024 * 1024;
            cancelled /= 1024 * 1024;
        }

        // Skip idle processes
        if args.idle && rchar == 0 && wchar == 0 && syscr == 0 &&
            syscw == 0 && read_bytes == 0 && write_bytes == 0 && cancelled == 0 {
            prev_data.insert(io.pid, io.clone());
            continue;
        }

        if args.human {
            println!(
                "{:>5} {:>6} {:>6} {:>8} {:>8} {:>6} {:>6} {:>6} {:<20}",
                io.pid,
                format_bytes(rchar),
                format_bytes(wchar),
                syscr,
                syscw,
                format_bytes(read_bytes),
                format_bytes(write_bytes),
                format_bytes(cancelled),
                io.command
            );
        } else {
            println!(
                "{:>5} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {:>8} {}",
                io.pid, rchar, wchar, syscr, syscw,
                read_bytes, write_bytes, cancelled, io.command
            );
        }

        prev_data.insert(io.pid, io.clone());
    }
}

fn main() {
    let args = Args::parse();
    let lang_idx = parse_lang(&args.lang);
    let lang = &LANG_STRINGS[lang_idx];

    let fmt = if args.text { FMT_TEXT } else { FMT_EMOJI };
    let err_prefix = if fmt == FMT_EMOJI { "✖" } else { "[ERROR]" };
    let warn_prefix = if fmt == FMT_EMOJI { "⚠" } else { "[WARNING]" };
    let info_prefix = if fmt == FMT_EMOJI { "ℹ" } else { "[INFO]" };

    if args.version {
        println!("{}  {} {} - {}", info_prefix, lang.ver_info, VERSION, VERSION_DATE);
        println!("    {} | {}", lang.lang_setting, lang.fmt_setting);
        return;
    }

    if args.help {
        println!("{}: iopp -h|--help {}", lang.usage_title, info_prefix);
        println!("{}: iopp [-ci] [-k|-m] [-L lang] [-e|-t] [interval [count]]", lang.usage_title);
        println!("            {}", lang.opt_command);
        println!("            {}", lang.opt_help);
        println!("            {}", lang.opt_idle);
        println!("            {}", lang.opt_kilobytes);
        println!("            {}", lang.opt_megabytes);
        println!("            {}", lang.opt_human);
        println!("            {}", lang.opt_version);
        println!("            {}", lang.opt_lang);
        println!("            {}", lang.opt_format);
        return;
    }

    let interval = args.interval.unwrap_or(0) as u64;
    let count = args.count.unwrap_or(1);

    print_header(&args, lang);

    let mut prev_data = HashMap::new();
    let mut iteration = 0;

    loop {
        let processes = collect_processes(args.command);

        if iteration > 0 {
            print_stats(&args, &processes, &mut prev_data, lang);
        } else {
            for io in &processes {
                prev_data.insert(io.pid, io.clone());
            }
        }

        iteration += 1;
        if iteration >= count as usize {
            break;
        }

        if interval > 0 {
            std::thread::sleep(Duration::from_secs(interval));
        }
    }
}
