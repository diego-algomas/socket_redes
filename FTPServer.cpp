//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//
//                     2ª de grado de Ingeniería Informática
//
//                        Main class of the FTP server
//
//****************************************************************************

#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

 #include <unistd.h>


#include <pthread.h>

#include <list>

#include "common.h"
#include "FTPServer.h"
#include "ClientConnection.h"


int define_socket_TCP(int port) {
   // Include the code for defining the socket.
  struct sockaddr_in sin;
  int socketFd = socket(AF_INET, SOCK_STREAM, 0);

  if (socketFd < 0){
    errexit("We cannot create the socket: %s\n",strerror(errno));
  }

  /*Sets the first num bytes of the block of memory pointed
  by ptr to the specified value */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  if (bind(socketFd, (struct sockaddr*)&sin , sizeof(sin)) < 0 ){
    errexit("We couldn't bind with the port: %s\n", strerror(errno));
  }

  //listen for connections on a socket
  if (listen(socketFd, 5) < 0 )
    errexit("Fail during listening: %s\n", strerror(errno) );

  return socketFd;

}

// This function is executed when the thread is executed.
void* run_client_connection(void *c) {
    ClientConnection *connection = (ClientConnection *)c;
    connection->WaitForRequests();

    return NULL;
}



FTPServer::FTPServer(int port) {
    this->port = port;

}


// Parada del servidor.
void FTPServer::stop() {
    close(msock);
    shutdown(msock, SHUT_RDWR);

}


// Starting of the server
void FTPServer::run() {

    struct sockaddr_in fsin; // Qué es fsin?
    int ssock;
    socklen_t alen = sizeof(fsin);

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
    msock = define_socket_TCP(port);  // This function must be implemented by you.
    while (1) {
	pthread_t thread;
        ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
        if(ssock < 0)
            errexit("Fallo en el accept: %s\n", strerror(errno));

	ClientConnection *connection = new ClientConnection(ssock);

	// Here a thread is created in order to process multiple
	// requests simultaneously

  // Run client connection es una función de este mismo .cpp
  // Espera a que termine el hilo?
	pthread_create(&thread, NULL, run_client_connection, (void*)connection);

    }

}
