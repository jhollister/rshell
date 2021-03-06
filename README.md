#RSHELL

##About
Rshell is a basic shell for UNIX based operating systems written for University of California Riverside's CS100 class. Included in rshell is an implementation of gnu ls, which is compiled and run separately.
####Features

######rshell
* Can run executables located in your current directory or your PATH.
* Chain commands together on one line using `; && ||`
* Redirect input and output with `> >> < |`
* Comments are supported using `#`
* Username, hostname and current directory will be diplayed at the prompt

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

######To run rshell enter the following:
```
bin/rshell
```

* Use the `;` connector to chain multiple commands together
* Use the `&&` connector and the next command will only run if the previous succeeded
* Use the `||` connector and the next commands will only run if the previous failed
* Chain more than one command together and the commands will be read in left to right order. Everything to the right of a connector is considered one command and will not be executed depending on whether the previous command succeeded or failed. For instance: `echo test || ls ; echo done`  Only `echo test` will be executed and both `ls` and `echo done` will not.
* Use `#` and everything after will be ignored by the program
* Use `>` to redirect output to a file.
* Use `<` to redirect input from a file to a command.
* Use `|` to pipe output from one command into standard input of another command.
* Use `cd` to change to home directory
* Use `cd [path]` to change to a path, either relative or absolute.
* `Ctrl-C` will send an interrupt signal to a running process.
* `Ctrl-Z` will stop a running process.
  * View stopped and background process with `jobs`
  * Bring the last process in `jobs` list to foreground with `fg`
  * Or choose a process to bring to foreground with `fg [# on jobs list]`
  * Bring the last process in `jobs` list to background with `bg`
  * Or chooose a process to bring to foreground with `bg [# on jobs list]`
* Type `exit` at the beginning of any command to exit rshell. NOTE: Any running or stopped jobs will be killed.

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
######rshell
* Quotation marks are not completely supported and this can break some commands such as `git commit -m "This is a commit"`. All of the space delimited tokens: `"This` `is` `a` `commit"` will be passed as arguments to git when git only expects one token: `"This is a commit"`
* `~` does not substitute for home as it does in bash
* Character limit of 4095 characters.
* Output redirection before a pipe can cause unexpected results.
* Input redirection after a pipe can cause unexpected results.
* Stopping a job that uses stdin will sometimes close automatically when brought to foreground, usually if you run a process that also uses stdin before bringing it to foreground.
* `bg` does not work well with programs that use stdin or complex programs such as vim. 

######ls
* Output format does not display correctly with a lot of small files.
* Output format is not as compact as the real ls.
* Output is displayed alphabetically from left to right as opposed to real ls where it is displayed alphabetically by column.
* Colors do not stack. So if a file is hidden and and executable it will still have black text but a gray background.

##License
All source files released under MIT license. See LICENSE file.

