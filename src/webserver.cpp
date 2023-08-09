# include "./Server.hpp"
# include "../parser/Parser.hpp"
# include "./Webserver.hpp"

//void createSocket(Server listServer)

void run(Server web){
    std::string port;

    web.printMap(web.serverMap);
    port = web.getItemFromMap(web, "Server 127.0.0.1432423423", "root", "server_root");
    // ajustar roooooot
    std::cout << "portaaaaaaaaaaaaa :" << port << std::endl;
}
