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
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <uuid/uuid.h>

const std::string AND_CONNECTOR = "&&";
const std::string OR_CONNECTOR  = "||";
const std::string CONNECTOR     = ";";
const std::string COMMENT       = "#";
const std::string REDIR_OUT     = ">";
const std::string REDIR_OUT_APP = ">>";
const std::string REDIR_IN      = "<";
const std::string PIPE          = "|";
const std::vector<std::string> DELIMS =
                   {COMMENT, AND_CONNECTOR, OR_CONNECTOR, CONNECTOR,
                    REDIR_OUT_APP, REDIR_OUT, REDIR_IN, PIPE};

struct Command {
    std::string prevConnector;
    std::string command;
    std::string nextConnector;
};

int fillCommands(const std::string &input, std::vector<Command> &commands);
int nextDelim(const std::string &input);
std::string getDelimAt(const std::string &input, int index);
int execCommandList(const std::vector<Command> &commands);
int setRedir(const std::string &connector, const std::string &next);
bool isRedir(const std::string &connector);
bool firstWord(const std::string &command, const std::string &word);
void execCommand(std::string command);
int strip(std::string &);
int countPipes(const std::vector<Command> &commands, int index);
void stripLeadingSpaces(std::string &str);
std::string getPrompt();
bool checkStatus(const int status, const std::string &connector);
void childSigHandler(int signum);
int getPath(std::vector<std::string> &paths);
std::string getCurrentDir();
int cd(int argc, char *argv[]);

int main()
{
    signal(SIGINT, SIG_IGN);
    int status = 0;
    while(status == 0) {
        std::string prompt = getPrompt();
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
 * TODO: Oh god, this function is terrible.
 */
int execCommandList(const std::vector<Command> &commands)
{
    bool execute = true;
    int status = 0;
    unsigned int i = 0;
    int savestdin;
    std::string prevConnector;
    std::string nextConnector;
    if ((savestdin = dup(STDIN_FILENO)) == -1)
                    perror("dup");
    while (execute && (i < commands.size())) {

        int cmd_status = 0;
        int pipefd[2];
        if (firstWord(commands[i].command, "exit")) {
            return -1;
        }
        else if (firstWord(commands[i].command, "cd")) {
            std::string cd_cmd = commands[i].command;
            int token_count = strip(cd_cmd);
            char *c_command = new char[cd_cmd.length()+1];
            strcpy(c_command, cd_cmd.c_str());
            char *tok = strtok(c_command, " ");
            char **args = new char*[token_count+1];
            for (int i = 0; i < token_count; i++) {
                args[i] = tok;
                tok = strtok(NULL, " ");
            }
            cmd_status = cd(token_count, args);
            delete[] args;
            delete c_command;
        }
        else {
            if (pipe(pipefd) == -1) {
                perror("execCommandList: pipe:");
                return 1;
            }
            int pid = fork();
            if (pid == -1) {
                perror("execCommandList: fork:");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0) { //in child process
                int j = i;
                while (isRedir(commands[j].nextConnector)) {
                    if (setRedir(commands[j].nextConnector,
                                 commands[j+1].command) == -1) {
                        exit(EXIT_FAILURE);
                    }
                    j++;
                }
                if (commands[j].nextConnector == PIPE) {
                    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                        perror("dup2");
                        return 1;
                    }
                    if (close(pipefd[0]) == -1) {
                        perror("close");
                        return 1;
                    }
                }
                execCommand(commands[i].command);
            }

            else {
                if (wait(&cmd_status) == -1) {
                    perror("wait: ");
                }
                if (WIFEXITED(cmd_status)) {
                    cmd_status = WEXITSTATUS(cmd_status);
                }
            }
            prevConnector = commands[i].prevConnector;
            while (isRedir(commands[i].nextConnector)) {
                i++;
            }
            nextConnector = commands[i].nextConnector;
            if (commands[i].nextConnector == PIPE) {
                if ((dup2(pipefd[0], STDIN_FILENO)) == -1)
                    perror("dup2");
                if (close(pipefd[1]) == -1)
                    perror("close");
            }
        }
        execute = checkStatus(cmd_status, commands[i].nextConnector);
        if ( prevConnector == PIPE && nextConnector != PIPE) {
            if ((dup2(savestdin, STDIN_FILENO)) == -1)
                perror("dup2");
        }
        i++;
        if (cmd_status == -1) status = cmd_status;
    }
    return status;
}


/*
 * Returns true if connector is one of the redirection connectors.
 */
bool isRedir(const std::string &connector)
{
    if (connector == "") {
        return false;
    }
    return (connector == REDIR_IN || connector.substr(connector.length()-1) == REDIR_OUT);
}

int countPipes(const std::vector<Command> &commands, int index)
{
    int count = 0;
    while (commands[index].nextConnector == PIPE) {
        index++;
        count++;
        while (isRedir(commands[index].nextConnector)) {
            index++;
        }
    }
    return count;
}

/*
 * Sets the appropriate redirect based on which connector is passed.
 * next is the next command which in this case will be a file
 */
int setRedir(const std::string &connector, const std::string &next)
{
    std::string file = next;
    strip(file);
    if (connector == REDIR_OUT) {
        int fd = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            perror("setRedir: open");
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("setRedir: dup2");
            return -1;
        }
    }
    else if (connector == REDIR_IN) {
        int fd = open(file.c_str(), O_RDONLY);
        if (fd == -1) {
            perror("setRedir: open");
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("setRedir: dup2");
            return -1;
        }
    }
    else if (connector == REDIR_OUT_APP) {
        int fd = open(file.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1) {
            perror("setRedir: open");
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("setRedir: dup2");
            return -1;
        }
    }
    else {
        return 1;
    }
    return 0;
}

