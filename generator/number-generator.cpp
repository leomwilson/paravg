#include <cstdio>
#include <iostream>
#include <fstream>
#include <random>

using namespace std;

int main (int argc, char** argv) {
    if (argc != 3) {
        cout << "Usage: ./number-generator numbers.txt 500" << endl;
        return 1;
    }

    ofstream OutFile(argv[1]);

    int count = stoi(argv[2]);

    std::default_random_engine randeng;
    std::uniform_int_distribution<int> randgen(0, 1000000 - 1);

    double sum = 0;

    for (int i = 0; i < count; i++) {
        float num = (float) randgen(randeng) / 100; // 0 - 9999.99
        sum += num;
        OutFile << num << endl;
    }

    cout << "Average: " << (sum / count) << endl;

    OutFile.close();

    return 0;
}