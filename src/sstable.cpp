#include "../include/sstable.h"

#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
static constexpr const char* TOMBSTONE = "__TOMBSTONE__";

void SSTable::writeTable(const string& filename,const unordered_map<string,string>& memTable)
{
    vector<pair<string,string>> data;

    for(const auto& p : memTable)
        data.push_back(p);

    sort(data.begin(),data.end());

    ofstream out(filename);

    for(const auto& p : data)
    {
        out<<p.first<<" "<< p.second<< "\n";
    }
}

bool SSTable::get(const string& filename,const string& key,string& value)
{
    ifstream in(filename);

    string k;
    string v;

    while(in >> k >> v)
    {
        if(k == key)
        {   if(v==TOMBSTONE)return false;
            value = v;
            return true;
        }
    }

    return false;
}


//sstable compaction

//helper function
void SSTable::loadTable(const string& filename,unordered_map<string,string>& map)
{
    ifstream in(filename);

    string key;
    string value;

    while(in >> key >> value)
    {
        
        map[key] = value;
        if(value==TOMBSTONE)map.erase(key);
    }
}