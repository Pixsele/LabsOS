#include <iostream>
#include <cmath>
#include<string>
using namespace std;

string check_correct(const string& message) {
    string input;
    bool correct = false;
    while (!correct) {
        cout << message << endl;
        cin >> input;
        bool flag = true;
        int i = 0;
        if (input[0] == '-' || input[0] == '+') {
            i++;
        }
        int dot_count = 0;
        for (i; i < input.length(); i++) {
            if (input[i] != '.') {
                if (!isdigit(input[i]) || isalpha(input[i]) || input[i] == '8' || input[i] == '9') {
                    flag = false;
                }
            }else {
                dot_count++;
                if (dot_count>1) {
                    flag = false;
                }
            }
        }
        if (flag) {
            try {
                stod(input);
                correct = true;
            }catch(out_of_range) {
                cout << "INVALID INPUT,try again" << endl;
            }
        }else {
            cout << "INVALID INPUT,try again" << endl;
        }
    }
        return input;
}

bool check_cont() {

    string input;
    bool result;
    bool correct = false;

    while (!correct) {
        cout << "Continue?(y/n)" << endl;
        cin >> input;

        if (input == "y") {
            correct = true;
            result = true;
        }else if (input == "n") {
            correct = true;
            result = false;
        }
        else {
            cout << "INVALID INPUT,try again" << endl;
        }
    }
    return result;
}

double to_decimal(string input) {
    bool neg = input[0] == '-';
    if (neg || input[0] == '+') {
        input.erase(0, 1);
    }
    double result = 0;
    if (input.find('.') == string::npos) {
        for (int i = input.length() - 1;i >= 0;i--) {
            result += (input[i] - '0') * pow(8,input.length() - i - 1);
        }
    }else {
        const int dot = input.find('.');

        int power = 0;
        for (int i = dot - 1;i >= 0;i--) {
            result += (input[i] - '0') * pow(8,power++);
        }
        power = -1;
        for (int i = dot + 1;i < input.length();i++) {
            result += (input[i] - '0') * pow(8,power--);
        }
    }
    return result * (neg ? -1 : 1);
}

void to_bin(string input) {

    int a = stoi(input);
    int binary[32];
    int i = 0;
    while (a>0) {

        binary[i] = a%2;
        a = a/2;
        i++;
    }

    for (int j = i - 1;j >= 0;j--) {
        cout << binary[j];
    }
}

int main() {

    bool cont = true;

    while (cont) {
        string number_seven = check_correct("Enter a number in septenary number system : ");
        cout <<"Result in decimal number system: " << to_decimal(number_seven) << endl;
        cout << "Binary: ";
        to_bin(number_seven);
        cont = check_cont();
    }
}
