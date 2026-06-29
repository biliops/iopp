// 编译： gcc -o /usr/local/bin/iopp iopp.c
// 运行： iopp -ciu 5

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define PROC "/proc"
#define VERSION "0.0.2"
#define VERSION_DATE "2026.06.29"

// 语言选项
#define LANG_ZH_CN 0  // 简体中文
#define LANG_ZH_TW 1  // 中文繁体
#define LANG_EN    2  // 英文

// 输出格式选项
#define FMT_EMOJI 0   // 表情符
#define FMT_TEXT  1   // 纯文本

int current_lang = LANG_ZH_CN;  // 默认简体中文
int current_fmt = FMT_EMOJI;    // 默认表情符

// 多语言字符串结构体
typedef struct {
    const char *err_buf;        // 错误-值大于缓冲区
    const char *warn_path;      // 警告-文件名长度
    const char *err_num;        // 错误-不是有效数字
    const char *label_pid;      // 进程ID
    const char *label_rchar;    // 读字符
    const char *label_wchar;    // 写字符
    const char *label_syscr;    // 系统读
    const char *label_syscw;    // 系统写
    const char *label_read;     // 读字节/读KB/读MB
    const char *label_write;    // 写字节/写KB/写MB
    const char *label_cancel;   // 取消写
    const char *label_cmd;      // 命令行
    const char *usage_title;    // 用法
    const char *opt_command;    // -c, --command
    const char *opt_help;       // -h, --help
    const char *opt_idle;       // -i, --idle
    const char *opt_kilobytes;  // -k, --kilobytes
    const char *opt_megabytes;  // -m, --megabytes
    const char *opt_human;      // -u, --human-readable
    const char *opt_version;    // -v, --version
    const char *opt_lang;       // -L, --lang
    const char *opt_format;     // -e, --emoji / -t, --text
    const char *ver_info;       // 版本信息
    const char *lang_setting;   // 语言设置
    const char *fmt_setting;    // 格式设置
} lang_strings_t;

// 字符串表
static const lang_strings_t lang_strings[] = {
    // 简体中文 (LANG_ZH_CN)
    {
        .err_buf    = "错误 - 值大于缓冲区",
        .warn_path  = "警告 - 文件名长度可能超出缓冲区",
        .err_num    = "错误：'%s' 不是有效的数字参数",
        .label_pid  = "进程ID",
        .label_rchar= "读字符",
        .label_wchar= "写字符",
        .label_syscr= "系统读",
        .label_syscw= "系统写",
        .label_read = "读字节",
        .label_write= "写字节",
        .label_cancel= "取消写",
        .label_cmd  = "命令行",
        .usage_title= "用法",
        .opt_command= "-c, --command 显示完整命令行",
        .opt_help   = "-h, --help 显示帮助",
        .opt_idle   = "-i, --idle 隐藏空闲进程",
        .opt_kilobytes="-k, --kilobytes 以千字节显示数据",
        .opt_megabytes="-m, --megabytes 以兆字节显示数据",
        .opt_human  = "-u, --human-readable 自动调整显示单位",
        .opt_version="-v, --version 显示版本信息",
        .opt_lang   = "-L, --lang <zh|tw|en> 设置语言 (默认: zh)",
        .opt_format = "-e, --emoji 使用表情符 (默认) | -t, --text 纯文本",
        .ver_info   = "iopp 版本",
        .lang_setting="语言: 简体中文",
        .fmt_setting="格式: 表情符",
    },
    // 中文繁体 (LANG_ZH_TW)
    {
        .err_buf    = "錯誤 - 值大於緩衝區",
        .warn_path  = "警告 - 檔案名稱長度可能超出緩衝區",
        .err_num    = "錯誤：'%s' 不是有效的數字參數",
        .label_pid  = "進程ID",
        .label_rchar= "讀字符",
        .label_wchar= "寫字符",
        .label_syscr= "系統讀",
        .label_syscw= "系統寫",
        .label_read = "讀位元組",
        .label_write= "寫位元組",
        .label_cancel= "取消寫",
        .label_cmd  = "命令列",
        .usage_title= "用法",
        .opt_command= "-c, --command 顯示完整命令列",
        .opt_help   = "-h, --help 顯示說明",
        .opt_idle   = "-i, --idle 隱藏空閒進程",
        .opt_kilobytes="-k, --kilobytes 以千位元組顯示資料",
        .opt_megabytes="-m, --megabytes 以兆位元組顯示資料",
        .opt_human  = "-u, --human-readable 自動調整顯示單位",
        .opt_version="-v, --version 顯示版本資訊",
        .opt_lang   = "-L, --lang <zh|tw|en> 設定語言 (預設: zh)",
        .opt_format = "-e, --emoji 使用表情符 (預設) | -t, --text 純文字",
        .ver_info   = "iopp 版本",
        .lang_setting="語言: 繁體中文",
        .fmt_setting="格式: 表情符",
    },
    // 英文 (LANG_EN)
    {
        .err_buf    = "Error - value exceeds buffer",
        .warn_path  = "Warning - filename may exceed buffer",
        .err_num    = "Error: '%s' is not a valid numeric argument",
        .label_pid  = "PID",
        .label_rchar= "RCHAR",
        .label_wchar= "WCHAR",
        .label_syscr= "SYSCR",
        .label_syscw= "SYSCW",
        .label_read = "READ",
        .label_write= "WRITE",
        .label_cancel= "CANCEL",
        .label_cmd  = "COMMAND",
        .usage_title= "Usage",
        .opt_command= "-c, --command Show full command line",
        .opt_help   = "-h, --help Show this help message",
        .opt_idle   = "-i, --idle Hide idle processes",
        .opt_kilobytes="-k, --kilobytes Display data in kilobytes",
        .opt_megabytes="-m, --megabytes Display data in megabytes",
        .opt_human  = "-u, --human-readable Auto-scale display units",
        .opt_version="-v, --version Show version information",
        .opt_lang   = "-L, --lang <zh|tw|en> Set language (default: zh)",
        .opt_format = "-e, --emoji Use emoji (default) | -t, --text Plain text",
        .ver_info   = "iopp version",
        .lang_setting="Language: English",
        .fmt_setting="Format: Emoji",
    }
};

