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
#define VERSION "0.0.1"
#define VERSION_DATE "2026.06.26"

#define GET_VALUE(v) \
		p = strchr(p, ':'); \
		++p; \
		++p; \
		q = strchr(p, '\n'); \
		length = q - p; \
		if (length >= BUFFERLEN) \
		{ \
			printf("❌  错误 - 值大于缓冲区: %d\n", __LINE__); \
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
		printf("⚠️  警告 - 文件名长度可能超出缓冲区: %d\n",__LINE__);
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
		printf("⚠️  警告 - 文件名长度可能超出缓冲区: %d\n",__LINE__);
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
		printf("%5s %6s %6s %8s %8s %6s %6s %6s %-20s\n", "进程ID", "读字符", "写字符","系统读", "系统写", "读字节", "写字节", "取消写", "命令行");
	else if (kb_flag == 1)
		printf("%5s %8s %8s %8s %8s %8s %8s %8s %s\n", "进程ID", "读字符", "写字符","系统读", "系统写", "读KB", "写KB", "取消KB", "命令行");
	else if (mb_flag == 1)
		printf("%5s %8s %8s %8s %8s %8s %8s %8s %s\n", "进程ID", "读字符", "写字符","系统读", "系统写", "读MB", "写MB", "取消MB", "命令行");
	else
		printf("%5s %8s %8s %8s %8s %8s %8s %8s %s\n", "进程ID", "读字符", "写字符","系统读", "系统写", "读字节", "写字节", "取消写", "命令行");

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
			printf("⚠️  警告 - 文件名长度可能超出缓冲区: %d\n",__LINE__);
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
	printf("用法: iopp -h|--help  ℹ️\n");
	printf("用法: iopp [-ci] [-k|-m] [间隔时间 [次数]]\n");
	printf("            -c, --command 显示完整命令行\n");
	printf("            -h, --help 显示帮助\n");
	printf("            -i, --idle 隐藏空闲进程\n");
	printf("            -k, --kilobytes 以千字节显示数据\n");
	printf("            -m, --megabytes 以兆字节显示数据\n");
	printf("            -u, --human-readable 自动调整显示单位\n");
	printf("            -v, --version 显示版本信息\n");
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
				{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "chikmuv", long_options, &option_index);
		if (c == -1) { // 处理间隔时间和次数参数
			if (argc == optind)
				break; // 没有额外参数
			else if ((argc - optind) == 1) {
				if (!is_valid_number(argv[optind])) {
					printf("❌  错误：'%s' 不是有效的数字参数\n", argv[optind]);
					usage();
					return 3;
				}
				delay = atoi(argv[optind]);
				max_count = -1;
			}
			else if ((argc - optind) == 2) {
				if (!is_valid_number(argv[optind])) {
					printf("❌  错误：'%s' 不是有效的数字参数\n", argv[optind]);
					usage();
					return 3;
				}
				if (!is_valid_number(argv[optind + 1])) {
					printf("❌  错误：'%s' 不是有效的数字参数\n", argv[optind + 1]);
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
			printf("ℹ️  iopp Version %s - %s\n", VERSION, VERSION_DATE);
			return 0;
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