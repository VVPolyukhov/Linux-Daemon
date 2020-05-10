#include <stdio.h> 
#include <string.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <resolv.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

bool flag = false;

void handler(int sig) {
   flag = true;
}

int Daemon(void) {
   signal(SIGINT, handler);
   while(1){
      if (flag){
         int fileDescriptor, ret;
         char buf[] = "Hello, I am Daemon";
         fileDescriptor = open("output.txt", O_CREAT|O_RDWR, S_IRWXU);
         lseek(fileDescriptor, 0, SEEK_END);
         ret = write(fileDescriptor, buf, sizeof(buf) - 1);
         close(fileDescriptor);
	 printf("Daemon stopped\n");  
         flag = false;
	 exit(0);
      }
   }
}

int main(int argc, char* argv[]) {
 pid_t parpid;
 if((parpid=fork())<0) { // создание дочернего процесса главного процесса (точная копия исполняемой программы)
	printf("\nCan't fork"); // если по какой-либо причине это сделать не удается выходим с ошибкой.
	exit(0);                // обращаемся к man fork
 }
 else if (parpid!=0) 	// если дочерний процесс уже существует
	exit(1);        // генерируем немедленный выход из программы (иначе будет еще одна копия программы)
 setsid(); 	// отрываемся от управляющего терминала и переходим в фоновый режим (перевод нашего дочернего процесса в новую сесию)
 printf("Daemon started. PID=%i\n", getpid());  
 Daemon();           // вызов демона!
 return 0;
}