// 获取当前语言的字符串
static inline const lang_strings_t *LANG() {
    return &lang_strings[current_lang];
}

// 表情符定义
static const char *EMOJI_ERR = "✖";
static const char *EMOJI_WARN = "⚠";
static const char *EMOJI_INFO = "ℹ";

// 纯文本替代
static const char *TEXT_ERR = "[ERROR]";
static const char *TEXT_WARN = "[WARNING]";
static const char *TEXT_INFO = "[INFO]";

// 根据格式获取前缀
static inline const char *ERR_PREFIX() { return (current_fmt == FMT_EMOJI) ? EMOJI_ERR : TEXT_ERR; }
static inline const char *WARN_PREFIX() { return (current_fmt == FMT_EMOJI) ? EMOJI_WARN : TEXT_WARN; }
static inline const char *INFO_PREFIX() { return (current_fmt == FMT_EMOJI) ? EMOJI_INFO : TEXT_INFO; }

#define GET_VALUE(v) \
		p = strchr(p, ':'); \
		++p; \
		++p; \
		q = strchr(p, '\n'); \
		length = q - p; \
		if (length >= BUFFERLEN) \
		{ \
			printf("%s  %s: %d\n", ERR_PREFIX(), LANG()->err_buf, __LINE__); \
			exit(1); \
		} \
		strncpy(value, p, length); \
		value[length] = '\0'; \
		v = atoll(value);

#define BTOKB(b) b >> 10
#define BTOMB(b) b >> 20

#define BUFFERLEN 255
#define COMMANDLEN 1024
#define VALUELEN 63

#define NUM_STRINGS 8

struct io_node {
	int pid;
	long long rchar;
	long long wchar;
	long long syscr;
	long long syscw;
	long long read_bytes;
	long long write_bytes;
	long long cancelled_write_bytes;
	char command[COMMANDLEN + 1];
	struct io_node *next;
};

struct io_node *head = NULL;
int command_flag = 0;
int idle_flag = 0;
int mb_flag = 0;
int kb_flag = 0;
int hr_flag = 0;

// 函数声明
char *format_b(long long);
struct io_node *get_ion(int);
struct io_node *new_ion(char *);
void upsert_data(struct io_node *);
int is_valid_number(const char *str);

char * format_b(long long amt) {
	static char retarray[NUM_STRINGS][24];
	static int	index = 0;
	register char *ret;
	register char tag = 'B';

	ret = retarray[index];
	index = (index + 1) % NUM_STRINGS;

	if (amt >= 10000) {
		amt = (amt + 512) / 1024;
		tag = 'K';
		if (amt >= 10000) {
			amt = (amt + 512) / 1024;
			tag = 'M';
			if (amt >= 10000) {
				amt = (amt + 512) / 1024;
				tag = 'G';
			}
		}
	}

	snprintf(ret, sizeof(retarray[index]), "%5lld%c", amt, tag);

	return (ret);
}

