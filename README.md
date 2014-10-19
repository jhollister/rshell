#RSHELL

##About
Rshell is a basic shell for UNIX based operating systems written for University of California Riverside's CS100 class. 
####Features
* Can run executables located in the system's bin directories
* Chain commands together on one line using `; && ||`
* Comments are supported using `#`
* Username and hostname will be diplayed at the prompt

##How to use
Project source can be downloaded from https://github.com/jhollister/rshell.git
######To compile and run Rshell run the following commands:
```
git clone https://github.com/jhollister/rshell.git
cd rshell
make
bin/rshell
```
* Use the `;` connector to chain multiple commands together
* Use the `&&` connector and the next command will only run if the previous succeeded
* Use the `||` connector and the next commands will only run if the previous failed
* Use `#` and everything after will be ignored by the program
* Type `exit` at the beginning of any command to exit rshell


##Bugs
* Quotation marks are not completely supported and this can break some commands such as `git commit -m "This is a commit"`. All of the space delimited tokens: `"This` `is` `a` `commit"` will be passed as arguments to git when git only expects one token: `"This is a commit"`
* When there is a syntax error in the command such as improper use of connectors the commands before the syntax error will still be exectuted. This can make it hard to debug complex commands with a mistake because it's hard to find out what executed and where the mistake was. A better way would be to do what bash does and not execute anything if there is a syntax error.
* Typing Ctrl-C after starting a program inside rshell does not close that program as expected, instead it closes Rshell.
* `~` does not substitute for home as it does in bash


##License
All source files released under MIT license. See LICENSE file.

