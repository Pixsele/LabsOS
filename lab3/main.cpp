#include <cstring>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <valarray>
#include <vector>
#include <fstream>

using namespace std;

enum notation {
    error,
    binary,
    octal,
    hexadecimal,
    ch11,
    ch12,
    ch13,
    ch14,
    ch15
};


int pipe_in[2];
int pipe_out[2];
pid_t pid;
string infile;
string outfile;

void help_message() {
    cout << "Help:"<< endl;
    cout << "--help: Show this help" << endl;
    cout << "<input_file> <output_file>: Read and Write from files" << endl;
}

vector<char> to_valid(const notation notation) {
    switch (notation) {
        case binary:
            return {'0','1'};
        case octal:
            return {'0','1','2','3','4','5','6','7'};
        case hexadecimal:
            return {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
        case ch11:
            return {'0','1','2','3','4','5','6','7','8','9','A'};
        case ch12:
            return {'0','1','2','3','4','5','6','7','8','9','A','B'};
        case ch13:
            return {'0','1','2','3','4','5','6','7','8','9','A','B','C'};
        case ch14:
            return {'0','1','2','3','4','5','6','7','8','9','A','B','C','D'};
        case ch15:
            return {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E'};
        default:
            return {};
    }
}

notation ch_notation(const char type) {
    notation select_notation;
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
        case '4':
            select_notation = ch11;
            break;
        case '5':
            select_notation = ch12;
            break;
        case '6':
            select_notation = ch13;
            break;
        case '7':
            select_notation = ch14;
            break;
        case '8':
            select_notation = ch15;
            break;
        default:
            select_notation = error;
    }
    return select_notation;
}

notation select_notation() {
    cout << "Select number system:" << endl;
    cout << "1 -- Binary" << endl;
    cout << "2 -- Octal" << endl;
    cout << "3 -- Hexadecimal" << endl;
    cout << "4 -- Ch11" << endl;
    cout << "5 -- Ch12" << endl;
    cout << "6 -- Ch13" << endl;
    cout << "7 -- Ch14" << endl;
    cout << "8 -- Ch15" << endl;
    notation select_notation = notation::binary;
    char type;
    bool error_type = true;
    while (error_type) {
        error_type = false;
        cin >> type;
        select_notation = ch_notation(type);
        if (select_notation == notation::error) {
            error_type = true;
            cout << "Invalid input." << endl;
        }
    }
    return select_notation;
}

bool correct_number(const notation notation, const string& number) {
    bool error_number = false;
    vector<char> valid = to_valid(notation);
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
    return error_number;
}

string enter_number(const notation notation) {
    cout << "Enter double number: "<< endl;
    string number;
    bool error_number = true;
    while (error_number) {
        error_number = false;
        cin >> number;
        error_number = correct_number(notation, number);
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
    const int dig = to_valid(notation).size();
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
    notation notation;
    read(pipe_in[0],&notation,sizeof(notation));

    int len;
    read(pipe_in[0],&len , sizeof(len));
    char * buffer = new char[len+1];
    read(pipe_in[0], buffer, len);
    buffer[len] = '\0';
    const double result = to_decimal(buffer, notation);
    write(pipe_out[1], &result, sizeof(result));
    delete[] buffer;
}

void client(const bool file_flag) {
    if (file_flag) {
        ifstream file(infile);
        if (!file.is_open()) {
            cerr << "Unable to open file " << infile << endl;
            exit(1);
        }
        char type;
        file >> type;
        string number;
        file >> number;
        file.close();
        const notation notation = ch_notation(type);
        if (notation == notation::error) {
            cout << "Invalid notation in file." << endl;
            exit(1);
        }
        if (correct_number(notation, number)) {
            cout << "Invalid number in file." << endl;
            exit(1);
        }
        write(pipe_in[1],&notation , sizeof(notation));
        const int len = number.length();

        write(pipe_in[1], &len, sizeof(len));
        write(pipe_in[1], number.c_str(), number.length());

        double result;
        read(pipe_out[0], &result, sizeof(result));
        ofstream file_out(outfile);
        file_out << result;
        file_out.close();
    } else {
        const notation notation = select_notation();
        write(pipe_in[1], &notation, sizeof(notation));

        const string number = enter_number(notation);
        const int len = number.length();

        write(pipe_in[1], &len, sizeof(len));
        write(pipe_in[1], number.c_str(), number.length());

        double result;
        read(pipe_out[0], &result, sizeof(result));
        cout << "Result: " << result << endl;
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

int main(const int argc, char const *argv[]) {
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        help_message();
        return 0;
    }
    if (argc == 1) {
        process(false);
        return 0;
    }
    if (argc == 3) {
        infile = argv[1];
        outfile = argv[2];
        process(true);
        return 0;
    }
    return 1;
}


