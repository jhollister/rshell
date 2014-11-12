#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>


#define F_ALL     0x1
#define F_LIST    0x2
#define F_RECURSE 0x4

using namespace std;

int getFlags(int argc, char** argv);
int getArgs(int argc, char** argv, vector<string> &files);
void printDir(const string, int flags);
void printFile(const string, const string parent,int longest, int flags);
void printFile(const string,const string parent,int flags);
void printFileList(const vector<string> &file_list, const string &parent, int flags);
void printDetails(const string path, int flags);
void printDetails(const vector<string> &file_names, const string &parent, int flags);
bool isDir(const string path);
bool isExe(const string path);
bool isLink(const string path);
void sortFiles(vector<string> &file_list);
bool fileCompare(string i, string j);
int longestString(const vector<string> &file_list);
int charCount(const vector<string> &file_list);
void removeDotfiles(vector<string> &file_list);
int getTerminalWidth();
string getFileType(struct stat& stat_buf);
string getFilePermissions(struct stat& stat_buf);
string getFileUser(struct stat& stat_buf);
string getFileGroup(struct stat& stat_buf);
string getFileLinks(struct stat& stat_buf);
string getFileTime(struct stat& stat_buf);
string getFileSize(struct stat& stat_buf);



int main(int argc, char** argv)
{
    argv++;
    argc--;
    vector<string> files;
    int flags = getFlags(argc, argv);
    if (flags == -1) {
        cerr << "Undefined flag passed\n";
        cerr << "Supported flags: -a -R -l\n";
        exit(EXIT_FAILURE);
    }
    int arg_length = getArgs(argc, argv, files);
    if (arg_length == 0) {
        printDir(".", flags);
    }
    else {
        for (int i = 0; i < arg_length; i++) {
            if (access(files[i].c_str(), F_OK) == -1) {
                perror(string("access: " + files[i]).c_str());
            }
            else if (isDir(files[i])) {
                if (arg_length > 1  && !(flags & F_RECURSE)) {
                    cout << files[i] << ":" << endl;
                }
                printDir(files[i], flags);
                if (i < (arg_length - 1)) cout << endl;
            }
            else {
                if (F_LIST & flags) printDetails(files[i], flags);
                printFile(files[i], "", flags | F_ALL);
                cout << endl;
                if (i < (arg_length - 1)) cout << endl;
            }
        }
    }
    return 0;
}

void printDir(const string path, int flags) {
    string full_path = path;
    if (path[path.length() - 1] != '/') {
        full_path += "/";
    }
    if (flags & F_RECURSE) {
        cout << path <<  ":" << endl;
    }
    vector<string> file_names;
    vector<string> dir_list;
    DIR *dirp = opendir(path.c_str());
    if (dirp == NULL) {
        perror("printDir: opendir");
        return;
    }
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        string dir_path = full_path + direntp->d_name;
        file_names.push_back(direntp->d_name);
        if ( (flags & F_RECURSE) && isDir(dir_path)) {
            if ((string(direntp->d_name) != ".") && (string(direntp->d_name) != "..")) {
                dir_list.push_back(direntp->d_name);
            }
        }
    }
    closedir(dirp);
    sortFiles(dir_list);
    sortFiles(file_names);
    if (!(flags & F_ALL)) {
        removeDotfiles(file_names);
        removeDotfiles(dir_list);
    }
    if (!file_names.empty()) {
        if (flags & F_LIST) {
            printDetails(file_names, full_path, flags);
        }
        else {
            printFileList(file_names, full_path, flags);
        }
    }
    // recursive listings
    if (!dir_list.empty()) {
        for (int i = 0; i < (int) dir_list.size(); i++) {
            cout << endl;
            //don't follow sym links when doing recursive
            if (!isLink(full_path + dir_list[i])) {
                printDir(full_path + dir_list[i], flags);
            }
        }
    }
}

void printFileList(const vector<string> &file_list, const string &parent, int flags) {
    int longest = longestString(file_list) + 2; //add two to account for spaces
    int term_width = getTerminalWidth() - longest;
    int line_break = term_width / longest;
    if (line_break == 0) line_break++;
    for (int i = 0; i < (int)file_list.size(); i++) {
        if (charCount(file_list) < term_width) {
            printFile(file_list[i],parent, flags);
        }
        else {
            if (i % line_break == 0 && i != 0) cout << endl;
            printFile(file_list[i], parent, longest,flags);
        }
    }
    cout << endl;
}

