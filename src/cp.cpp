#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <time.h>

using namespace std;

void printError();

void copycpp(const char* input, const char* output){

	ifstream in;
	fstream out;
	if (access(input, F_OK) == -1){
		cout << "Input file does not exist." << endl;
		exit(1);
    }
	in.open(input);
	if (access(output, F_OK) != -1){
		cout << "File already exists." << endl;
		exit(1);
	}
	out.open(output, fstream::out);
	char temp = in.get();
	while (in.good()){
		out.put(temp);
        temp = in.get();
	}
	in.close();
	out.close();

}

void copySys(const char* input, const char* output ){
	int in = open(input, O_RDONLY);
	if (in == -1){
		perror("open");
		exit(1);
	}
	int out = open(output, O_WRONLY | O_CREAT , S_IWUSR | S_IRUSR);
	if (out == -1){
		perror("open out");
		exit(1);
	}
	char ch;
	int r = read (in,&ch, sizeof(ch));
	while (r != 0){
		if (r == -1){
			perror("read error");
			close(in);
			close(out);
			exit(1);
		}
		int w = write(out, &ch, sizeof(char));
		if (w == -1){
			perror("write error");
			close(in);
			close(out);
			exit(1);
		}
		r = read(in, &ch, 1);
	}
	close(in);
	close(out);
}


void copyBuf(const char* input, const char* output ){
	int in = open(input, O_RDONLY);
	if (in == -1){
		perror("open");
		exit(1);
	}
	int out = open(output, O_WRONLY | O_CREAT , S_IWUSR | S_IRUSR);
	if (out == -1){
		perror("open out");
		exit(1);
	}
	char ch[BUFSIZ];
	int r = read (in, ch, sizeof(ch));
	while (r != 0){
		if (r == -1){
			perror("read error");
			close(in);
			close(out);
			exit(1);
		}
		int w = write(out, ch, r);
		if (w == -1){
			perror("write error");
			close(in);
			close(out);
			exit(1);
		}
		r = read(in, ch, sizeof(ch));
	}
	close(in);
	close(out);
}

int main (int argc, char* argv[]){
    vector<const char *> files;
    int num_files = 0;
    char flag;
    int num_flags = 0;
    argc--;
    argv++;
    if (argc == 0) {
        printError();
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            flag = argv[i][1];
            if (argv[i][2] != 0) {
                cout << "Error: Too many flags" << endl;
                printError();
                exit(EXIT_FAILURE);
            }
            num_flags++;
        }
        else {
            files.push_back(argv[i]);
            num_files++;
        }
    }
    if (num_files != 2) {
        cout << "Error: 2 files must be added as arguments" << endl;
        printError();
        exit(EXIT_FAILURE);
    }
    if (num_flags > 1) {
        cout << "Error: Too many flags" << endl;
        printError();
    }
	if (access(files[1], F_OK) != -1){
		cout << "Error: DESTINATION file already exists." << endl;
		exit(EXIT_FAILURE);
	}
    if (flag == 'f') {
        copycpp(files[0], files[1]);
    }
    else if (flag == 'c') {
        copySys(files[0], files[1]);
    }
    else {
        copyBuf(files[0], files[1]);
    }
    return 0;
}

void printError() {
    cout << "Usage: cp [flag] SOURCE DEST" << endl;
    cout << "Flags: -f=fstream copy" << endl;
    cout << "       -c=read/write char by char copy" << endl;
    cout << "       -b=read/write buffer copy" << endl;
    cout << "       no flag=buffer copy" << endl;

}
