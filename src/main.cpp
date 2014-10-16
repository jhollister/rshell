#include <iostream>
#include <unistd.h>
#include <string>
#include <string.h>

int nextCommand(const char*, const char**);

int main() {
    while(1) {
        std::cout << "$ ";
        
        std::string str;
        std::getline(std::cin, str);
        const char *delims[4] = {"||", "&&", ";", 0};
        char *cstr = new char[str.length()+1];
        /* char *cdelims = new char[delims.length()+1]; */
        /* strcpy(cdelims, delims.c_str()); */
        strcpy(cstr, str.c_str());
        /* pch = strtok(cdelims, " "); */

        nextCommand(cstr, delims);
    }
}

//returns the length of the next command given the start and the delims
int nextCommand(const char *start, const char *delims[]) {
    int delim_pos = 0;
    char *current_delim = new char[3]; // = delims[delim_pos];
    int current_pos = 0;
    while (start[current_pos] != 0) {
        while (delims[delim_pos] != NULL) {
            strcpy(current_delim, delims[delim_pos]);
            /* std::cout << current_delim << std::endl; */
            if (strncmp(current_delim, start + current_pos, strlen(current_delim)) == 0) {
                std::cout << "Equals at " << current_pos << std::endl;
                return current_pos;
                
            }
            delim_pos++;
        }
        current_pos++;
        delim_pos = 0;
    }
    return 0; // no connectors
}