void printFile(const string file_name, const string parent, int longest, int flags) {
    bool print = true;
    if (!(F_ALL & flags) && file_name[0] == '.') print = false;

    if(print) {
        string colorDir = "\033[0;34m";
        string colorExe = "\033[0;32m";
        string colorHidden = "\033[7;37m";
        string colorLink = "\033[0;36m";
        string color("");
        if (isLink(parent + file_name)) {
            color += colorLink;
        }
        else if (isDir(parent + file_name)) {
            color += colorDir;
        }
        else if (isExe(parent + file_name)) {
            color += colorExe;
        }
        if (file_name[0] == '.') {
            color += colorHidden;
        }
        cout << color;
        cout << left << setw(longest) << file_name << "\033[0m  ";
        if ((flags & F_LIST) && isLink(parent + file_name)) {
           char buf[BUFSIZ];
           int length = readlink(string(parent + file_name).c_str(), buf, BUFSIZ);
           if (length == -1) {
               perror("readlink");
               return;
           }
           buf[length] = 0; //append null character
           cout <<  setw(0) << "-> ";
           if (access(string(parent + buf).c_str(), F_OK) == -1) {
               // This system call checks if the file exists
               // if it doesn't exist then I will print out the link but color
               // it red to show that it is not a valid link
               // Please don't mark me down for not calling perror
               cout << setw(0) << "\033[0;31m" << buf << "\033[0m";
           }
           else {
               printFile(string(buf), parent, flags | F_ALL);
           }
        }
    }
}

void printFile(const string file_name, const string parent, int flags) {
    printFile(file_name, parent, 0, flags);
}



void printDetails(const string path, int flags) {
    struct stat stat_buf;
    if (lstat(path.c_str(), &stat_buf) == -1) {
        perror("printDetails: stat");
        return;
    }
    cout << getFileType(stat_buf) << getFilePermissions(stat_buf) << " ";
    cout << getFileLinks(stat_buf) << " ";
    cout << getFileUser(stat_buf) << " ";
    cout << getFileGroup(stat_buf) << " ";
    cout << getFileSize(stat_buf) << " ";
    cout << getFileTime(stat_buf) << " ";
}

void printDetails(const vector<string> &file_names, const string &parent, int flags) {
    string parent_slash = parent;
    if (parent_slash[parent.length() - 1] != '/')
        parent_slash += "/";
    if (file_names.size() == 1) {
        //no formatting necessary
        printDetails(parent_slash + file_names[0], flags);
        printFile(file_names[0], parent_slash, flags);
        cout << endl;
        return;
    }
    struct stat stat_buf;
    vector<string> perms(file_names.size());
    vector<string> links(file_names.size());
    vector<string> users(file_names.size());
    vector<string> groups(file_names.size());
    vector<string> sizes(file_names.size());
    vector<string> times(file_names.size());

    int total = 0;
    for (int i = 0; i < (int)file_names.size(); i++) {
        string full_path = parent_slash;
        full_path += file_names[i];
        if (lstat(full_path.c_str(), &stat_buf) == -1) {
            cerr << full_path << endl;
            perror("printDetails: stat");
            return;
        }
        //storing filetype in permissions as one "word"
        perms[i] = getFileType(stat_buf);
        perms[i] += getFilePermissions(stat_buf);
        links[i] = getFileLinks(stat_buf);
        users[i] = getFileUser(stat_buf);
        groups[i] = getFileGroup(stat_buf);
        sizes[i] = getFileSize(stat_buf);
        times[i] = getFileTime(stat_buf);
        //add to total dividing blocks by 2 because ls gives blocks in
        //1024 increments and st_blocks in 512 byte increments
        total += stat_buf.st_blocks / 2;
    }

    cout << "total " << total << endl;
    //now loop through and print, setting the correct width
    for (int i = 0; i < (int)file_names.size(); i++) {
        cout << right;
        cout << setw(longestString(perms)) << perms[i] << " ";
        cout << setw(longestString(links)) << links[i] << " ";
        cout << setw(longestString(users)) << users[i] << " ";
        cout << setw(longestString(groups)) << groups[i] << " ";
        cout << setw(longestString(sizes)) << sizes[i] << " ";
        cout << setw(longestString(times)) << times[i] << " ";
        printFile(file_names[i], parent_slash, flags);
        cout << endl;
    }
}

string getFileTime(struct stat& stat_buf) {
    string time(ctime(&stat_buf.st_mtime));
    return time.substr(4, ( time.substr(4).length() - 9) );
}

string getFileSize(struct stat& stat_buf) {
    string size;
    ostringstream convert;
    convert << (long) stat_buf.st_size;
    size = convert.str();
    return size;
}

string getFileLinks(struct stat& stat_buf) {
    string links;
    ostringstream convert;
    convert << (long) stat_buf.st_nlink;
    links = convert.str();
    return links;
}


string getFileGroup(struct stat& stat_buf) {
    struct group* grp = getgrgid(stat_buf.st_gid);
    string group;
    if (!grp) {
        perror("getgrgid");
        group = "?";
    }
    else {
        group = grp->gr_name;
    }
    return group;
}
string getFileUser(struct stat& stat_buf) {
    struct passwd* pwd = getpwuid(stat_buf.st_uid);
    string username;
    if (!pwd) {
        perror("getpwduid");
        username = "?";
    }
    else {
        username = pwd->pw_name;
    }
    return username;
}

