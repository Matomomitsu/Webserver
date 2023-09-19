# include "./Server.hpp"
# include "../parser/Parser.hpp"
# include "./Webserver.hpp"

void run(Server &web){
    std::string port;

    web.containsCgi = false;
    web.autoindex = false;
    //web.printMap(web.serverMap);
}
