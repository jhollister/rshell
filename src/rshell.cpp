#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

const std::string AND_CONNECTOR = "&&";
const std::string OR_CONNECTOR =  "||";
const std::string CONNECTOR = ";";
const std::string COMMENT = "#";
const std::vector<std::string> DELIMS =
                   {COMMENT, AND_CONNECTOR, OR_CONNECTOR, CONNECTOR};

struct Command {
    std::string prevConnector;
    std::string command;
    std::string nextConnector;
};

int fillCommands(const std::string &input, std::vector<Command> &commands);
int nextDelim(const std::string &input);
std::string getDelimAt(const std::string &input, int index);
int execCommandList(const std::vector<Command> &commands);
int execCommand(const  std::string command);
int strip(std::string &);
void stripLeadingSpaces(std::string &str);
std::string getPrompt();

int main()
{
    int status = 0;
    std::string prompt = getPrompt();
    while(status == 0) {
        std::cout << prompt;
        std::string input;
        std::vector<Command> commands;
        std::getline(std::cin, input);
        int length = fillCommands(input, commands);
        if (length == -1) {
            std::cout << "rshell: Syntax error near unexpected token: "
                      << commands.back().nextConnector << std::endl;
        }
        else if (!commands.empty()){
            status = execCommandList(commands);
        }
    }

    //return 1 if there was an error
    if (status == 1) {
        std::cerr << "rshell: ERROR: Something went wrong\n";
        return 1;
    }
    return 0;
}


/*
 * Executes the list of commands in commands
 * Returns:  1: An error occured and we couldn't finish the commands
 *           0: Successfully executed all commands
 *          -1: Exit was called
 */
int execCommandList(const std::vector<Command> &commands)
{
    bool execute = true;
    int cmd_status = 0; // keeps track of the status of the last command run
    int status = 0;
    unsigned int i = 0;
    while (execute && (i < commands.size())) {
        cmd_status = execCommand(commands[i].command);
        if (cmd_status == -1) {
            // exit was called in execCommand
            status = -1;
            execute = false;
        }
        else if(commands[i].nextConnector == COMMENT ||
                commands[i].nextConnector == "") {
            execute = false;
        }
        else if(commands[i].nextConnector == CONNECTOR) {
            execute = true;
        }
        else if(commands[i].nextConnector == AND_CONNECTOR) {
            execute = !(cmd_status);
        }
        else if(commands[i].nextConnector == OR_CONNECTOR) {
            execute = cmd_status;
        }
        else {
            //don't know why this would happen
            status = 1;
            execute = false;
        }
        i++;
    }
    return status;
}

/*
 * Executes whatever is in the string 'command' with execvp
 * Can have extra whitespace in command but the command must be clear of any
 * connectors or comment characters in order to work correctly.
 * Returns: 0 - succesfully executed command
 *          > 0 - an error occurred
 *          -1  - exit was called
 */
int execCommand(std::string command)
{
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
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { // in child process
            if (execvp(args[0], args) == -1) {
                perror("execvp: ");
                _exit(EXIT_FAILURE);
            }
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

int execCommand(const Command &command)
{
    return 0;
}

/*
 * Strips the extra whitespace from a string and returns the number of tokens
 * in the string.
 * Input: "   This     is    a    string  "
 * Output string: "This is a string" Output int: 4
 */
int strip(std::string &str)
{
    char *temp = new char[str.length()+1];
    strcpy(temp, str.c_str());
    str="";
    char *tok = strtok(temp, " \n\t\v\f\r");
    int count = 0;
    while(tok != NULL) {
        str += tok;
        tok = strtok(NULL, " \n\t\v\f\r");
        if (tok != NULL) str += " ";
        count++;
    }
    delete[] temp;
    return count;
}

/*
 * Fills the list of Commands given the input
 * Separates each command based on whether there is a given delimiter
 * in input that matches a delimiter in DELIMS
 * Returns the amount of commands
 * Returns -1 if there is an error in parsing
 */
int fillCommands(const std::string &input, std::vector<Command> &commands)
{
    int start = 0;
    int size = 0;
    std::string command_str = input;
    std::string prev_connector = "";
    std::string next_connector = "";
    strip(command_str);
    Command current_command;
    while(command_str.substr(start) != "") {
        int next = nextDelim(command_str.substr(start));
        std::string current_delim = getDelimAt(command_str.substr(start), next);
        if (next_connector == COMMENT || (current_delim == COMMENT && next == 0)) {
            // comments are treated differently than all other connectors
            // since they do not cause syntax errors
            current_command.prevConnector = COMMENT;
            current_command.nextConnector = "";
            current_command.command = "";
            commands.push_back(current_command);
            return size;
        }
        current_command.prevConnector = next_connector;
        current_command.nextConnector = current_delim;
        next_connector = current_delim;
        current_command.command = command_str.substr(start, next);
        commands.push_back(current_command);
        size++;
        start += next + current_delim.length();
        if (next == 0) {
            return -1;
        }
    }
    if (!commands.empty() && commands.back().nextConnector != "") {
        // If the last connector is not an empty string something went wrong
        return -1;
    }

    return size;
}

/*
 * Returns the index of the next delim in input
 * If there is no delim in input returns the length of input
 */
int nextDelim(const std::string &input)
{
    int ind = 0;
    while(input[ind] != 0) {
        for (unsigned int i = 0; i < DELIMS.size(); i++) {
            if (input.substr(ind, DELIMS[i].length()) == DELIMS[i]) {
                return ind;
            }
        }
        ind++;
    }
    return ind;
}

/*
 * Returns the value of the delimiter located at the given index.
 * Returns an empty string if there is no delimiter there.
 */
std::string getDelimAt(const std::string &input, int index)
{
    std::string delim = "";
    for (unsigned int i = 0; i < DELIMS.size(); i++) {
        if (input.substr(index, DELIMS[i].length()) == DELIMS[i]) {
            delim = DELIMS[i];
            return delim;
        }
    }
    return delim;
}


/*
 * Returns the prompt as a string.
 * If there is an error getting username no username will be displayed
 * If it can't find hostname prints an error and does not display a hostname
 * If the hostname is greater than 20 characters it will be truncated
 * If it has trouble with username it will default to an empty username
 */
std::string getPrompt()
{
    std::string prompt = "";
    char *login = getlogin();
    if (login == NULL) {
        perror("getlogin: ");
    }
    else {
        prompt += login;
    }

    int host_len = 20;
    char *hostname = new char[host_len];
    if (gethostname(hostname, host_len) == -1) {
        perror("gethostname: ");
    }
    else {

        if (login != NULL) prompt += "@";
        prompt += hostname;
    }
    prompt += "$ ";
    delete[] hostname;
    return prompt;
}
