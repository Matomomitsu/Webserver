#ifndef PARSER_HPP
# define PARSER_HPP


# include <iostream>
# include <vector>
# include <fcntl.h>
# include <fstream>
# include <sstream>
# include <cstdio>
# include <vector>
# include <map>
#include <string>
#include "../src/Server.hpp"

class Parser{
    public:
        std::string inputFileRead;
        Server parserFile(std::string inputFilePath);
    private:
        
};

#endif
