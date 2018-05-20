//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//
//                     2ª de grado de Ingeniería Informática
//
//              This class processes an FTP transactions.
//
//****************************************************************************



#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <iostream>
#include <dirent.h>

#include "common.h"

#include "ClientConnection.h"


ClientConnection::ClientConnection(int s) {
    int sock = (int)(s);

    char buffer[MAX_BUFF];
    pasiveMode=false;


    control_socket = s;
    /*The fdopen() function associates a stream with the existing
     file descriptor, fd. The mode of the stream (one of the values
     "r", "r+", "w", "w+", "a", "a+") must be compatible with the
     mode of the file descriptor.*/
    fd = fdopen(s, "a+");

    if (fd == NULL){

    std::cout << "Connection closed" << std::endl;
    fclose(fd);
    close(control_socket);
    ok = false;
    //return ;

    }else{

    ok = true;
    data_socket = -1;

  }


}


ClientConnection::~ClientConnection() {
 	fclose(fd);
	close(control_socket);

}

int connect_TCP( uint32_t address,  uint16_t  port) {

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = (unsigned long)address;

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0){
      errexit("Socket cannot be created: %s\n", strerror(errno));
    }

    if (connect(socketFd, (struct sockaddr*)&sin, sizeof(sin)) < 0){
      errexit("We cannot connect %s\n");
    }

    return socketFd;

}


void ClientConnection::stop() {
    close(data_socket);
    close(control_socket);
    parar = true;
}






#define COMMAND(cmd) strcmp(command, cmd) == 0

// This method processes the requests.
// Here you should implement the actions related to the FTP commands.
// See the example for the USER command.
// If you think that you have to add other commands feel free to do so. You
// are allowed to add auxiliary methods if necessary.

