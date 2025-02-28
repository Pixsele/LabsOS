#include <cstring>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <valarray>
#include <vector>
#include <fstream>

using namespace std;

enum notation {
    binary,
    octal,
    hexadecimal
};


int pipe_in[2];
int pipe_out[2];
pid_t pid;
string infile;
string outfile;

void help_message() {
    cout << "Usage:" << endl;
}

vector<char> to_valid(const notation notation) {
    switch (notation) {
        case binary:
            return {'0','1'};
        case octal:
            return {'0','1','2','3','4','5','6','7'};
        case hexadecimal:
            return {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    }
}

int dig_from_notation(const notation notation) {
    switch (notation) {
        case binary:
            return 2;
        case octal:
            return 8;
        case hexadecimal:
            return 16;
    }
}

notation select_notation() {
    cout << "Select number system:" << endl;
    cout << "1 -- Binary" << endl;
    cout << "2 -- Octal" << endl;
    cout << "3 -- Hexadecimal" << endl;
    notation select_notation = notation::binary;
    char type;
    bool error_type = true;
    while (error_type) {
        error_type = false;
        cin >> type;
        switch (type) {
            case '1':
                select_notation = binary;
            break;
            case '2':
                select_notation = octal;
            break;
            case '3':
                select_notation = hexadecimal;
            break;
            default:
                cout << "Invalid input,try again" << endl;
                error_type = true;
        }
    }
    return select_notation;
}

string enter_number(const notation notation) {
    vector<char> valid = to_valid(notation);
    cout << "Enter double number: "<< endl;
    string number;
    bool error_number = true;
    while (error_number) {
        error_number = false;
        cin >> number;
        int i = 0;
        if (number[0] == '-' or number[0] == '+') {
            i++;
        }
        int dot_count = 0;
        for (; i < number.length(); i++) {
            const char c = number[i];
            if (number[i] != '.') {
                bool correct = false;
                for (const char j : valid) {
                    if (c == j) {
                        correct = true;
                    }
                }
                if (!correct) {
                    error_number = true;
                }
            }
            else {
                dot_count++;
                if (dot_count > 1) {
                    error_number = true;
                }
            }
        }
        if (error_number) {
            cout << "Invalid input,try again" << endl;
        }
    }
    return number;
}

double to_decimal(string number, const notation notation) {
    const bool neg = number[0] == '-';
    if (neg || number[0] == '+') {
        number.erase(0, 1);
    }
    double decimal = 0;
    const int dig = dig_from_notation(notation);
    if (number.find('.') == string::npos) {
        for (int i = number.length() - 1; i >= 0; i--){
            int temp = 0;
            if (isdigit(number[i])) {
                temp = number[i] - '0';
            }else {
                temp = number[i] - 'A' + 10;
            }
            decimal += temp * pow(dig, (number.length() - i -1));
        }
    }else {
        const int dot = number.find('.');

        int power = 0;
        for (int i = dot - 1;i >= 0;i--) {
            int temp = 0;
            if (isdigit(number[i])) {
                temp = number[i] - '0';
            }else {
                temp = number[i] - 'A' + 10;
            }
            decimal += temp * pow(dig,power++);
        }
        power = -1;
        for (int i = dot + 1;i < number.length();i++) {
            int temp = 0;
            if (isdigit(number[i])) {
                temp = number[i] - '0';
            }else {
                temp = number[i] - 'A' + 10;
            }
            decimal += temp * pow(dig,power--);
        }
    }

    return decimal * (neg ? -1 : 1);
}

void server() {
    cout << "Server started." << endl;
    notation notation;
    read(pipe_in[0],&notation,sizeof(notation));

    int len;
    read(pipe_in[0],&len , sizeof(len));
    cout << "Log" << len << endl;
    char * buffer = new char[len+1];
    read(pipe_in[0], buffer, len);
    buffer[len] = '\0';
    cout << "Log" << buffer << endl;
    const double result = to_decimal(buffer, notation);
    cout <<"Log: " <<result << endl;
    write(pipe_out[1], &result, sizeof(result));
    delete[] buffer;
}

void client(const bool file_flag) {
    if (file_flag) {
        // ifstream file(infile);
        // if (!file.is_open()) {
        //     cerr << "Unable to open file " << infile << endl;
        //     exit(1);
        // }
        // cout << "Fiel" << endl;
        // char type;
        // file >> type;
        // string number;
        // file >> number;


    }else {
        const notation notation = select_notation();
        write(pipe_in[1],&notation , sizeof(notation));

        const string number = enter_number(notation);
        const int len = number.length();

        write(pipe_in[1], &len, sizeof(len));
        write(pipe_in[1], number.c_str(), number.length());

        double result;
        read(pipe_out[0], &result, sizeof(result));
        cout <<"Result: " <<result << endl;
    }

}

void process(const bool file_flag) {
    pipe(pipe_in);
    pipe(pipe_out);
    pid = fork();
    if (pid < 0) {
        cerr << "Fork failed" << endl;
        exit(1);
    }

    if (pid > 0) {
        client(file_flag);
    }else {
        server();
    }
    for (int i = 0; i < 2; i++) {
        close(pipe_in[i]);
        close(pipe_out[i]);
    }

}



int main(int argc, char const *argv[]) {
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help_message();
        return 0;
    }
    if (argc == 1) {
        process(false);
        return 0;
    }
    // if (argc == 3) {
    //     infile = argv[1];
    //     outfile = argv[2];
    //     process(true);
    //     return 0;
    // }

    return 1;
}


