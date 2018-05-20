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
#include <iostream>

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


void FTPServer::run() {

    struct sockaddr_in fsin;
    int ssock;
    socklen_t alen = sizeof(fsin);
    msock = define_socket_TCP(port);

    while (1) {
        pthread_t thread;
        ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
        if(ssock < 0)
            errexit("Accept Failure: %s\n", strerror(errno));
        ClientConnection *connection = new ClientConnection(ssock);
        pthread_create(&thread, NULL, run_client_connection, (void*)connection);
    }

}
