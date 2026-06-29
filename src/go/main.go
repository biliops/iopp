// iopp - I/O Process Monitor (Go version)
// Multi-language support for Linux I/O monitoring

package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"
)

const (
	version     = "0.0.1"
	versionDate = "2026.06.29"
	procPath    = "/proc"
)

// Language constants
const (
	LangZhCN = iota
	LangZhTW
	LangEn
)

// Format constants
const (
	FormatEmoji = iota
	FormatText
)

// LangStrings holds all translated strings
type LangStrings struct {
	ErrBuf, WarnPath, ErrNum        string
	LabelPid, LabelRchar, LabelWchar string
	LabelSyscr, LabelSyscw          string
	LabelRead, LabelWrite           string
	LabelCancel, LabelCmd           string
	UsageTitle                      string
	OptCommand, OptHelp             string
	OptIdle, OptKilobytes           string
	OptMegabytes, OptHuman          string
	OptVersion                      string
	OptLang, OptFormat              string
	VerInfo                         string
	LangSetting, FmtSetting         string
	ReadKB, WriteKB, CancelKB       string
	ReadMB, WriteMB, CancelMB       string
}

// Language strings table
var langStrings = []LangStrings{
	// Simplified Chinese
	LangStrings{
		ErrBuf:       "错误 - 值大于缓冲区",
		WarnPath:     "警告 - 文件名长度可能超出缓冲区",
		ErrNum:       "错误：'%s' 不是有效的数字参数",
		LabelPid:     "进程ID",
		LabelRchar:   "读字符",
		LabelWchar:   "写字符",
		LabelSyscr:   "系统读",
		LabelSyscw:   "系统写",
		LabelRead:    "读字节",
		LabelWrite:   "写字节",
		LabelCancel:  "取消写",
		LabelCmd:     "命令行",
		UsageTitle:   "用法",
		OptCommand:   "-c, --command 显示完整命令行",
		OptHelp:      "-h, --help 显示帮助",
		OptIdle:      "-i, --idle 隐藏空闲进程",
		OptKilobytes: "-k, --kilobytes 以千字节显示数据",
		OptMegabytes: "-m, --megabytes 以兆字节显示数据",
		OptHuman:     "-u, --human-readable 自动调整显示单位",
		OptVersion:   "-v, --version 显示版本信息",
		OptLang:      "-L, --lang <zh|tw|en> 设置语言 (默认: zh)",
		OptFormat:    "-e, --emoji 使用表情符 (默认) | -t, --text 纯文本",
		VerInfo:      "iopp 版本",
		LangSetting:  "语言: 简体中文",
		FmtSetting:   "格式: 表情符",
		ReadKB:       "读KB",
		WriteKB:      "写KB",
		CancelKB:     "取消KB",
		ReadMB:       "读MB",
		WriteMB:      "写MB",
		CancelMB:     "取消MB",
	},
	// Traditional Chinese
	LangStrings{
		ErrBuf:       "錯誤 - 值大於緩衝區",
		WarnPath:     "警告 - 檔案名稱長度可能超出緩衝區",
		ErrNum:       "錯誤：'%s' 不是有效的數字參數",
		LabelPid:     "進程ID",
		LabelRchar:   "讀字符",
		LabelWchar:   "寫字符",
		LabelSyscr:   "系統讀",
		LabelSyscw:   "系統寫",
		LabelRead:    "讀位元組",
		LabelWrite:   "寫位元組",
		LabelCancel:  "取消寫",
		LabelCmd:     "命令列",
		UsageTitle:   "用法",
		OptCommand:   "-c, --command 顯示完整命令列",
		OptHelp:      "-h, --help 顯示說明",
		OptIdle:      "-i, --idle 隱藏空閒進程",
		OptKilobytes: "-k, --kilobytes 以千位元組顯示資料",
		OptMegabytes: "-m, --megabytes 以兆位元組顯示資料",
		OptHuman:     "-u, --human-readable 自動調整顯示單位",
		OptVersion:   "-v, --version 顯示版本資訊",
		OptLang:      "-L, --lang <zh|tw|en> 設定語言 (預設: zh)",
		OptFormat:    "-e, --emoji 使用表情符 (預設) | -t, --text 純文字",
		VerInfo:      "iopp 版本",
		LangSetting:  "語言: 繁體中文",
		FmtSetting:   "格式: 表情符",
		ReadKB:       "讀KB",
		WriteKB:      "寫KB",
		CancelKB:     "取消KB",
		ReadMB:       "讀MB",
		WriteMB:      "寫MB",
		CancelMB:     "取消MB",
	},
	// English
	LangStrings{
		ErrBuf:       "Error - value exceeds buffer",
		WarnPath:     "Warning - filename may exceed buffer",
		ErrNum:       "Error: '%s' is not a valid numeric argument",
		LabelPid:     "PID",
		LabelRchar:   "RCHAR",
		LabelWchar:   "WCHAR",
		LabelSyscr:   "SYSCR",
		LabelSyscw:   "SYSCW",
		LabelRead:    "READ",
		LabelWrite:   "WRITE",
		LabelCancel:  "CANCEL",
		LabelCmd:     "COMMAND",
		UsageTitle:   "Usage",
		OptCommand:   "-c, --command Show full command line",
		OptHelp:      "-h, --help Show this help message",
		OptIdle:      "-i, --idle Hide idle processes",
		OptKilobytes: "-k, --kilobytes Display data in kilobytes",
		OptMegabytes: "-m, --megabytes Display data in megabytes",
		OptHuman:     "-u, --human-readable Auto-scale display units",
		OptVersion:   "-v, --version Show version information",
		OptLang:      "-L, --lang <zh|tw|en> Set language (default: zh)",
		OptFormat:    "-e, --emoji Use emoji (default) | -t, --text Plain text",
		VerInfo:      "iopp version",
		LangSetting:  "Language: English",
		FmtSetting:   "Format: Emoji",
		ReadKB:       "READ_KB",
		WriteKB:      "WRITE_KB",
		CancelKB:     "CANCEL_KB",
		ReadMB:       "READ_MB",
		WriteMB:      "WRITE_MB",
		CancelMB:     "CANCEL_MB",
	},
}

