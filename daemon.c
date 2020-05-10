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
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#define N 64
#define MAXCHAR 100

int signal_flag;

void my_signal (int sig) {
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

// Создаю семафор
sem_t sem;

pid_t processes[N];

void Daemon(char* data[], int size) {

	int fd;
	int fd2;
	int fd3;

	while (1) {
		pause();
		switch (signal_flag) {
			case 1:
				exit(0);
			case 2: {
				char buffer[2048];
				char *buf;
				char *pch[N]; 
				char *args[N][MAXCHAR]; 
				char *comms[N];
				char *buffer1[MAXCHAR * N];
				sem_init(&sem, 0, 1);
				
				fd = open("log.txt", O_CREAT | O_RDWR, S_IRWXU);
				fd2 = open(data[1], O_CREAT | O_RDWR, S_IRWXU);
				fd3 = open("output.txt", O_CREAT | O_RDWR, S_IRWXU);
				read(fd2, buffer, 2048);
				close(fd2);
				
				// Делю текст на строки
				
				int ind = 0;
				// Количество команд на выполнение
				int proc_number = 0;
				// Символ для последней команды
				buf = strtok(buffer, "$");
				pch[ind] = strtok(buf, "\n");
				while (pch[ind] != NULL) {
					ind = ind + 1;
					pch[ind] = strtok(NULL, "\n");
				}
				
				for (int i = 0; i < N; i++) 
					if (pch[i] != NULL) 
						proc_number++;
				
				int ind2;
				for (int i = 0; i < proc_number; i++) {
					buffer1[ind2] = strtok(pch[i], " ");
					comms[i] = buffer1[ind2];
					int counter = 0;
					while (buffer1[ind2] != NULL) {
						counter++;
						ind2 = ind2 + 1;
						buffer1[ind2] = strtok(NULL, " ");
					}
					for (int k = 0; k < counter; k++) 
						args[i][k] = buffer1[ind2 - counter + k + 1];
				}
				
				char buf1[] = "\nCompleted successfully";
				char buf2[] = "\nThe program is not executed";

				pid_t pid;
				for (int i = 0; i < proc_number; ++i) {
					if (processes[i] != 0) {
						int status2;
						wait(&status2);
					}
					else {
						pid = fork();
						if (pid == 0) {
							close(1);
							dup2(fd3, 1);
							if (execv(comms[i], args[i]) < 0)
								exit(1);
						}
						else {
							int status;
							// Семафор для того, чтобы
							// процессы не писали одновременно в файл
							wait(&status);
							sem_wait(&sem);
							if (WEXITSTATUS(status) != 1)
								write(fd, buf1, 25);
							else
								write(fd, buf2, 30);
							sem_post(&sem);
						}
						// Выходим из созданного fork
						exit(0);
					}
				}
				sem_destroy(&sem);
				signal_flag = 0;
				close(fd);
				close(fd3);
				break;
			}
		}
	}
}


int main(int argc, char* argv[]) {
	if(fork())
		exit(0);
    setsid(); // отрываемся от терминала
	// Определяем сигналы
    signal(SIGINT,my_signal);
	signal(SIGUSR1,my_signal);
    
    int fd;
	char buf[] = "Daemon started at ";
	time_t pretime;
	struct tm * start_time;
	time(&pretime);
	start_time = localtime(&pretime);
	fd = open("log.txt", O_CREAT|O_RDWR, S_IRWXU);
	lseek(fd, 0, SEEK_END);
	// Передаём время начала работы программы
	write(fd, buf, sizeof(buf)-1);
	write(fd, asctime(start_time), 25);
	close(fd);
    
    Daemon(argv, argc-1);
}
