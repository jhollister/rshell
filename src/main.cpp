#include <iostream>
#include <unistd.h>
#include <string>
#include <string.h>

const char *AND_CONNECTOR = "&&";
const char *OR_CONNECTOR =  "||";
const char *CONNECTOR = ";";
const char *COMMENT = "#";
const char *DELIMS[5] = {COMMENT, AND_CONNECTOR, OR_CONNECTOR, CONNECTOR, 0};
std::string nextToken(const std::string&, int &);
int execCommandList(const std::string &);
int execCommand(std::string &);
int strip(std::string &);


int main() {
    while(1) {
        std::cout << "$ ";
        
        std::string str;
        std::getline(std::cin, str);
        /* char *current_pos= new char[str.length()+1]; */
        /* char *cdelims = new char[delims.length()+1]; */
        /* strcpy(cdelims, delims.c_str()); */
        /* pch = strtok(cdelims, " "); */
        /* std::cout << str << std::end */
        /* int start = 0; */
        /* std::string command = nextToken(str, start); */
        /* std::cout << command << std::endl; */
//        execCommandList(str);
        execCommand(str);
    }
}


/* Executes the list of commands in command_list separated by connectors
 * TODO: Clean this up so it is not so long
 */
int execCommandList(const std::string &command_list) {
    int count = 0; // keeps track of the number of commands run
    int current_ind = 0;// keeps track of the current character
    bool execute = true;
    int lastCmdStatus = 0; // keeps track of the status of the last command run
    std::string current_command = nextToken(command_list, current_ind);
    while (execute) { //(current_command != "") {
        std::cout << current_command << std::endl;
        //if (execute) executeCommand(currentCommand);
        if (command_list.substr(current_ind, strlen(CONNECTOR)) == CONNECTOR) {
            // ';' connector was used
            current_ind += strlen(CONNECTOR);
            current_command = nextToken(command_list, current_ind);
            execute = true;
        }
        else if(command_list.substr(current_ind, strlen(AND_CONNECTOR)) == AND_CONNECTOR) {
            // "&&" connector was used
            current_ind += strlen(AND_CONNECTOR);
            if (lastCmdStatus == 0) {
                current_command = nextToken(command_list, current_ind);
                execute = true;
            }
            else {
                execute = false;
            }
        }
        else if(command_list.substr(current_ind, strlen(OR_CONNECTOR)) == OR_CONNECTOR) {
            // "||" connector was used
            current_ind += strlen(OR_CONNECTOR);
            if (lastCmdStatus == -1) {
                current_command = nextToken(command_list, current_ind);
                execute = true;
            }
            else {
                execute = false;
            }
        }
        else if((command_list.substr(current_ind, strlen(COMMENT)) == COMMENT) 
                || command_list[current_ind] == 0) {
            // '#' comment character was used or we have executed last command
            execute = false;
        }
        count++;
    }
    return 0;
}

int execCommand(std::string &command) {
    int token_count = strip(command);
    /* std::cout << token_count << " -  " << command << std::endl; */
    char *c_command = new char[command.length()+1];
    strcpy(c_command, command.c_str());
    char *tok = strtok(c_command, " ");
    
    char **args = new char*[token_count];
    for (int i = 0; i < token_count; i++) {
        args[i] = tok;
        std::cout << tok << std::endl;
        tok = strtok(NULL, " ");
    }

    delete[] c_command;
    delete[] args;
    return 0;
}

int strip(std::string &str) {
    char *temp = new char[str.length()+1];
    strcpy(temp, str.c_str());
    str="";
    char *tok = strtok(temp, " ");
    int count = 0;
    while(tok != NULL) {
        str += tok;
        str += " ";
        tok = strtok(NULL, " ");
        count++;
    }
    delete[] temp;
    return count;
}

/* Returns the current token in the command string and updates the reference
 * variable current_ind to be the start of the next command.
 */
std::string nextToken(const std::string &command, int &current_ind) {
    int delim_pos = 0;
    char *current_delim = new char[3]; // = delims[delim_pos];
    int start = current_ind;
    int length = 0;
    while (command[current_ind] != 0) {
        while (DELIMS[delim_pos] != NULL) {
            strcpy(current_delim, DELIMS[delim_pos]);
            /* std::cout << current_delim << std::endl; */
            //if (strncmp(current_delim, start + current_ind,
            //            strlen(current_delim)) == 0) {
            if (strcmp(command.substr(current_ind, strlen(current_delim)).c_str(),
                       current_delim) == 0) {
                delete[] current_delim;
                return command.substr(start, length);
            }
            delim_pos++;
        }
        length++;
        current_ind++;
        delim_pos = 0;
    }
    delete[] current_delim;
    return command.substr(start, length); // no connectors
}
