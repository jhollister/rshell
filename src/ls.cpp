#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
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
int getArgs(int argc, char** argv, char** files);
void printDir(const string, int flags);
void printFile(const string, int longest, int flags);
void printFile(const string,int flags);
void printFileList(const vector<string> &file_list, const string &parent, int flags);
void printDetails(const string path, int flags);
void printDetails(const vector<string> &file_names, const string &parent, int flags);
bool isDir(const string path);
void sortFiles(vector<string> &file_list);
bool fileCompare(string i, string j);
int longestString(const vector<string> &file_list);
int charCount(const vector<string> &file_list);
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
    char** files = new char*[argc];
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
            if (isDir(files[i])) {
                if (arg_length > 1) cout << files[i] << ":" << endl;
                printDir(files[i], flags);
                if (i < (arg_length - 1)) cout << endl;
            }
            else {
                printFile(files[i], flags & F_ALL);
                cout << endl;
                if (i < (arg_length - 1)) cout << endl;
            }
        }
    }

    delete[] files;
    return 0;
}

void printDir(const string path, int flags) {
    vector<string> file_names;
    vector<string> dir_list;
    DIR *dirp = opendir(path.c_str());
    if (dirp == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        file_names.push_back(direntp->d_name);
        //if(isDir(path.append(direntp->d_name)) && (flag & F_RECURSE))
    }
    closedir(dirp);
    sortFiles(file_names);
    //printFileList(file_names, path, flags);
    printDetails(file_names, path, flags);
}

void printFileList(const vector<string> &file_list, const string &parent, int flags) {
    int longest = longestString(file_list);
    int term_width = 80;//assume terminal width of 80 characters
    int line_break = term_width / longest;
    for (int i = 0; i < (int)file_list.size(); i++) {
        string full_path = parent;
        if (parent[parent.length()] != '/') {
            full_path += "/";
        }
        full_path += file_list[i];
        //printDetails(full_path);
        if (charCount(file_list) < term_width) {
            printFile(file_list[i], flags);
        }
        else {
            if (i % line_break == 0 && i != 0) cout << endl;
            printFile(file_list[i], longest,flags);
        }
    }
    cout << endl;
}

void printFile(const string file_name, int longest, int flags) {
    cout << left <<  setw(longest) << file_name << "  ";
}

void printFile(const string file_name, int flags) {
    printFile(file_name, 0, flags);
}



void printDetails(const string path, int flags) {
    struct stat stat_buf;
    if (stat(path.c_str(), &stat_buf) == -1) {
        perror("stat");
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
    if (parent_slash[parent.length()] != '/')
        parent_slash += "/";
    if (file_names.size() == 1) {
        //no formatting necessary
        printDetails(parent_slash.append(file_names[0]), flags);
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
        if (stat(full_path.c_str(), &stat_buf) == -1) {
            perror("stat");
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
        cout << left;
        cout << setw(longestString(perms)) << perms[i] << " ";
        cout << setw(longestString(links)) << links[i] << " ";
        cout << setw(longestString(users)) << users[i] << " ";
        cout << setw(longestString(groups)) << groups[i] << " ";
        cout << setw(longestString(sizes)) << sizes[i] << " ";
        cout << setw(longestString(times)) << times[i] << " ";
        printFile(file_names[i], flags ^ F_LIST); //clear flag so details aren't printed
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
        perror("getpwuid");
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


bool isDir(string path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == -1) {
       perror("stat");
       return false;
    }
    return S_ISDIR(sb.st_mode);
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
int getArgs(int argc, char** argv, char** files)
{
    int size = 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] != '-') {
            files[size++] = argv[i];
        }
    }
    return size;
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
