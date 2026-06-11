#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>
using namespace std;

class KVStore {
private:
    string walFile;
    unordered_map<std::string, uint64_t> keyDir;
    shared_mutex sm;

    void rebuildkeydirinternal();

public:
    KVStore(const string& filename);

    unordered_map<string,uint64_t>& getKeyDir();

    bool set(const string& key,const string& value);

    bool get(const string& key,string& value);

    bool del(const string& key);

    void printKeyDir();

    void rebuildkeydir();

    void compact();
};