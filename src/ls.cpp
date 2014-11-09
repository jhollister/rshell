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


#define F_ALL     0x1
#define F_LIST    0x2
#define F_RECURSE 0x4

struct file_output {
    std::string file_name;
    std::string file_details;
};

int getFlags(int argc, char** argv);
int getArgs(int argc, char** argv, char** files);
int printDetails(const std::string path);
std::string getFileType(const std::string path);
std::string getFileType(struct stat& stat_buf);
std::string getFilePermissions(struct stat& stat_buf);
std::string getFileUser(struct stat& stat_buf);
std::string getFileGroup(struct stat& stat_buf);
std::string getFileLinks(struct stat& stat_buf);
std::string getFileTime(struct stat& stat_buf);
std::string getFileSize(struct stat& stat_buf);




int main(int argc, char** argv)
{
    argv++;
    argc--;
    int flags = getFlags(argc, argv);
    char** files = new char*[argc];
    int arg_length = getArgs(argc, argv, files);
    printDetails("Makefile");
    flags++;
    arg_length++;
    delete[] files;
    return 0;
}

void printDir(const char* path, int flags) {
    DIR *dirp = opendir(path);
    if (dirp == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    dirent *direntp;
    while ((direntp = readdir(dirp)))
        std::cout << direntp->d_name << std::endl;  // use stat here to find attributes of file
    closedir(dirp);
}


int printDetails(const std::string path) {
    struct stat stat_buf;
    if (stat(path.c_str(), &stat_buf) == -1) {
        perror("stat");
        return -1;
    }
    std::cout << getFileType(stat_buf);
    std::cout << getFilePermissions(stat_buf) << " ";
    std::cout << getFileLinks(stat_buf) << " ";
    std::cout << getFileUser(stat_buf) << " ";
    std::cout << getFileGroup(stat_buf) << " ";
    std::cout << getFileSize(stat_buf) << " ";
    std::cout << getFileTime(stat_buf) << " ";
    return 0;
}

std::string getFileTime(struct stat& stat_buf) {
    std::string time(ctime(&stat_buf.st_mtime));
    return time;
}

std::string getFileSize(struct stat& stat_buf) {
    std::string size;
    std::ostringstream convert;
    convert << (long) stat_buf.st_size;
    size = convert.str();
    return size;
}

std::string getFileLinks(struct stat& stat_buf) {
    std::string links;
    std::ostringstream convert;
    convert << (long) stat_buf.st_nlink;
    links = convert.str();
    return links;
}


std::string getFileGroup(struct stat& stat_buf) {
    struct group* grp = getgrgid(stat_buf.st_gid);
    std::string group;
    if (!grp) {
        perror("getpwuid");
        group = "?";
    }
    else {
        group = grp->gr_name;
    }
    return group;
}
std::string getFileUser(struct stat& stat_buf) {
    struct passwd* pwd = getpwuid(stat_buf.st_uid);
    std::string username;
    if (!pwd) {
        perror("getpwduid");
        username = "?";
    }
    else {
        username = pwd->pw_name;
    }
    return username;
}

std::string getFilePermissions(struct stat& sb) {
    std::string perms("");
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

std::string getFileType(struct stat& sb) {
    std::string file_type;
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:
            file_type = "b";
            break;
        case S_IFCHR:
            file_type = "c";
            break;
        case S_IFDIR:
            file_type = "d";
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
        case S_IFSOCK:
            file_type = "s";
            break;
        default:
            file_type = "?";
            break;
    }
    return file_type;
}


std::string getFileType(const std::string path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) == -1) {
       perror("stat");
       return "?";
    }
    return getFileType(sb);
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
