#include <unistd.h>
#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
int signal_flag;

void my_signal (int sig) {
	// Осталась лишь только смена флага
        switch(sig) {
			case SIGUSR1:
				signal_flag = 1;
				break; 
			case SIGINT:
				signal_flag = 2;
				break;
			default:
				break;
        }
}

void Daemon(char* data[], int size) {
	pid_t pid;
	int fd;

	while(1) {
		pause();
		switch(signal_flag) {
			case 1:
				exit(0);
			case 2: 
				if (pid = fork())
					break;
				else {
					// Исправил, теперь реально выделяю память
					char** buffer = NULL;
					buffer = (char**)malloc(32*sizeof(char*));
					char** argv2 = NULL;
					argv2 = (char**)malloc(size*sizeof(char*));
					argv2[size]=NULL;
					for(int i=0;i<size-1;i++) 
						argv2[i]=data[i+2];
					fd = open(data[1], O_CREAT|O_RDWR, S_IRWXU);
					read(fd,buffer[0],7);
					close(1);
					dup2(fd,1);
					close(fd);
					signal_flag = 0;
					execve(buffer[0], argv2, NULL);
					free(buffer);
					free(argv2);
					break;
				}
		}
	}
}


int main(int argc,char* argv[])
{

    if(fork())
		exit(0);
    setsid(); // отрываемся от терминала
	// Определяем сигналы
    signal(SIGINT,my_signal);
	signal(SIGUSR1,my_signal);
    printf("Daemon started\n");

    int fd;
	char buf[] = "Daemon started at ";
	time_t pretime;
	struct tm * start_time;
	time(&pretime);
	start_time = localtime(&pretime);
	fd = open("dmn.txt", O_CREAT|O_RDWR, S_IRWXU);
	lseek(fd, 0, SEEK_END);
	// Передаём время начала работы программы
	write(fd, buf, sizeof(buf)-1);
	write(fd, asctime(start_time), 25);
	close(fd);

    Daemon(argv,argc-1);
}