void ClientConnection::WaitForRequests() {
    if (!ok) { // If there is any problem  on the inicialization
      std::cout << "Initialization error\n";
	     return;
    }

    fprintf(fd, "220 Service ready\n");

    while(!parar) {

      fscanf(fd, "%s", command);
      if (COMMAND("USER")) {
            fscanf(fd, "%s", arg);
          if (strcmp(arg, "Adri") == 0  ){
            fprintf(fd, "331 User name ok, need password\n");
          }else{
          fprintf(fd, "530 Not logged in\n");
          parar = true;
            }
          }
      else if (COMMAND("PWD")) {

            char pwd[MAX_BUFF];
            getcwd(pwd,sizeof(pwd));
            fprintf(fd,"257 %s working directory.\n",pwd);

      }
      else if (COMMAND("PASS")) {

          fscanf(fd, "%s", arg);
          if (strcmp(arg, "1234") == 0  ){
            fprintf(fd, "230 User logged in\n");
          }else{
            fprintf(fd, "530 Not logged in\n");
            parar = true;
            }
      }
      else if (COMMAND("PORT")) {

              int ip1[4];
              int port1[2];

              fscanf(fd,"%d,%d,%d,%d,%d,%d", &ip1[0],&ip1[1],&ip1[2],&ip1[3],&port1[0],&port1[1]);

              uint32_t  ip = ip1[3]<<24 | ip1[2]<<16 | ip1[1]<<8| ip1[0];
              uint16_t  port = port1[0]<<8 | port1[1];

              data_socket = connect_TCP (ip, port);
              if(data_socket>=0){
                fprintf(fd, "200 Port ok\n");
              }
              else {
                fprintf(fd, "421 fail\n.");
              }
              fflush(fd);
    }
      else if (COMMAND("PASV")) {

           pasiveMode = true;

           struct sockaddr_in sin;
           int socketFd = socket(AF_INET, SOCK_STREAM, 0);

           if (socketFd < 0){
             errexit("We cannot create the socket: %s\n",strerror(errno));
           }

           /*Sets the first num bytes of the block of memory pointed
           by ptr to the specified value */
           memset(&sin, 0, sizeof(sin));
           sin.sin_family = AF_INET;
           sin.sin_addr.s_addr = inet_addr("127.0.0.1");
           sin.sin_port = 0;

           socklen_t sinsiz =sizeof(sin);

           if (bind(socketFd, (struct sockaddr*)&sin , sizeof(sin)) < 0 ){
             errexit("We couldn't bind with the port: %s\n", strerror(errno));
           }


           //listen for connections on a socket
           if (listen(socketFd, 5) < 0 )
             errexit("Fail during listening: %s\n", strerror(errno) );

           data_socket=socketFd;
           getsockname(data_socket,(struct sockaddr*)&sin,&sinsiz);
           fprintf(fd, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n",(unsigned int)(sin.sin_addr.s_addr & 0xff),(unsigned int)((sin.sin_addr.s_addr >> 8) & 0xff),
                                                                          (unsigned int)((sin.sin_addr.s_addr >> 16) & 0xff),
                                                                          (unsigned int)((sin.sin_addr.s_addr >> 24) & 0xff),
                                                                          (unsigned int)(sin.sin_port & 0xff),
                                                                          (unsigned int)(sin.sin_port >> 8));
           fflush(fd);

      }
      else if (COMMAND("CWD")) {
          fscanf(fd,"%s",arg);
          fprintf(fd,"250 directory changed to %s\n",arg);
      }
      else if (COMMAND("STOR") ) {

            fscanf(fd,"%s",arg);
            FILE* fichero;
            fichero=fopen(arg,"wb");
            if(fichero==NULL)
                fprintf(fd,"450 Requested file action not taken.\n File unavailable\n");
            else
                fprintf(fd,"150 File status okay; about to open data connection.\n");
            fflush(fd);
            if (pasiveMode){

              struct sockaddr_in fsin;
              socklen_t fsin_len = sizeof(fsin);

              data_socket = accept(data_socket, (struct sockaddr*)&fsin, &fsin_len);

              if(data_socket<0)
                   errexit("Fallo en el accept: %s\n", strerror(errno));
            }

            char received[MAX_BUFF];
            int done=MAX_BUFF;
            while(done==MAX_BUFF){
                done=recv(data_socket,received,MAX_BUFF,0);
                fwrite(received,sizeof(char),done,fichero);
            }
            fclose(fichero);

            fprintf(fd,"226 Closing data connection.\n");
            fflush(fd);
            close(data_socket);

      }
      else if (COMMAND("SYST")) {
      fprintf(fd, "215 UNIX Type: L8.\n");
      }
      else if (COMMAND("TYPE")) {
      fprintf(fd, "200 Port ok\n");
      }
      else if (COMMAND("RETR")) {

              fscanf(fd, "%s", arg);

              FILE* fichero;
              fichero = fopen(arg,"rb");

              if (fichero == NULL){

                fprintf(fd, "450 Requested file action not taken. File unavailable (e.g., file busy, non-existent)\n");
                close(data_socket);

              }else{

              /* size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
              --> Reads an array of count elements,  each one with a size of size bytes,
              from the stream and stores them in the block of memory specified by ptr.*/

              fprintf(fd, "150 File status okay; about to open data connection\n");

              if (pasiveMode){

                struct sockaddr_in fsin;
                socklen_t fsin_len = sizeof(fsin);

                data_socket = accept(data_socket, (struct sockaddr*)&fsin, &fsin_len);

                if(data_socket<0)
                     errexit("Fallo en el accept: %s\n", strerror(errno));
              }

              // We obtain the file lenght

              fseek(fichero, 0, SEEK_END);
              long ficheroSize = ftell(fichero);
              rewind(fichero);

              // We save memory for the buffer
              char* buffer;
              buffer = (char*) malloc(sizeof(char)*ficheroSize);
              if (buffer == NULL){
                std::cout << "Memory error\n";
              }


              // We store the file in the buffer
              size_t result = fread(buffer, 1, ficheroSize, fichero);
              if (result != ficheroSize)
              std::cout << "Lecture error\n";




              // We send the file
              char* ptr = buffer;
              while (ficheroSize > 0){
                // "On success, these calls return the number of characters sent." --> send()
                int charactersSent = send(data_socket, ptr, ficheroSize , 0);
                ptr += charactersSent;
                ficheroSize  -= charactersSent;

              }
              fclose(fichero);
              close(data_socket);
              fprintf(fd, "226 Closing data connection.\n");
              fflush(fd);

          }

      }
      else if (COMMAND("QUIT")) {
          fprintf(fd,"221 Service closing control connection.\n");
          fflush(fd);
          stop();
      }
      else if (COMMAND("LIST")) {


                fprintf(fd, "125 Data connection already open; transfer Starting\n");
                fflush(fd);


                char buffer[MAX_BUFF];
                std::string ls = "ls -l";

                FILE* fichero = popen(ls.c_str(), "r");

                if (fichero == NULL){
                  fprintf(fd, "450 Requested file action not taken. File unavailable\n");
                  fflush(fd);
                  close(data_socket);
                }

                else{

                    if (pasiveMode){

                      struct sockaddr_in fsin;
                      socklen_t fsin_len = sizeof(fsin);

                      data_socket = accept(data_socket, (struct sockaddr*)&fsin, &fsin_len);

                      if(data_socket<0)
                           errexit("Accept failed: %s\n", strerror(errno));

                    }
                std::string contenido;

                while (!feof(fichero)){
                    if (fgets(buffer, MAX_BUFF, fichero) != NULL)
                       contenido = buffer;

                    send(data_socket, contenido.c_str(), contenido.size(), 0);
                  }

                  fprintf(fd, "250 Closing data connection. Requested file action successful.\n");
                  fflush(fd);
                  pclose(fichero);
                  close(data_socket);

                }


      }
      else  {

      fprintf(fd, "502 Command not implemented.\n");
      fflush(fd);
	    printf("Comando : %s %s\n", command, arg);
	    printf("Internal server error\n");

      }

    }

    fclose(fd);


    return;

}
