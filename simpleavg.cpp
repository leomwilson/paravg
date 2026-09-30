#include <chrono>
#include <iostream>
#include <fstream>


using namespace std;

int main(int argc, char** argv) {
    auto startTime = chrono::steady_clock::now();
    int count = 0;
    double sum = 0;

    if (argc != 2) {
        cout << "Usage: ./simpleavg numbers.txt" << endl;
        return 1;
    }

    ifstream infile(argv[1]);

    float num;

    while (infile >> num) {
        sum += num;
        count++;
    }

    auto endTime = chrono::steady_clock::now();
    auto duration = chrono::duration<double>(endTime - startTime).count();
    cout << "Computed average: " << (sum / count) << endl;
    cout << "Processed " << count << " numbers in " << duration << " seconds." << endl;

    return 0;
}