string getFilePermissions(struct stat& sb) {
    string perms("");
    if (sb.st_mode & S_IRUSR) {
        perms += "r";
    }
    else perms += "-";
    if (sb.st_mode & S_IWUSR) {
        perms += "w";
    }
    else perms += "-";
    if (sb.st_mode & S_IXUSR) {
        perms += "x";
    }
    else perms += "-";
    if (sb.st_mode & S_IRGRP) {
        perms += "r";
    }
    else perms += "-";
    if (sb.st_mode & S_IWGRP) {
        perms += "w";
    }
    else perms += "-";
    if (sb.st_mode & S_IXGRP) {
        perms += "x";
    }
    else perms += "-";
    if (sb.st_mode & S_IROTH) {
        perms += "r";
    }
    else perms += "-";
    if (sb.st_mode & S_IWOTH) {
        perms += "w";
    }
    else perms += "-";
    if (sb.st_mode & S_IXOTH) {
        perms += "x";
    }
    else perms += "-";

    return perms;
}

string getFileType(struct stat& sb) {
    string file_type;
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:
            file_type = "b";
            break;
        case S_IFSOCK:
            file_type = "s";
            break;
        case S_IFCHR:
            file_type = "c";
            break;
        case S_IFIFO:
            file_type = "p";
            break;
        case S_IFLNK:
            file_type = "l";
            break;
        case S_IFREG:
            file_type = "-";
            break;
        case S_IFDIR:
            file_type = "d";
            break;
        default:
            file_type = "?";
            break;
    }
    return file_type;
}

bool isLink(const string path) {
    struct stat sb;
    if (lstat(path.c_str(), &sb) == -1) {
        perror("isLink: stat");
        return false;
    }
    return S_ISLNK(sb.st_mode);
}

bool isDir(const string path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == -1) {
       perror("isDir: stat");
       return false;
    }
    return S_ISDIR(sb.st_mode);
}

bool isExe(const string path) {
    struct stat sb;
    if (lstat(path.c_str(), &sb) == -1) {
        perror("isExe: stat");
        return false;
    }
    return (sb.st_mode & S_IXUSR);
}

// Loops through arguments and and sets flags
// Returns -1 if an undefined flag is passed
int getFlags(int size, char** argv)
{
    int flags = 0;
    for (int i = 0; i < size; i++) {
        if (argv[i][0] == '-') {
            char *arg = argv[i] + 1;
            while (*arg) {
                switch (arg[0]) {
                    case 'a':
                        flags |= F_ALL;
                        break;
                    case 'l':
                        flags |= F_LIST;
                        break;
                    case 'R':
                        flags |= F_RECURSE;
                        break;
                    default:
                        return -1;
                }
                arg++;
            }
        }
    }
    return flags;
}

// Finds non-flag arguments and add them to the file list.
// Returns the size of the list.
int getArgs(int argc, char** argv, vector<string> &files)
{
    int size = 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] != '-') {
            files.push_back(argv[i]);
            size++;
        }
    }
    return size;
}

int getTerminalWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

void sortFiles(vector<string> &file_list) {
    sort(file_list.begin(), file_list.end(), fileCompare);
}


bool fileCompare(string i, string j) {
    string first = i;
    string second = j;
    if (i == ".") return true;
    else if (j == ".") return false;
    else if (i == "..") return true;
    else if (j == "..") return false;

    if (i[0] == '.') {
        int ind = 1;
        //in case there is more than one dot in front of file
        while (i[ind] == '.') {
            ind++;
            if (!i[ind]) {
                return true; //only dots so we will try to put it up front
            }
        }
        first = i.substr(ind, string::npos);
    }
    if (j[0] == '.') {
        int ind = 1;
        //in case there is more than one dot in front of file
        while (j[ind] == '.') {
            ind++;
            if (!j[ind]) {
                return true; //only dots
            }
        }
        second = j.substr(ind, string::npos);
    }
    transform(first.begin(), first.end(), first.begin(), ::toupper);
    transform(second.begin(), second.end(), second.begin(), ::toupper);
    return first < second;
}

void removeDotfiles(vector<string> &file_list) {
    for (int i = 0; i < (int) file_list.size(); i++) {
        if (file_list[i][0] == '.') {
            file_list.erase(file_list.begin()+i);
            i--;
        }
    }
}

int longestString(const vector<string> &file_list) {
    int length = 0;
    for (int i = 0; i < (int)file_list.size(); i++) {
        if ((int)file_list[i].length() > length) {
            length = file_list[i].length();
        }
    }
    return length;
}

int charCount(const vector<string> &file_list) {
    int count = 0;
    for (int i = 0; i < (int)file_list.size(); i++) {
        count += (int) file_list[i].length();
    }
    count += 2 * ((int)file_list.size() - 1);// add spaces (2 between each file)
    return count;
}
