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


};


ClientConnection::~ClientConnection() {
 	fclose(fd);
	close(control_socket);

}

// Nos dan directamente la direccion no hace falta la consulta dns.
// Pero quién nos la da y de qué manera, no es más facil poner un string???

int connect_TCP( uint32_t address,  int  port) {

    std::cout << "ADDRESS RECEIVED" << int(address) << "PORT RECEIVED" << int(port) << "\n";

    struct sockaddr_in sin;
    struct hostent* hent;

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

      int ip1, ip2, ip3, ip4, ip5, ip6;

      fscanf(fd,"%i,%i,%i,%i,%i,%i", &ip1,&ip2,&ip3,&ip4,&ip5,&ip6);
      std::cout << "IPS ARE -->" << ip1 << ip2 << ip3 << ip4 << ip5<< ip6 << "\n";
      std::cout << ip6 << "\n";

      uint32_t ip = ip4 << 24 | ip3 << 16 | ip2 << 8 | ip1;
      uint32_t port = ip6 << 8 | ip5;
      std::cout << "IP -> " << ip << "PORT -> " << port << "\n";

      data_socket = connect_TCP (uint32_t(ip), port);

      fprintf(fd, "200 Port ok\n");
      // Aquí tenemos que conectarnos, es decir connectar el data socket.

      /* Este port no es el mismo que el del FTP aqui tiene que recibirse una info*/
    }
      else if (COMMAND("PASV")) {

      }
      else if (COMMAND("CWD")) {

      }
      else if (COMMAND("STOR") ) {

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
      //  std::cout << arg << "\n";
        std::cout << "ERROR!!!!!!!!!!!\n";
      }
      fprintf(fd, "450 Requested file action not taken. File unavailable (e.g., file busy, non-existent)\n");

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

      }
      else if (COMMAND("LIST")) {

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