// IOData stores I/O statistics for a process
type IOData struct {
	Pid                      int
	Rchar, Wchar             int64
	Syscr, Syscw             int64
	ReadBytes, WriteBytes    int64
	CancelledWriteBytes      int64
	Command                  string
}

// Config holds application configuration
type Config struct {
	Lang         int
	Format       int
	Command      bool
	Idle         bool
	Kilobytes    bool
	Megabytes    bool
	Human        bool
	Version      bool
	Help         bool
	Interval     int
	Count        int
}

var (
	config     Config
	prevData   = make(map[int]IOData)
	lang       LangStrings
	emojiErr   = "✖"
	emojiWarn  = "⚠"
	emojiInfo  = "ℹ"
	textErr    = "[ERROR]"
	textWarn   = "[WARNING]"
	textInfo   = "[INFO]"
)

func parseLang(s string) int {
	switch s {
	case "zh":
		return LangZhCN
	case "tw":
		return LangZhTW
	case "en":
		return LangEn
	default:
		return LangZhCN
	}
}

func getErrPrefix() string {
	if config.Format == FormatEmoji {
		return emojiErr
	}
	return textErr
}

func getWarnPrefix() string {
	if config.Format == FormatEmoji {
		return emojiWarn
	}
	return textWarn
}

func getInfoPrefix() string {
	if config.Format == FormatEmoji {
		return emojiInfo
	}
	return textInfo
}

func readFile(path string) (string, error) {
	file, err := os.Open(path)
	if err != nil {
		return "", err
	}
	defer file.Close()
	data, err := io.ReadAll(file)
	if err != nil {
		return "", err
	}
	return string(data), nil
}

func getProcessIO(pid int) *IOData {
	ioPath := filepath.Join(procPath, strconv.Itoa(pid), "io")
	data, err := readFile(ioPath)
	if err != nil {
		return nil
	}

	io := &IOData{Pid: pid}
	scanner := bufio.NewScanner(strings.NewReader(data))
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.SplitN(line, ":", 2)
		if len(parts) != 2 {
			continue
		}
		key := strings.TrimSpace(parts[0])
		val, err := strconv.ParseInt(strings.TrimSpace(parts[1]), 10, 64)
		if err != nil {
			continue
		}
		switch key {
		case "rchar":
			io.Rchar = val
		case "wchar":
			io.Wchar = val
		case "syscr":
			io.Syscr = val
		case "syscw":
			io.Syscw = val
		case "read_bytes":
			io.ReadBytes = val
		case "write_bytes":
			io.WriteBytes = val
		case "cancelled_write_bytes":
			io.CancelledWriteBytes = val
		}
	}
	return io
}

