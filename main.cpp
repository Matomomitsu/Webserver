#include "./parser/Parser.hpp"
#include "./src/Server.hpp"
#include "./src/Webserver.hpp"
#include <signal.h>
#include <cstdlib>

void sigIntHandlerFunc(int sig){
    (void)sig;
    std::cout << "\nFinishing the WebServer ...\n";
    //webserver.finish();
    exit(0);
}

int main (int argc, char *argv[]){

    Parser parser;
    Server web;
    
     if(argc != 2){
       std::cout << "Erro no arquivo de entrada" << std::endl;
       return(-1);
    }

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigIntHandlerFunc;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    web = parser.parserFile(argv[1]);
    run(web);

    return(0);
}