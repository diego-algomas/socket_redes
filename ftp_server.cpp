#include <iostream>
#include <signal.h>

#include "FTPServer.h"

FTPServer *server;

extern "C" void sighandler(int signal, siginfo_t *info, void *ptr) {
  std::cout << "Sigaction has been fired" << std::endl;
  server->stop();
  exit(-1);
}


void exit_handler() {
    server->stop();
}


int main(int argc, char **argv) {

    struct sigaction action;
    action.sa_sigaction = sighandler;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &action , NULL);
    server = new FTPServer(2121);
    atexit(exit_handler); /* It only specifies how to stop the server*/
    server->run(); /* Then the servers inits*/

}
