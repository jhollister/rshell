#include <iostream>

#define F_ALL     0x1
#define F_LIST    0x2
#define F_RECURSE 0x4


int getFlags(int argc, char** argv);

int main(int argc, char** argv)
{
    argv++;
    argc--;
    getFlags(argc, argv);
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
                std::cout << arg[0] << std::endl;
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