int is_valid_number(const char *str) {
	if (!str || *str == '\0')
		return 0;
	if (*str == '-')
		str++;
	while (*str) {
		if (!isdigit(*str))
			return 0;
		str++;
	}
	return 1;
}

int get_cmdline(struct io_node *ion) {
	int fd;
	int length;
	char filename[BUFFERLEN + 1];
	char buffer[COMMANDLEN + 1];
	char *p;
	char *q;

	length = snprintf(filename, BUFFERLEN, "%s/%d/cmdline", PROC, ion->pid);
	if (length == BUFFERLEN)
		printf("%s  %s: %d\n", WARN_PREFIX(), LANG()->warn_path, __LINE__);
	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return 1;
	length = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	buffer[length] = '\0';
	if (length == 0)
		return 2;
	if (command_flag == 0) 	{
		p = strchr(buffer, '(');
		++p;
		q = strchr(p, ')');
		length = q - p;
	}
	else
		p = buffer;
	length = length < COMMANDLEN ? length : COMMANDLEN;
	strncpy(ion->command, p, length);
	ion->command[length] = '\0';
	return 0;
}

struct io_node * get_ion(int pid) {
	struct io_node *c = head;

	while (c != NULL) {
		if (c->pid == pid)
			break;
		c = c->next;
	}
	return c;
}

int get_tcomm(struct io_node *ion) {
	int fd;
	int length;
	char filename[BUFFERLEN + 1];
	char buffer[BUFFERLEN + 1];
	char *p;
	char *q;

	length = snprintf(filename, BUFFERLEN, "%s/%d/stat", PROC, ion->pid);
	if (length == BUFFERLEN)
		printf("%s  %s: %d\n", WARN_PREFIX(), LANG()->warn_path, __LINE__);
	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return 1;
	length = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	p = strchr(buffer, '(');
	++p;
	q = strchr(p, ')');
	length = q - p;
	length = length < BUFFERLEN ? length : BUFFERLEN;
	strncpy(ion->command, p, length);
	ion->command[length] = '\0';
	return 0;
}

struct io_node * insert_ion(struct io_node *ion) {
	struct io_node *c;
	struct io_node *p;

	if (ion->pid < head->pid) { // 将链表头作为特殊情况处理
		ion->next = head;
		head = ion;
		return head;
	}

	c = head->next;
	p = head;
	while (c != NULL) {
		if (ion->pid < c->pid) 	{
			ion->next = c;
			p->next = ion;
			return head;
		}
		p = c;
		c = c->next;
	}

	if (c == NULL) // 添加到链表末尾
		p->next = ion;

	return head;
}

