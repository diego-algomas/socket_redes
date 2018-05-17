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

// ¿A qué se inicializa parar?

/*  S es el descriptor del puerto aceptado*/
ClientConnection::ClientConnection(int s) {
    int sock = (int)(s);
    /* Se asigna el descriptor a una variable llamada sock*/

    char buffer[MAX_BUFF];

    /* Descriptor del socket de control*/
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


}//;


ClientConnection::~ClientConnection() {
 	fclose(fd);
	close(control_socket);

}

// Nos dan directamente la direccion no hace falta la consulta dns.
// Pero quién nos la da y de qué manera, no es más facil poner un string???

int connect_TCP( uint32_t address,  uint16_t  port) {

    std::cout << "ADDRESS RECEIVED " << address << "PORT RECEIVED " << port << "\n";

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
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);

    getsockname(socketFd,(struct sockaddr *)&sa, &sa_len);

    std::cout<<"ip es "<<sa.sin_addr.s_addr<<"puerto"<<sa.sin_port<<std::endl;

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

// Hay un socket de datos y un socket de control

void ClientConnection::WaitForRequests() {
    if (!ok) { // Si ha habido errores de inicialización
      std::cout << "Error de inicialización\n";
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
        /*PWD
                  257
                  500, 501, 502, 421, 550*/
            char pwd[MAX_BUFF];
            getcwd(pwd,sizeof(pwd));
            //fprintf(fd,"Working on: %s\n",pwd);

            fprintf(fd,"257 %s working directory.\n",pwd);

      }
      else if (COMMAND("PASS")) {
        /*230
                 202
                 530
                 500, 501, 503, 421
                 332*/
          fscanf(fd, "%s", arg);
          if (strcmp(arg, "1234") == 0  ){
            fprintf(fd, "230 User logged in\n");
          }else{
            fprintf(fd, "530 Not logged in\n");
            parar = true;
            }
      }
      else if (COMMAND("PORT")) {

              std::cout << "IM INTO PORT\n";
                /*PORT
                          200
                          500, 501, 421, 530*/

              int ip1[4];
              int port1[2];

              fscanf(fd,"%d,%d,%d,%d,%d,%d", &ip1[0],&ip1[1],&ip1[2],&ip1[3],&port1[0],&port1[1]);
              std::cout << "IPS ARE -->" << ip1[0] << '\n'<< ip1[1]<< '\n' << ip1[2]<< '\n' << ip1[3] << '\n'<< port1[0]<< port1[1] << "\n";
//              std::cout << ip6 << "\n";

              uint32_t  ip = ip1[3]<<24 | ip1[2]<<16 | ip1[1]<<8| ip1[0];
              uint16_t  port = port1[0]<<8 | port1[1];
             std::cout << "IP -> " << ip << "PORT -> " << port << "\n";

              data_socket = connect_TCP (ip, port);
              if(data_socket>=0){
                fprintf(fd, "200 Port ok\n");
              }
              else {
                fprintf(fd, "421 fail\n.");
              }
          // Aquí tenemos que conectarnos, es decir connectar el data socket.
        std::cout<<"entrado"<<std::endl;
          /* Este port no es el mismo que el del FTP aqui tiene que recibirse una info*/
    }
      else if (COMMAND("PASV")) {

           struct sockaddr_in sin;
           int socketFd = socket(AF_INET, SOCK_STREAM, 0);

           if (socketFd < 0){
             errexit("We cannot create the socket: %s\n",strerror(errno));
           }

           /*Sets the first num bytes of the block of memory pointed
           by ptr to the specified value */
           memset(&sin, 0, sizeof(sin));
           sin.sin_family = AF_INET;
           sin.sin_addr.s_addr = inet_addr("127.0.0.1");                    //Decidimos nosotros o no? seguramente no yo lo cambio despues
           sin.sin_port = 50000;


           if (bind(socketFd, (struct sockaddr*)&sin , sizeof(sin)) < 0 ){
             errexit("We couldn't bind with the port: %s\n", strerror(errno));
           }


           //listen for connections on a socket
           if (listen(socketFd, 5) < 0 )
             errexit("Fail during listening: %s\n", strerror(errno) );


           std::cout<<sin.sin_addr.s_addr<<std::endl;
           fprintf(fd, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n",(unsigned int)(sin.sin_addr.s_addr & 0xff),(unsigned int)((sin.sin_addr.s_addr >> 8) & 0xff),
                                                                          (unsigned int)((sin.sin_addr.s_addr >> 16) & 0xff),
                                                                          (unsigned int)((sin.sin_addr.s_addr >> 24) & 0xff),
                                                                          (unsigned int)(sin.sin_port & 0xff),
                                                                          (unsigned int)(sin.sin_port >> 8));
           fflush(fd);
//            printf("227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n",(unsigned int)(sin.sin_addr.s_addr & 0xff),(unsigned int)((sin.sin_addr.s_addr >> 8) & 0xff),
//                   (unsigned int)((sin.sin_addr.s_addr >> 16) & 0xff),
//                   (unsigned int)((sin.sin_addr.s_addr >> 24) & 0xff),
//                   (unsigned int)(sin.sin_port & 0xff),
//                   (unsigned int)(sin.sin_port >> 8));


            struct sockaddr_in fsin;
            socklen_t alen = sizeof(fsin);
            int ssock;
            std::cout<<"Salio"<<std::endl;
            ssock =accept(socketFd, (struct sockaddr *)&fsin, &alen);
            std::cout<<ssock<<std::endl;
           if(ssock<0)
                errexit("Fallo en el accept: %s\n", strerror(errno));



            std::cout<<"llego"<<std::endl;
           data_socket=socketFd;

      }
      else if (COMMAND("CWD")) {
          fscanf(fd,"%s",arg);
          fprintf(fd,"250 directory changed to %s\n",arg);
      }
      else if (COMMAND("STOR") ) {

//          125, 150
//             (110)
//             226, 250
//             425, 426, 451, 551, 552
//          532, 450, 452, 553
//          500, 501, 421, 530
            std::cout<<data_socket<<std::endl;
            fscanf(fd,"%s",arg);
            FILE* fichero;
            fichero=fopen(arg,"wb");
            if(fichero==NULL)
                fprintf(fd,"450 Requested file action not taken.\n File unavailable\n");
            else
                fprintf(fd,"150 File status okay; about to open data connection.\n");
            fflush(fd);
            char received[MAX_BUFF];
            int done=MAX_BUFF;
            while(done==MAX_BUFF){
                std::cout<<"entra"<<std::endl;
                done=recv(data_socket,received,MAX_BUFF,0);
                std::cout<<"sale"<<std::endl;
                fwrite(received,sizeof(char),done,fichero);
            }
            std::cout<<"Sale"<<std::endl;
           // fprintf(fd,"250 Requested file action okay, completed");
            fclose(fichero);
            close(data_socket);
            std::cout<<"no se queda colgado"<<std::endl;
            fprintf(fd,"226 Closing data connection.\n");
            fflush(fd);

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

                std::cout << "ERROR!!!!!!!!!!!\n";
                fprintf(fd, "450 Requested file action not taken. File unavailable (e.g., file busy, non-existent)\n");

              }

              /* Tenemos que utilizar fread para leer el fichero en un buffer y poder ir mandandolo por medio de send()
              progresivamente*/

              /* size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
              --> Reads an array of count elements,  each one with a size of size bytes,
              from the stream and stores them in the block of memory specified by ptr.*/

              fprintf(fd, "150 File status okay; about to open data connection\n");

              // Obtenemos la longitud del fichero.

              std::cout << "1\n";
              fseek(fichero, 0, SEEK_END);
              std::cout << "2\n";
              long ficheroSize = ftell(fichero);
              std::cout << "3\n";
              rewind(fichero);

              // Ahora reservamos memoria para ese buffer
              std::cout << "2\n";
              char* buffer;
              buffer = (char*) malloc(sizeof(char)*ficheroSize);
              if (buffer == NULL){
                std::cout << "Memory error\n";
              }

              std::cout << "3\n";
              // Guardamos el fichero en el buffer
              size_t result = fread(buffer, 1, ficheroSize, fichero);
              if (result != ficheroSize)
              std::cout << "Error de lectura\n";


              //TODO send devuelve -1 asi que hay un error.

              // Enviamos el fichero
              char* ptr = buffer;
              while (result > 0){
                // "On success, these calls return the number of characters sent." --> send()
                // ¿Habrá un problema con el data socket?
                int charactersSent = send(data_socket, ptr, result , 0);
                std::cout << charactersSent << "\n";
                break;
                ptr += charactersSent;
                result  -= charactersSent;
                std::cout << charactersSent << "\n";
              }

              fprintf(fd, "226 Closing data connection.\n");

              /* Abrir el fichero con fopen y mandarlo con send() a la dirección IP que nos tienen que dar supongo.
                ¿Y luego usar connect_tcp?*/
      }
      else if (COMMAND("QUIT")) {
          fprintf(fd,"221 Service closing control connection.\n");
          fclose(fd);
      }
      else if (COMMAND("LIST")) {
        std::cout<<"ejecuta"<<std::endl;
      }
      else  {

      fprintf(fd, "502 Command not implemented.\n");
      fflush(fd);
	    printf("Comando : %s %s\n", command, arg);
	    printf("Error interno del servidor\n");

      }

    }

    fclose(fd);


    return;

};
