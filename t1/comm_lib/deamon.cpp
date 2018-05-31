#include "deamon.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define UNUSED(x) (void)(x)

static char *saved_pwd_path;
int change_to_deamon()
{
	saved_pwd_path = get_current_dir_name();
	signal(SIGHUP,SIG_IGN); 
	int pid;
//	int i;
	pid = fork();
	if(pid > 0)
		exit(0);//是父进程，结束父进程
	else if(pid< 0)
		exit(1);//fork失败，退出
//是第一子进程，后台继续执行
	setsid();//第一子进程成为新的会话组长和进程组长
//并与控制终端分离
	pid = fork();
	if(pid > 0)	
		exit(0);//是第一子进程，结束第一子进程
	else if(pid< 0)
		exit(1);//fork失败，退出
//是第二子进程，继续
//第二子进程不再是会话组长

	for(int i=0;i< 10;++i)//关闭打开的文件描述符
		close(i);
//	chdir("/tmp");//改变工作目录到/tmp
	umask(0);//重设文件创建掩模
	return (0);
}

uint64_t write_pid_file()
{
	char buf[64];
	
	int fd = open("pid.txt", O_CREAT | O_TRUNC | O_WRONLY, 0600);
//	if (!fd)
//		return;
	uint64_t pid;
	pid = getpid();
	sprintf(buf, "%lu", pid);
	size_t ret = write(fd, buf, strlen(buf));
	UNUSED(ret);
	close(fd);
	return pid;
}

int open_err_log_file()
{
	int fd = open("memerr.txt", O_CREAT | O_TRUNC | O_WRONLY, 0600);
	if (fd != 2)
	{
		dup2(fd, 2);
		close(fd);		
	}
	return (0);
}

	