bool firstWord(const std::string &command, const std::string &word)
{
    std::string cmd = command;
    stripLeadingSpaces(cmd);
    return (cmd.substr(0, std::string(word).length()) == word);
}

/*
 * Checks the status of a connector based on what value status is.
 * if staus > 1 then the command failed
 * if status = 0 then the command succeeded
 */
bool checkStatus(const int status, const std::string &connector)
{
    bool execute = true;
    if (status == -1) {
        // exit was called in execCommand
        execute = false;
    }
    else if(connector == COMMENT || connector == "") {
        execute = false;
    }
    else if(connector == CONNECTOR) {
        execute = true;
    }
    else if(connector == AND_CONNECTOR) {
        execute = !(status);
    }
    else if(connector == OR_CONNECTOR) {
        execute = status;
    }
    else {
        execute = true;
    }
    return execute;
}

/*
 * Executes whatever is in the string 'command' with execvp
 * Can have extra whitespace in command but the command must be clear of any
 * connectors or comment characters in order to work correctly.
 * DOES not return. Always call this in a child process
 */
void execCommand(std::string command)
{
    signal(SIGINT, childSigHandler);
    int token_count = strip(command);
    char *c_command = new char[command.length()+1];
    strcpy(c_command, command.c_str());
    char *tok = strtok(c_command, " ");

    char **args = new char*[token_count+1];
    for (int i = 0; i < token_count; i++) {
        args[i] = tok;
        tok = strtok(NULL, " ");
    }
    args[token_count] = 0; // null terminate the array

    std::vector<std::string> paths;
    //make the current directory the first path to try:
    std::string current_dir = getCurrentDir();
    paths.push_back(current_dir);
    if (getPath(paths) == -1)  {
        perror("getPath");
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = 0; i < paths.size(); i++) {
        std::string path = paths[i];
        path += "/";
        path += args[0];
        execv(path.c_str(), args);
    }
    //Reached the end of the loop without finding a match in path
    perror("execv");
    delete[] args;
    delete[] c_command;
    _exit(EXIT_FAILURE);
}


int getPath(std::vector<std::string> &paths) {
    char *c_paths = getenv("PATH");
    if (c_paths == NULL) {
        return -1;
    }
    char *tok = strtok(c_paths, ":");
    while (tok != NULL) {
        paths.push_back(tok);
        tok = strtok(NULL, ":");
    }
    return 0;

}

std::string getCurrentDir() {
    char cwd[BUFSIZ];
    if (getcwd(cwd, BUFSIZ) == NULL) {
        perror("getCurrentDir: getcwd");
        return "";
    }
    return std::string(cwd);
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
 * Strips all leading spaces from str
 */
void stripLeadingSpaces(std::string &str)
{
    while (str[0] == ' ') {
       str = str.substr(1);
    }
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
    int size = 0;
    std::string command_str = input;
    std::string next_connector = "";
    strip(command_str);
    Command current_command;
    while(command_str != "") {
        int start = 0;
        stripLeadingSpaces(command_str);
        int next = nextDelim(command_str);
        std::string current_delim = getDelimAt(command_str, next);
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
        current_command.command = command_str.substr(0, next);
        commands.push_back(current_command);
        size++;
        start = next + current_delim.length();
        command_str = command_str.substr(start);
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
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    if (pwd == NULL) {
        perror("getlogin");
    }
    else {
        prompt += pwd->pw_name;
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
    std::string current_dir = getCurrentDir();
    prompt += ":";
    prompt += current_dir;
    prompt += "$ ";
    delete[] hostname;
    return prompt;
}


void childSigHandler(int signum)
{
    int pid = getpid();
    if (kill(pid, signum) == -1) {
        perror("kill");
        exit(EXIT_FAILURE);
    }
}

int cd(int argc, char *argv[]) {
    std::string full_path = "";
    if (argc == 1) {
        char *home = getenv("HOME");
        if (home == NULL) {
            perror("cd: getenv");
            return 1;
        }
        full_path = home;
    }
    else {
        if (argv[1][0] != '/') {
            full_path = getCurrentDir();
            full_path += "/";
        }
        full_path += argv[1];
    }
    if (chdir(full_path.c_str()) == -1) {
        perror("cd: chdir");
        return 1;
    }
    return 0;
}
