#include <iostream>
#include <dirent.h>

#define F_ALL     0x1
#define F_LIST    0x2
#define F_RECURSE 0x4


int getFlags(int argc, char** argv);
int getFiles(int argc, char** argv, char** files);
void printFiles(int size, char** files, int flags);
void printFile(char* file, int flags);

int main(int argc, char** argv)
{
    argv++;
    argc--;
    getFlags(argc, argv);
    char** files = new char*[argc];
    getFiles(argc, argv, files);
    DIR* dir = opendir(files[0]);
    std::cout << readdir(dir)->d_name << std::endl;
    delete[] files;
    return 0;
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
int getFiles(int argc, char** argv, char** files)
{
    int size = 0;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] != '-') {
            files[size++] = argv[i];
        }
    }
    return size;
}