void get_stats() {
	DIR *dir = opendir(PROC);
	struct dirent *ent;
	char filename[BUFFERLEN + 1];
	char buffer[BUFFERLEN + 1];

	char value[BUFFERLEN + 1];

	// 显示列标题
	if (hr_flag == 1)
		printf("%5s %6s %6s %8s %8s %6s %6s %6s %-20s\n",
			LANG()->label_pid, LANG()->label_rchar, LANG()->label_wchar,
			LANG()->label_syscr, LANG()->label_syscw,
			LANG()->label_read, LANG()->label_write, LANG()->label_cancel,
			LANG()->label_cmd);
	else if (kb_flag == 1)
		printf("%5s %8s %8s %8s %8s %8s %8s %8s %s\n",
			LANG()->label_pid, LANG()->label_rchar, LANG()->label_wchar,
			LANG()->label_syscr, LANG()->label_syscw,
			"读KB", "写KB", "取消KB",
			LANG()->label_cmd);
	else if (mb_flag == 1)
		printf("%5s %8s %8s %8s %8s %8s %8s %8s %s\n",
			LANG()->label_pid, LANG()->label_rchar, LANG()->label_wchar,
			LANG()->label_syscr, LANG()->label_syscw,
			"读MB", "写MB", "取消MB",
			LANG()->label_cmd);
	else
		printf("%5s %8s %8s %8s %8s %8s %8s %8s %s\n",
			LANG()->label_pid, LANG()->label_rchar, LANG()->label_wchar,
			LANG()->label_syscr, LANG()->label_syscw,
			LANG()->label_read, LANG()->label_write, LANG()->label_cancel,
			LANG()->label_cmd);

	while ((ent = readdir(dir)) != NULL) { // 遍历进程表并为每个进程显示一行
		int rc = 0;
		int fd;
		int length;

		char *p;
		char *q;

		struct io_node *ion;
		struct io_node *old_ion;

		long long rchar;
		long long wchar;
		long long syscr;
		long long syscw;
		long long read_bytes;
		long long write_bytes;
		long long cancelled_write_bytes;

		if (!isdigit(ent->d_name[0]))
			continue;

		ion = new_ion(ent->d_name);

		if (command_flag == 1)
			rc = get_cmdline(ion);
		if (command_flag == 0 || rc != 0) // 如果没有请求完整命令行或命令行为空...
			rc = get_tcomm(ion);

		if (rc != 0) {
			free(ion);
			continue;
		}

		// 读取 'io' 文件
		length = snprintf(filename, BUFFERLEN, "%s/%s/io", PROC, ent->d_name);
		if (length == BUFFERLEN)
			printf("%s  %s: %d\n", WARN_PREFIX(), LANG()->warn_path, __LINE__);
		fd = open(filename, O_RDONLY);
		if (fd == -1) {
			free(ion);
			continue;
		}
		length = read(fd, buffer, sizeof(buffer) - 1);
		close(fd);
		buffer[length] = '\0';

		// 解析 io 文件数据
		p = buffer;
		GET_VALUE(ion->rchar);
		GET_VALUE(ion->wchar);
		GET_VALUE(ion->syscr);
		GET_VALUE(ion->syscw);
		GET_VALUE(ion->read_bytes);
		GET_VALUE(ion->write_bytes);
		GET_VALUE(ion->cancelled_write_bytes);

		old_ion = get_ion(ion->pid);

		// 显示进程的IO数据
		if (old_ion != NULL) {
			rchar = ion->rchar - old_ion->rchar;
			wchar = ion->wchar - old_ion->wchar;
			syscr = ion->syscr - old_ion->syscr;
			syscw = ion->syscw - old_ion->syscw;
			read_bytes = ion->read_bytes - old_ion->read_bytes;
			write_bytes = ion->write_bytes - old_ion->write_bytes;
			cancelled_write_bytes = ion->cancelled_write_bytes - old_ion->cancelled_write_bytes;

			if (kb_flag == 1 && hr_flag == 0) {
				rchar = BTOKB(rchar);
				wchar = BTOKB(wchar);
				syscr = BTOKB(syscr);
				syscw = BTOKB(syscw);
				read_bytes = BTOKB(read_bytes);
				write_bytes = BTOKB(write_bytes);
				cancelled_write_bytes = BTOKB(cancelled_write_bytes);
			}
			else if (mb_flag == 1 && hr_flag == 0) {
				rchar = BTOMB(rchar);
				wchar = BTOMB(wchar);
				syscr = BTOMB(syscr);
				syscw = BTOMB(syscw);
				read_bytes = BTOMB(read_bytes);
				write_bytes = BTOMB(write_bytes);
				cancelled_write_bytes = BTOMB(cancelled_write_bytes);
			}

			if (!(idle_flag == 1 && rchar == 0 && wchar == 0 && syscr == 0 &&
					syscw == 0 && read_bytes == 0 && write_bytes == 0 &&
					cancelled_write_bytes == 0)) {
				if (hr_flag == 0)
					printf("%5d %8lld %8lld %8lld %8lld %8lld %8lld %8lld %s\n",
							ion->pid,
							rchar,
							wchar,
							syscr,
							syscw,
							read_bytes,
							write_bytes,
							cancelled_write_bytes,
							ion->command);
				else
					printf("%5d %6s %6s %8lld %8lld %6s %6s %6s %-20s\n",
							ion->pid,
							format_b(rchar),
							format_b(wchar),
							syscr,
							syscw,
							format_b(read_bytes),
							format_b(write_bytes),
							format_b(cancelled_write_bytes),
							ion->command);
			}
		}
		else if (idle_flag != 1) // 没有先前数据时，显示0而不是计算负数（仅当显示空闲进程时）
			printf("%5d %8d %8d %8d %8d %8d %8d %8d %s\n",
					ion->pid, 0, 0, 0, 0, 0, 0, 0, ion->command);

		upsert_data(ion);
	}
	closedir(dir);
	return;
}

struct io_node * new_ion(char *pid) {
	struct io_node *ion;

	ion = (struct io_node *) malloc(sizeof(struct io_node));
	bzero(ion, sizeof(struct io_node));
	ion->pid = atoi(pid);

	return ion;
}

