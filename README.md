#RSHELL

##About
Rshell is a basic shell for UNIX based operating systems written for University of California Riverside's CS100 class. Included in rshell is an implementation of gnu ls, which is compiled and run separately.
####Features
######rshell
* Can run executables located in the system's bin directories
* Chain commands together on one line using `; && ||`
* Comments are supported using `#`
* Username and hostname will be diplayed at the prompt

######ls
* Displays files in columns that scale to the size of your terminal.
* Colorized output where directories are blue, executables are green, symbolic links are cyan, and hidden files have a gray background.
* Symbolic links will point to their link when the `-l` flag is passed. If the link is dead it will display it in red.

##How to use
Project source can be downloaded from https://github.com/jhollister/rshell.git
######To compile both rshell and ls run the following commands:
```
git clone https://github.com/jhollister/rshell.git
cd rshell
make
```
#####To run rshell enter the following:
```
bin/rshell
```

* Use the `;` connector to chain multiple commands together
* Use the `&&` connector and the next command will only run if the previous succeeded
* Use the `||` connector and the next commands will only run if the previous failed
* Chain more than one command together and the commands will be read in left to right order. Everything to the right of a connector is considered one command and will not be executed depending on whether the previous command succeeded or failed. For instance: `echo test || ls ; echo done`  Only `echo test` will be executed and both `ls` and `echo done` will not.
* Use `#` and everything after will be ignored by the program
* Type `exit` at the beginning of any command to exit rshell

######To run ls:
```
bin/ls [FLAG]... [FILE]...
```
* Flags for ls include -a -l -R and can be used in any order and included anywhere in command
* -a  - Show all files including hidden
* -l  - Show details of each file
* -R  - Recursively print the directory and every subdirectory
* [FILE] can be any number of files, each will be handled in the order they are entered.


##Bugs and Limitations
* Quotation marks are not completely supported and this can break some commands such as `git commit -m "This is a commit"`. All of the space delimited tokens: `"This` `is` `a` `commit"` will be passed as arguments to git when git only expects one token: `"This is a commit"`
* When there is a syntax error in the command such as improper use of connectors the commands before the syntax error will still be exectuted. This can make it hard to debug complex commands with a mistake because it's hard to find out what executed and where the mistake was. A better way would be to do what bash does and not execute anything if there is a syntax error.
* Typing `Ctrl-C` after starting a program inside rshell does not close that program as expected, instead it closes Rshell. This  causes a memory leak as well since deallocation is done in the parent process and isn't able to take place.
* `~` does not substitute for home as it does in bash
* When bringing rshell into the foreground with `fg` after suspending it with `Ctrl-z` the prompt does not display
* Using ctrl-z while a child process is running suspends all of rshell.
* Character limit of 4095 characters.


##License
All source files released under MIT license. See LICENSE file.

