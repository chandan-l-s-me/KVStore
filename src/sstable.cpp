#include "../include/sstable.h"

#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
static constexpr const char* TOMBSTONE = "__TOMBSTONE__";

pair<string,string> SSTable::writeTable(const string& filename,const unordered_map<string,string>& memTable)
{
    vector<pair<string,string>> data;

    for(const auto& p : memTable)
        data.push_back(p);

    sort(data.begin(),data.end());

    ofstream out(filename,ios::binary);

    for(const auto& p : data)
    {
        uint32_t keyLen =p.first.size();

        uint32_t valueLen =p.second.size();

        out.write((char*)&keyLen,sizeof(keyLen));

        out.write((char*)&valueLen,sizeof(valueLen));

        out.write(p.first.data(),keyLen);

        out.write(p.second.data(),valueLen);
    }
    string minKey = data.front().first;
    string maxKey = data.back().first;

    return {minKey,maxKey};
}

bool SSTable::get(const string& filename,const string& key,string& value)
{
    ifstream in(filename,ios::binary);

    string k;
    string v;

    while(true)
    {
        uint32_t keyLen;
        uint32_t valueLen;

        if(!in.read((char*)&keyLen,sizeof(keyLen)))break;

        if(!in.read((char*)&valueLen,sizeof(valueLen)))break;
        

        string storedKey(keyLen,'\0');

        string storedValue(valueLen,'\0');

        in.read(&storedKey[0],keyLen);

        in.read(&storedValue[0],valueLen);

        if(storedKey == key)
        {
            value = storedValue;
            return true;
        }
    }

    return false;
}


//sstable compaction

//helper function
void SSTable::loadTable(const string& filename,unordered_map<string,string>& map)
{
    ifstream in(filename,ios::binary);
        
    while(true)
    {
        uint32_t keyLen;
        uint32_t valueLen;

        if(!in.read((char*)&keyLen,sizeof(keyLen)))break;
        if(!in.read((char*)&valueLen,sizeof(valueLen)))break;

        string Key(keyLen,'\0');

        string Value(valueLen,'\0');

        in.read(&Key[0],keyLen);

        in.read(&Value[0],valueLen);

        map[Key] = Value;
        if(Value == TOMBSTONE)
        {
            map.erase(Key);
        }
    }
}

//helper function for min-max in sstable

pair<string,string>SSTable::getMinMaxKey(const string& filename){

    ifstream in(filename,ios::binary);
    string minKey;
    string maxKey;

    bool first = true;

    while(true)
    {
        uint32_t keyLen;
        uint32_t valueLen;

        if(!in.read((char*)&keyLen,sizeof(keyLen)))break;
        

        in.read((char*)&valueLen,sizeof(valueLen));

        string key(keyLen,'\0');
        string value(valueLen,'\0');

        in.read(&key[0],keyLen);
        in.read(&value[0],valueLen);

        if(first)
        {
            minKey = key;
            first = false;
        }

        maxKey = key;
    }

    return {
        minKey,
        maxKey
    };
}