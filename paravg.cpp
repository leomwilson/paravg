#include <chrono>
#include <iostream>
#include <fstream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <stop_token>
#include <unistd.h>

using namespace std;

mutex m; // protects all of these variables

int chunksize;
int chunkc;

int totalchunks = 0;
double totalsum = 0;

int prodi = 0; // next empty chunk
int consi = 0; // next full chunk
condition_variable_any fullcv;
int fullc = 0;
condition_variable_any emptycv;
int emptyc = 0;

float** chunks;
ifstream infile;

void worker(stop_token stop) {
    while (!stop.stop_requested()) {
        unique_lock<mutex> lock(m);
        fullcv.wait(lock, stop, []{ return fullc > 0;});
        if (stop.stop_requested()) { break; }
        float* data = chunks[consi];
        consi = (consi + 1) % chunkc;
        fullc--;
        lock.unlock();

        // work on the chunk, this won't be overwritten
        double sum = 0;

        for (int i = 0; i < chunksize; i++) {
            sum += data[i];
        }

        lock.lock();
        emptyc++;
        totalchunks++;
        totalsum += (sum / chunksize);

        lock.unlock();
        emptycv.notify_one();
    }
}

void reader(stop_token stop) {
    while(!stop.stop_requested()) {
        unique_lock<mutex> lock(m);
        emptycv.wait(lock, stop, []{ return emptyc > 0;});
        if (stop.stop_requested()) { break; }
        float* chunk = chunks[prodi];
        prodi = (prodi + 1) % chunkc;
        emptyc--;
        lock.unlock();

        // read in a new chunk
        bool incomplete = false;
        for (int i = 0; i < chunksize; i++) {
            if (!(infile >> chunk[i])) {
                incomplete = true;
                break;
            }
        }
        if (incomplete) {
            break; // reader done
        }

        lock.lock();
        fullc++;
        lock.unlock();
        fullcv.notify_one();
    }
}

int main(int argc, char** argv) {
    auto startTime = chrono::steady_clock::now();

    if (argc != 4) {
        std::cout << "Usage: ./simpleavg [threads] [chunk-size] numbers.txt" << endl;
        std::cout << "Note that chunk-size should be an approximate divisor of the count of numbers." << endl;
        return 1;
    }

    int threadc = stoi(argv[1]);
    chunkc = threadc * 4;
    emptyc = chunkc;
    chunksize = stoi(argv[2]);

    infile.open(argv[3]);

    totalchunks = 0;
    totalsum = 0;
    
    chunks = new float*[chunkc];

    for (int i = 0; i < chunkc; i++) {
        chunks[i] = new float[chunksize];
    }

    //jthread readerThread(reader);

    jthread* workerThreads = new jthread[threadc];

    for (int i = 0; i < threadc; i++) {
        workerThreads[i] = jthread(worker);
    }

    reader(stop_token());

    //readerThread.join();

    // make sure the workers have time to pick up the last chunks
    usleep(100); // 0.1ms

    for (int i = 0; i < threadc; i++) {
        workerThreads[i].request_stop();
    }

    auto endTime = chrono::steady_clock::now();
    auto duration = chrono::duration<double>(endTime - startTime).count();
    std::cout << "Computed average: " << (totalsum / totalchunks) << endl;
    std::cout << "Processed " << (totalchunks * chunksize) << " numbers in " << duration << " seconds." << endl;

    return 0;
}