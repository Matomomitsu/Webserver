#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>

void listDirectoriesAsButtons(const std::string& directoryPath, std::ostream& output) {
    output << "<html><head><title>Lista de Diretórios</title></head><body>";

    DIR* dir;
    struct dirent* entry;

    if ((dir = opendir(directoryPath.c_str())) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
                    output << "<form action=\"" << entry->d_name << "\">"
                           << "<input type=\"submit\" value=\"" << entry->d_name << "\"/>"
                           << "</form>";
                }
            }
        }
        closedir(dir);
    }

    output << "</body></html>";
}

int main() {
    std::ofstream htmlFile("/home/user42/Desktop/Webserver/git_atual/testes/teste.html");
    if (htmlFile.is_open()) {
        listDirectoriesAsButtons(".", htmlFile);  // Lista os diretórios no diretório atual
        htmlFile.close();
    } else {
        std::cerr << "Erro ao abrir o arquivo HTML." << std::endl;
        return 1;
    }

    return 0;
}