void upsert_data(struct io_node *ion) {
	struct io_node *n;
	
	if (head == NULL) { // 链表为空
		head = ion;
		return;
	}

	n = head;
	while (n != NULL) { // 检查之前是否见过此进程ID
		if (n->pid == ion->pid) {
			n->rchar = ion->rchar;
			n->wchar = ion->wchar;
			n->syscr = ion->syscr;
			n->syscw = ion->syscw;
			n->read_bytes = ion->read_bytes;
			n->write_bytes = ion->write_bytes;
			n->cancelled_write_bytes = ion->cancelled_write_bytes;
			
			strcpy(n->command, ion->command); // 如果进程ID发生回绕，则命令可能与之前不同
			free(ion);
			return;
		}
		n = n->next;
	}

	head = insert_ion(ion); // 将此进程ID添加到链表
	return;
}

void usage() {
	printf("%s: iopp -h|--help %s\n", LANG()->usage_title, INFO_PREFIX());
	printf("%s: iopp [-ci] [-k|-m] [-L lang] [-e|-t] [interval [count]]\n", LANG()->usage_title);
	printf("            %s\n", LANG()->opt_command);
	printf("            %s\n", LANG()->opt_help);
	printf("            %s\n", LANG()->opt_idle);
	printf("            %s\n", LANG()->opt_kilobytes);
	printf("            %s\n", LANG()->opt_megabytes);
	printf("            %s\n", LANG()->opt_human);
	printf("            %s\n", LANG()->opt_version);
	printf("            %s\n", LANG()->opt_lang);
	printf("            %s\n", LANG()->opt_format);
}

int main(int argc, char *argv[]) {
	int c;
	int delay = 0;
	int count = 0;
	int max_count = 1;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
				{ "command", no_argument, 0, 'c' },
				{ "help", no_argument, 0, 'h' },
				{ "human-readable", no_argument, 0, 'u' },
				{ "idle", no_argument, 0, 'i' },
				{ "kilobytes", no_argument, 0, 'k' },
				{ "megabytes", no_argument, 0, 'm' },
				{ "version", no_argument, 0, 'v' },
				{ "lang", required_argument, 0, 'L' },
				{ "emoji", no_argument, 0, 'e' },
				{ "text", no_argument, 0, 't' },
				{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "chikmuvL:et", long_options, &option_index);
		if (c == -1) { // 处理间隔时间和次数参数
			if (argc == optind)
				break; // 没有额外参数
			else if ((argc - optind) == 1) {
				if (!is_valid_number(argv[optind])) {
					printf("%s  %s: '%s'\n", ERR_PREFIX(), LANG()->err_num, argv[optind]);
					usage();
					return 3;
				}
				delay = atoi(argv[optind]);
				max_count = -1;
			}
			else if ((argc - optind) == 2) {
				if (!is_valid_number(argv[optind])) {
					printf("%s  %s: '%s'\n", ERR_PREFIX(), LANG()->err_num, argv[optind]);
					usage();
					return 3;
				}
				if (!is_valid_number(argv[optind + 1])) {
					printf("%s  %s: '%s'\n", ERR_PREFIX(), LANG()->err_num, argv[optind + 1]);
					usage();
					return 3;
				}
				delay = atoi(argv[optind]);
				max_count = atoi(argv[optind + 1]);
			}
			else { // 额外参数过多
				usage();
				return 3;
			}
			break;
		}

		switch (c) {
		case 'c':
			command_flag = 1;
			break;
		case 'h':
			usage();
			return 0;
		case 'i':
			idle_flag = 1;
			break;
		case 'k':
			kb_flag = 1;
			break;
		case 'm':
			mb_flag = 1;
			break;
		case 'u':
			hr_flag = 1;
			break;
		case 'v':
			printf("%s  %s %s - %s\n", INFO_PREFIX(), LANG()->ver_info, VERSION, VERSION_DATE);
			printf("    %s | %s\n", LANG()->lang_setting, LANG()->fmt_setting);
			return 0;
		case 'L':
			if (optarg) {
				if (strcmp(optarg, "zh") == 0)
					current_lang = LANG_ZH_CN;
				else if (strcmp(optarg, "tw") == 0)
					current_lang = LANG_ZH_TW;
				else if (strcmp(optarg, "en") == 0)
					current_lang = LANG_EN;
				else {
					printf("%s  %s: '%s'\n", ERR_PREFIX(), "Invalid language, use: zh|tw|en", optarg);
					return 3;
				}
			}
			break;
		case 'e':
			current_fmt = FMT_EMOJI;
			break;
		case 't':
			current_fmt = FMT_TEXT;
			break;
		default:
			usage();
			return 2;
		}
	}

	while (max_count == -1 || count++ < max_count) 	{
		get_stats();
		if (count != max_count)
			sleep(delay);
	}
	return 0;
}