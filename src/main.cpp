#include "../include/kvstore.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <cstdint>
#include<mutex>
#include <shared_mutex>
#include <chrono>
#include <thread>
#include<atomic>
#include<random>
using namespace std;

int main() {

    remove("data.log");

    KVStore kv("data.log");

    cout << "Preloading data...\n";

    for(int i=0;i<10000;i++) {
        kv.set(
            "key" + to_string(i),
            "value" + to_string(i)
        );
    }

    atomic<bool> stop(false);

    atomic<long long> reads(0);
    atomic<long long> writes(0);
    atomic<long long> deletes(0);
    atomic<long long> compactions(0);

    vector<thread> workers;

    // Readers
    for(int t=0;t<20;t++) {

        workers.emplace_back([&]() {

            mt19937 rng(random_device{}());

            while(!stop.load()) {

                int idx = rng() % 10000;

                string val;

                kv.get(
                    "key" + to_string(idx),
                    val
                );

                reads++;
            }
        });
    }

    // Writers
    for(int t=0;t<10;t++) {

        workers.emplace_back([&]() {

            mt19937 rng(random_device{}());

            while(!stop.load()) {

                int idx = rng() % 10000;

                kv.set(
                    "key" + to_string(idx),
                    "updated_" + to_string(idx)
                );

                writes++;
            }
        });
    }

    // Deleters
    for(int t=0;t<5;t++) {

        workers.emplace_back([&]() {

            mt19937 rng(random_device{}());

            while(!stop.load()) {

                int idx = rng() % 10000;

                kv.del(
                    "key" + to_string(idx)
                );

                deletes++;
            }
        });
    }

    thread compactor([&]() {

        while(!stop.load()) {

            this_thread::sleep_for(
                chrono::seconds(2)
            );

            kv.compact();

            compactions++;
        }
    });

    cout << "\nRunning benchmark for 5 minutes...\n";

    auto benchmarkStart =
        chrono::steady_clock::now();

    this_thread::sleep_for(
        chrono::minutes(5)
    );

    stop = true;

    for(auto& t : workers)
        t.join();

    compactor.join();

    auto benchmarkEnd =
        chrono::steady_clock::now();

    double seconds =
        chrono::duration<double>(
            benchmarkEnd - benchmarkStart
        ).count();

    cout << "\n========== RESULTS ==========\n";

    cout << "Duration       : "
         << seconds
         << " sec\n";

    cout << "Reads          : "
         << reads.load()
         << "\n";

    cout << "Writes         : "
         << writes.load()
         << "\n";

    cout << "Deletes        : "
         << deletes.load()
         << "\n";

    cout << "Compactions    : "
         << compactions.load()
         << "\n";

    cout << "\nThroughput\n";

    cout << "Read/sec       : "
         << reads.load()/seconds
         << "\n";

    cout << "Write/sec      : "
         << writes.load()/seconds
         << "\n";

    cout << "Delete/sec     : "
         << deletes.load()/seconds
         << "\n";

    cout << "Total Ops/sec  : "
         << (reads.load() +
             writes.load() +
             deletes.load())
             / seconds
         << "\n";

    cout << "\nKeyDir size    : "
         << kv.getKeyDir().size()
         << "\n";

    cout << "\nVerifying recovery...\n";

    auto recoveryStart =
        chrono::steady_clock::now();

    KVStore recovered("data.log");

    auto recoveryEnd =
        chrono::steady_clock::now();

    cout << "Recovery Time  : "
         << chrono::duration_cast<
                chrono::milliseconds
            >(recoveryEnd - recoveryStart)
                .count()
         << " ms\n";

    cout << "Recovered Keys : "
         << recovered.getKeyDir().size()
         << "\n";

    cout << "\n=============================\n";
}