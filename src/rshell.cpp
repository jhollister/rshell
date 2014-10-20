#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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
void printPrompt();

int main() {
    int status = 0;
    while(status == 0) {
        printPrompt();
        std::string str;
        std::getline(std::cin, str);
        status = execCommandList(str);
    }
    //return 1 if there was an error
    return status == -1 ? 1 : 0; 

}


/* Executes the list of commands in command_list separated by connectors
 * Uses the DELIMS global constants to determine which connector was used.
 * TODO: Clean this up so it is not so long
 * TODO: Add a status return so main() can know if something went wrong.
 */
int execCommandList(const std::string &command_list) {
    int count = 0; // keeps track of the number of commands run
    int current_ind = 0;// keeps track of the current character
    bool execute = true;
    int cmd_status = 0; // keeps track of the status of the last command run
    std::string current_command = nextToken(command_list, current_ind);
    strip(current_command);
    while (execute) {
        cmd_status = execCommand(current_command);
        
        if (cmd_status == -1) {
            return 1;
        }
        if((command_list.substr(current_ind, strlen(COMMENT)) == COMMENT) 
                || command_list[current_ind] == 0) {
            // '#' comment character was used or we have executed last command
            execute = false;
        }
        else if (current_command == "") {
            // command was empty so there was a syntax error 
            // Most likely two connectors together such as ';;' or '&& &&'
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
 * Executes whatever is in the string 'command' with execvp
 * Can have extra whitespace in command but the command must be clear of any 
 * connectors or comment characters in order to work correctly.
 * Returns: 0 - succesfully executed command
 *          > 0 - an error occurred
 *          -1  - exit was called
 */
int execCommand(std::string &command) {
    int status = 1; //return status of function - 0 success Nonzero failure
    int token_count = strip(command);
    if (command == "") { // if the command is empty just return
        return status;
    }
    char *c_command = new char[command.length()+1];
    strcpy(c_command, command.c_str());
    char *tok = strtok(c_command, " ");
    
    char **args = new char*[token_count+1];
    for (int i = 0; i < token_count; i++) {
        args[i] = tok;
        tok = strtok(NULL, " ");
    }
    args[token_count] = 0; // null terminate the array

    if (strcmp(args[0], "exit") == 0) { // check to see if exit was entered
        std::cout << "exiting rshell...\n";
        status = -1;
    }

    else {
        // time for the fun stuff now.
        int pid = fork();
        if (pid == -1) {  // error in fork
            perror("fork: ");
            status = 1; //error
        }
        else if (pid == 0) { // in child process
            if (execvp(args[0], args) == -1) {
                perror("execvp: ");
                exit(1);
            }
            else exit(0);
        }
        else {  // in parent
            if (wait(&status) == -1) {
                perror("wait: ");
            }
            if (WIFEXITED(status)) {
                status = WEXITSTATUS(status);
            }
        }
    }

    delete[] args;
    delete[] c_command;
    return status;
}

/*
 * Strips the extra whitespace from a string and returns the number of tokens
 * in the string. 
 * Input: "   This     is    a    string  "
 * Output string: "This is a string" Output int: 4
 */
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

/*
 * Returns the current token in the command string and updates the reference
 * variable current_ind to be the index of the start of the next command.
 * Uses global constant DELIMS to determine what separates each command.
 */
std::string nextToken(const std::string &command, int &current_ind) {
    int delim_pos = 0;
    char *current_delim = new char[3];
    int start = current_ind;
    int length = 0;
    while (command[current_ind] != 0) {
        while (DELIMS[delim_pos] != NULL) {
            strcpy(current_delim, DELIMS[delim_pos]);
            if (strcmp(command.substr(current_ind, 
                                      strlen(current_delim)).c_str(),
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

/*
 * Returns the prompt as a string. 
 * Will always be in the format: username@hostname$
 * If it can't find hostname prints an error and sets it to default "hostname"
 * If the hostname is greater than 20 characters it will be truncated
 * TODO: Make it so that the command prompt can be easily customizable
 *       (like bashs's PS1=)
 * TODO: Display an error if it has trouble finding username
 */
void printPrompt() {
    std::string prompt = "";
    prompt += getlogin();
    prompt += "@";
    int host_len = 20;
    char *hostname = new char[host_len];
    if (gethostname(hostname, host_len) == -1) {
        perror("gethostname: ");
        std::cout << "Using generic default 'hostname' instead.\n";
        strcpy(hostname, "hostname");
    }
    prompt += hostname;
    prompt +=  "$ ";
    delete[] hostname;
    std::cout << prompt;
}
