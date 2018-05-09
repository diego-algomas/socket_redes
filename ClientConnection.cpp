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

int connect_TCP( uint32_t address,  uint16_t  port) {

    struct sockaddr_in sin;
    struct hostent* hent; // ¿Qué es hostent?




    /*       struct hostent {

                char  *h_name;            /* official name of host */
    //          char **h_aliases;         /* alias list */
    //          int    h_addrtype;        /* host address type */
    //          int    h_length;          /* length of address */
    //          char **h_addr_list;       /* list of addresses */
    //          h_addr The first address in h_addr_list for backward compatibility.
  //}

          /*  struct sockaddr_in {

              short            sin_family;   // e.g. AF_INET
              unsigned short   sin_port;     // e.g. htons(3490)
              struct in_addr   sin_addr;     // see struct in_addr, below
              char             sin_zero[8];  // zero this if you want to

          };*/




      /*    struct in_addr {
          in_addr_t s_addr;               /* the IP address in network byte order    */
        //};



    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if (hent = gethostbyname(host)){
      memcpy(&sin.sin_addr, hent -> h_addr, hent -> h_length);
    }else if ((sin.sin_addr.s_addr = inet_addr((char*)host)) == INADDR_NONE){
      errexit("I cannot resolve the name \"%s\"\n", host);
    }

    socketFd = socket(AF_INET, SOCK_STEAM, 0);
    if (socketFd < 0){
      errexit("Socket cannot be created: %s\n", strerror(errno));
    }

    if (connect(socketFd, (struct sockaddr*)&sin, sizeof(sin)) < 0){
      errexit("We cannot connect with %s: %s\n", host, strerrno(errno));
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
	     return;
    }

    fprintf(fd, "220 Service ready\n");

    while(!parar) {



      fscanf(fd, "%s", command);
      if (COMMAND("USER")) {
	    fscanf(fd, "%s", arg);
	    fprintf(fd, "331 User name ok, need password\n");
      }
      else if (COMMAND("PWD")) {

      }
      else if (COMMAND("PASS")) {

      }
      else if (COMMAND("PORT")) {

      }
      else if (COMMAND("PASV")) {

      }
      else if (COMMAND("CWD")) {

      }
      else if (COMMAND("STOR") ) {

      }
      else if (COMMAND("SYST")) {

      }
      else if (COMMAND("TYPE")) {

      }
      else if (COMMAND("RETR")) {

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