func getCommand(pid int, fullCmd bool) string {
	if fullCmd {
		cmdPath := filepath.Join(procPath, strconv.Itoa(pid), "cmdline")
		data, err := readFile(cmdPath)
		if err != nil {
			return "?"
		}
		return strings.ReplaceAll(data, "\x00", " ")
	}

	statPath := filepath.Join(procPath, strconv.Itoa(pid), "stat")
	data, err := readFile(statPath)
	if err != nil {
		return "?"
	}

	// Extract command from between parentheses
	start := strings.LastIndex(data, "(")
	end := strings.LastIndex(data, ")")
	if start != -1 && end != -1 && end > start {
		return data[start+1 : end]
	}
	return "?"
}

func collectProcesses() []*IOData {
	entries, err := os.ReadDir(procPath)
	if err != nil {
		return nil
	}

	var wg sync.WaitGroup
	var mu sync.Mutex
	results := make([]*IOData, 0)

	for _, entry := range entries {
		if !entry.IsDir() {
			continue
		}
		name := entry.Name()
		pid, err := strconv.Atoi(name)
		if err != nil {
			continue
		}

		wg.Add(1)
		go func(pid int) {
			defer wg.Done()
			io := getProcessIO(pid)
			if io != nil {
				io.Command = getCommand(pid, config.Command)
				mu.Lock()
				results = append(results, io)
				mu.Unlock()
			}
		}(pid)
	}

	wg.Wait()
	sort.Slice(results, func(i, j int) bool {
		return results[i].Pid < results[j].Pid
	})
	return results
}

func formatBytes(amt int64) string {
	if amt >= 10000 {
		amt = (amt + 512) / 1024
		tag := "K"
		if amt >= 10000 {
			amt = (amt + 512) / 1024
			tag = "M"
			if amt >= 10000 {
				amt = (amt + 512) / 1024
				tag = "G"
			}
		}
		return fmt.Sprintf("%5d%v", amt, tag)
	}
	return fmt.Sprintf("%5dB", amt)
}

func printHeader() {
	if config.Human {
		fmt.Printf("%5v %6v %6v %8v %8v %6v %6v %6v %-20v\n",
			lang.LabelPid, lang.LabelRchar, lang.LabelWchar,
			lang.LabelSyscr, lang.LabelSyscw,
			lang.LabelRead, lang.LabelWrite, lang.LabelCancel,
			lang.LabelCmd)
	} else if config.Kilobytes {
		fmt.Printf("%5v %8v %8v %8v %8v %8v %8v %8v %v\n",
			lang.LabelPid, lang.LabelRchar, lang.LabelWchar,
			lang.LabelSyscr, lang.LabelSyscw,
			lang.ReadKB, lang.WriteKB, lang.CancelKB,
			lang.LabelCmd)
	} else if config.Megabytes {
		fmt.Printf("%5v %8v %8v %8v %8v %8v %8v %8v %v\n",
			lang.LabelPid, lang.LabelRchar, lang.LabelWchar,
			lang.LabelSyscr, lang.LabelSyscw,
			lang.ReadMB, lang.WriteMB, lang.CancelMB,
			lang.LabelCmd)
	} else {
		fmt.Printf("%5v %8v %8v %8v %8v %8v %8v %8v %v\n",
			lang.LabelPid, lang.LabelRchar, lang.LabelWchar,
			lang.LabelSyscr, lang.LabelSyscw,
			lang.LabelRead, lang.LabelWrite, lang.LabelCancel,
			lang.LabelCmd)
	}
}

