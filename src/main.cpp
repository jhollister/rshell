#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

const char *AND_CONNECTOR = "&&";
const char *OR_CONNECTOR =  "||";
const char *CONNECTOR = ";";
const char *COMMENT = "#";
const char *DELIMS[5] = {COMMENT, AND_CONNECTOR, OR_CONNECTOR, CONNECTOR, 0};

std::string nextToken(const std::string&, int &);
int execCommandList(const std::string &);
int execCommand(std::string &);
int strip(std::string &);
std::string getPrompt();

int main() {
    std::string prompt = getPrompt();
    while(1) {
        std::cout << prompt;
        std::string str;
        std::getline(std::cin, str);
        execCommandList(str);
    }
}


/* Executes the list of commands in command_list separated by connectors
 * TODO: Clean this up so it is not so long
 */
int execCommandList(const std::string &command_list) {
    int count = 0; // keeps track of the number of commands run
    int current_ind = 0;// keeps track of the current character
    bool execute = true;
    int cmd_status = 0; // keeps track of the status of the last command run
    std::string current_command = nextToken(command_list, current_ind);
    strip(current_command);
    /* if (current_command == "" && (command_list[current_ind+1] == '#' || */
    /*                               command_list[current_ind+1] == 0)) { */
    /*     // nothing was inputted so just move on with no error message */
    /*     execute = false; */
    /* } */
    while (execute) { //(current_command != "") {
        /* std::cout << current_command << std::endl; */
        cmd_status = execCommand(current_command);
        
        if((command_list.substr(current_ind, strlen(COMMENT)) == COMMENT) 
                || command_list[current_ind] == 0) {
            // '#' comment character was used or we have executed last command
            execute = false;
        }
        else if (current_command == "") {
            std::cout << "rshell: syntax error near unexpected token: " <<
                         command_list[current_ind] << std::endl;
            execute = false;
        }
        else if (command_list.substr(current_ind, strlen(CONNECTOR)) ==
                CONNECTOR) {
            // ';' connector was used
            current_ind += strlen(CONNECTOR);
            current_command = nextToken(command_list, current_ind);
            execute = true;
        }
        else if(command_list.substr(current_ind, strlen(AND_CONNECTOR)) ==
                AND_CONNECTOR) {
            // "&&" connector was used
            current_ind += strlen(AND_CONNECTOR);
            if (cmd_status == 0) {
                current_command = nextToken(command_list, current_ind);
                execute = true;
            }
            else {
                execute = false;
            }
        }
        else if(command_list.substr(current_ind, strlen(OR_CONNECTOR)) ==
                OR_CONNECTOR) {
            // "||" connector was used
            current_ind += strlen(OR_CONNECTOR);
            if (cmd_status != 0) {
                current_command = nextToken(command_list, current_ind);
                execute = true;
            }
            else {
                execute = false;
            }
        }
        else {
            std::cout << "rshell: Something went wrong at: " <<
                         command_list[current_ind] << std::endl;
            execute = false;
        }
        count++;
        strip(current_command);
    }
    return 0;
}

/*
 * TODO: Don't call strip since it has already been stripped (need token count though)
 */
int execCommand(std::string &command) {
    int status = 0; //return status of this function - 1 failed - 0 success
    int token_count = strip(command);
    /* std::cout << token_count << " -  " << command << std::endl; */
    char *c_command = new char[command.length()+1];
    strcpy(c_command, command.c_str());
    char *tok = strtok(c_command, " ");
    
    char **args = new char*[token_count+1];
    for (int i = 0; i < token_count; i++) {
        args[i] = tok;
        /* std::cout << tok << std::endl; */
        tok = strtok(NULL, " ");
    }
    args[token_count] = 0; // null terminate the array

    // time for the fun stuff now.
    int pid = fork();
    if (pid == -1) {  // error in fork
        perror("fork: ");
        exit(1);
    }
    else if (pid == 0) { // in child process
        /* std::cout << "In child process." << std::endl; */
        /* std::cout << "Executing " << args[0] << std::endl; */
        if (execvp(args[0], args)) {
            perror("execvp: ");
            exit(1);
        }
            
        exit(0);

    }
    else {  // in parent
        if (wait(&status) == -1) {
            perror("wait: ");
            exit(1);
        }
        status = WEXITSTATUS(status);
        /* std::cout << status << " Child process is kill\n"; */
    }

    delete[] c_command;
    delete[] args;
    return status;
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

/* Returns the prompt as a string. 
 * Will always be in the format: username@hostname$
 * TODO: Make it so that the command prompt can be easily customizable
 *       (like bashs's PS1=)
 */
std::string getPrompt() {
    std::string prompt = "";
    prompt += getlogin();
    prompt += "@";
    int host_len = 20;
    char *hostname = new char[host_len];
    if (gethostname(hostname, host_len) == -1) {
        perror("gethostname: ");
        exit(1);
    }
    prompt += hostname;
    prompt +=  "$ ";
    delete[] hostname;
    return prompt;
}
