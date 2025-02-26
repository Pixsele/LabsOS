#include <regex>
#include <cstring>
#include <fstream>
#include<iostream>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

void err(const string& message) {
    cout << message << endl;
    cout << "--help view commads" << endl;
}

//доп
void rm(const char* file) {
    if (unlink(file) != 0) {
        err("Unlink error");
    }else {
        cout << "Remove file " << file << endl;
    }
}

void cp(char* filename, char* newfile) {
    if (strcmp(filename,newfile) == 0) {
        err("File can't copied into self");
    }else {
        ifstream in;
        ofstream out;
        in.open(filename,ios::binary);

        if (in.is_open()) {
            int sizebuffer = 100;
            char* buffer = new char[sizebuffer];
            out.open(newfile, ios::binary);

            while (!in.eof()) {
                in.read(buffer, sizebuffer);
                if (in.gcount()) {
                    out.write(buffer, in.gcount());
                }
            }
            delete[] buffer;
        }else {
            err("Can't open file");
        }
        in.close();
        out.close();
    }
}

void mv(const char* file, const char* dir) {

    ifstream in;
    in.open(file,ios::binary);
    if (!in.is_open()) {
        err("Can't open file");
    }else {
        bool flag = true;
        int lenFileName = 0;
        for (size_t i = strlen(file)-1; i > 0 && flag; i--) {
            if (file[i] == '/') {
                flag = false;
            }else {
                lenFileName++;
            }
        }
        char* fileName = new char[lenFileName];
        strcpy(fileName, file+strlen(file)-lenFileName);
        cout << fileName << endl;
        char * newDir = new char[strlen(dir) + lenFileName + 2];

        strcpy(newDir, dir);
        strcpy(newDir + strlen(dir), "/");
        strcpy(newDir+strlen(dir)+1, fileName);
        strcpy(newDir+strlen(dir)+1+lenFileName+1, "\0");

        rename(file, newDir);
        delete[] fileName;
        delete[] newDir;
    }

    in.close();
}

const char* file_mode(const struct stat &bf) {
    char* mode = new char[10];
    mode[0] = (bf.st_mode & S_IRUSR) ? 'r' : '-';
    mode[1] = (bf.st_mode & S_IWUSR) ? 'w' : '-';
    mode[2] = (bf.st_mode & S_IXUSR) ? 'x' : '-';
    mode[3] = (bf.st_mode & S_IRGRP) ? 'r' : '-';
    mode[4] = (bf.st_mode & S_IWGRP) ? 'w' : '-';
    mode[5] = (bf.st_mode & S_IXGRP) ? 'x' : '-';
    mode[6] = (bf.st_mode & S_IROTH) ? 'r' : '-';
    mode[7] = (bf.st_mode & S_IWOTH) ? 'w' : '-';
    mode[8] = (bf.st_mode & S_IXOTH) ? 'x' : '-';
    mode[9] = '\0';
    return mode;
}

const char* file_type(const struct stat& bf) {
    if (S_ISDIR(bf.st_mode)) return "directory";
    if (S_ISREG(bf.st_mode)) return "regular file";
    if (S_ISLNK(bf.st_mode)) return "symbolic link";
    if (S_ISCHR(bf.st_mode)) return "character special";
    if (S_ISBLK(bf.st_mode)) return "block device";
    if (S_ISFIFO(bf.st_mode)) return "fifo";
    if (S_ISSOCK(bf.st_mode)) return "socket";
    return "other";
}

void file_info(const char* file) {
    struct stat bf{};

    if (lstat(file,&bf)) {
        err("Incorrect file name");
    }else {
        const char *mode = file_mode(bf);
        cout << "File name: " << file << endl;
        cout << "File type: " << file_type(bf) << endl;
        cout << "File mode: " << mode << endl;
        cout << "File size: " << bf.st_size << " byte" <<endl;
        cout << "File time edited: " << ctime(&bf.st_mtime) << endl;

        delete[] mode;
    }
}

int  bit_mask(const char* mode) {
    int ans = 0;
    for (int i = 0, x = 256;i < 9;i++,x/=2) {
        if (mode[i] != '-') {
            ans += x;
        }
    }
    return ans;
}

void change_mode(const char* file, const char* mode) {
    const regex patternS{"(r|-)(w|-)(x|-)(r|-)(w|-)(x|-)(r|-)(w|-)(x|-)"};
    const regex patternDigit{"([0-7])([0-7])([0-7])"};
    if (regex_match(mode,patternS)) {
        chmod(file,bit_mask(mode));
        cout << "Mode: " << file << " successful changed to: " << mode <<  endl;
    }else if (regex_match(mode,patternDigit)) {
        chmod(file,stoi(mode,nullptr,8));
        cout << "Mode: " << file << " successful changed to: " << mode <<  endl;
    }else {
        cout << "Incorrect mode" << endl;
    }
}

int main(const int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        cout << "Help:"<< endl;
        cout << "--help: Show this help" << endl;
        cout << "--move <file> <dir> move file into directory" << endl;
        cout << "--copy <file> <newfile> copy file" << endl;
        cout << "--info <file> show info file" << endl;
        cout << "--chmod <file> <mode> change mode of file, <mode> = [0-7,0-7,0-7] or [rwxrwxrwx]" << endl;
        cout << "--remove <file> remove file" << endl;
    }else if (argc == 3 && strcmp(argv[1], "--info") == 0) {
        file_info(argv[2]);
    }else if (argc == 3 && strcmp(argv[1], "--remove") == 0) {
        rm(argv[2]);
    }else if (argc == 4 && strcmp(argv[1], "--move") == 0) {
        mv (argv[2], argv[3]);
    }else if (argc == 4 && strcmp(argv[1], "--copy") == 0) {
        cp (argv[2], argv[3]);
    }else if (argc == 4 && strcmp(argv[1], "--chmod") == 0) {
        change_mode (argv[2], argv[3]);
    }else {
        err("Incorrect arguments");
    }
}