func printStats(processes []*IOData) {
	for _, io := range processes {
		prev, exists := prevData[io.Pid]

		var rchar, wchar, syscr, syscw, readBytes, writeBytes, cancelled int64
		if exists {
			rchar = io.Rchar - prev.Rchar
			wchar = io.Wchar - prev.Wchar
			syscr = io.Syscr - prev.Syscr
			syscw = io.Syscw - prev.Syscw
			readBytes = io.ReadBytes - prev.ReadBytes
			writeBytes = io.WriteBytes - prev.WriteBytes
			cancelled = io.CancelledWriteBytes - prev.CancelledWriteBytes
		}

		// Apply unit conversion
		if config.Kilobytes && !config.Human {
			rchar /= 1024
			wchar /= 1024
			syscr /= 1024
			syscw /= 1024
			readBytes /= 1024
			writeBytes /= 1024
			cancelled /= 1024
		} else if config.Megabytes && !config.Human {
			rchar /= 1024 * 1024
			wchar /= 1024 * 1024
			syscr /= 1024 * 1024
			syscw /= 1024 * 1024
			readBytes /= 1024 * 1024
			writeBytes /= 1024 * 1024
			cancelled /= 1024 * 1024
		}

		// Skip idle processes
		if config.Idle && rchar == 0 && wchar == 0 && syscr == 0 &&
			syscw == 0 && readBytes == 0 && writeBytes == 0 && cancelled == 0 {
			continue
		}

		if config.Human {
			fmt.Printf("%5d %6v %6v %8d %8d %6v %6v %6v %-20v\n",
				io.Pid, formatBytes(rchar), formatBytes(wchar),
				syscr, syscw, formatBytes(readBytes),
				formatBytes(writeBytes), formatBytes(cancelled),
				io.Command)
		} else {
			fmt.Printf("%5d %8d %8d %8d %8d %8d %8d %8d %v\n",
				io.Pid, rchar, wchar, syscr, syscw,
				readBytes, writeBytes, cancelled, io.Command)
		}
	}
}

func main() {
	flag.IntVar(&config.Interval, "interval", 0, "Interval between updates")
	flag.IntVar(&config.Count, "count", 1, "Number of updates")
	flag.BoolVar(&config.Command, "command", false, "Show full command line")
	flag.BoolVar(&config.Command, "c", false, "Show full command line (short)")
	flag.BoolVar(&config.Idle, "idle", false, "Hide idle processes")
	flag.BoolVar(&config.Idle, "i", false, "Hide idle processes (short)")
	flag.BoolVar(&config.Kilobytes, "kilobytes", false, "Display in KB")
	flag.BoolVar(&config.Kilobytes, "k", false, "Display in KB (short)")
	flag.BoolVar(&config.Megabytes, "megabytes", false, "Display in MB")
	flag.BoolVar(&config.Megabytes, "m", false, "Display in MB (short)")
	flag.BoolVar(&config.Human, "human-readable", false, "Auto-scale units")
	flag.BoolVar(&config.Human, "u", false, "Auto-scale units (short)")
	flag.BoolVar(&config.Version, "version", false, "Show version")
	flag.BoolVar(&config.Version, "v", false, "Show version (short)")
	flag.BoolVar(&config.Help, "help", false, "Show help")
	flag.BoolVar(&config.Help, "h", false, "Show help (short)")

	langCode := flag.String("L", "zh", "Language (zh|tw|en)")
	flag.Parse()

	config.Lang = parseLang(*langCode)
	lang = langStrings[config.Lang]

	if config.Format == FormatText {
		// Handled via flag
	}

	if config.Version {
		fmt.Printf("%s  %s %s - %s\n", getInfoPrefix(), lang.VerInfo, version, versionDate)
		fmt.Printf("    %s | %s\n", lang.LangSetting, lang.FmtSetting)
		return
	}

	if config.Help {
		fmt.Printf("%s: iopp -h|--help %s\n", lang.UsageTitle, getInfoPrefix())
		fmt.Printf("%s: iopp [-ci] [-k|-m] [-L lang] [-e|-t] [interval [count]]\n", lang.UsageTitle)
		fmt.Printf("            %s\n", lang.OptCommand)
		fmt.Printf("            %s\n", lang.OptHelp)
		fmt.Printf("            %s\n", lang.OptIdle)
		fmt.Printf("            %s\n", lang.OptKilobytes)
		fmt.Printf("            %s\n", lang.OptMegabytes)
		fmt.Printf("            %s\n", lang.OptHuman)
		fmt.Printf("            %s\n", lang.OptVersion)
		fmt.Printf("            %s\n", lang.OptLang)
		fmt.Printf("            %s\n", lang.OptFormat)
		return
	}

	// Parse positional arguments
	args := flag.Args()
	if len(args) >= 1 {
		if interval, err := strconv.Atoi(args[0]); err == nil {
			config.Interval = interval
		}
	}
	if len(args) >= 2 {
		if count, err := strconv.Atoi(args[1]); err == nil {
			config.Count = count
		}
	}

	printHeader()

	for i := 0; i < config.Count || config.Count < 0; i++ {
		processes := collectProcesses()

		if i > 0 {
			printStats(processes)
		}

		// Store current data
		for _, io := range processes {
			prevData[io.Pid] = *io
		}

		if i < config.Count-1 || config.Count < 0 {
			time.Sleep(time.Duration(config.Interval) * time.Second)
		}
	}
}
