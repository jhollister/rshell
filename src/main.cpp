#include <iostream>
#include <unistd.h>
#include <string>
#include <string.h>

#define AND_CONNECTOR "&&"
#define OR_CONNECTOR  "||"
#define CONNECTOR     ";"

const char *DELIMS[4] = {AND_CONNECTOR, OR_CONNECTOR, CONNECTOR, 0};
std::string nextToken(const std::string&, int, const char**);

int main() {
    while(1) {
        std::cout << "$ ";
        
        std::string str;
        std::getline(std::cin, str);
        /* char *current_pos= new char[str.length()+1]; */
        /* char *cdelims = new char[delims.length()+1]; */
        /* strcpy(cdelims, delims.c_str()); */
        /* strcpy(current_pos, str.c_str()); */
        /* pch = strtok(cdelims, " "); */
        std::cout << str << std::endl;
        int start = 0;
        std::string command = nextToken(str, start, DELIMS);
        std::cout << command << std::endl;
        /* execCommand(str.substr(start, length)); */
    }


}

//TODO: Memory management
//returns the length of the current command given the start and the delims
std::string nextToken(const std::string &command, int start, const char *delims[]) {
    int delim_pos = 0;
    char *current_delim = new char[3]; // = delims[delim_pos];
    int current_ind = start;
    while (command[current_ind] != 0) {
        while (delims[delim_pos] != NULL) {
            strcpy(current_delim, delims[delim_pos]);
            /* std::cout << current_delim << std::endl; */
            //if (strncmp(current_delim, start + current_ind,
            //            strlen(current_delim)) == 0) {
            if (strcmp(command.substr(current_ind, strlen(current_delim)).c_str(),
                       current_delim) == 0) {
                delete[] current_delim;
                return command.substr(start, current_ind);
            }
            delim_pos++;
        }
        current_ind++;
        delim_pos = 0;
    }
    delete[] current_delim;
    return command.substr(start, current_ind); // no connectors
}
