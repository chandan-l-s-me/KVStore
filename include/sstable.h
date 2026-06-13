#pragma once
#include<iostream>
#include <string>
#include <unordered_map>
using namespace std;


class SSTable
{
public:

    static void writeTable(const string& filename,const unordered_map<string,string>& memTable);

    static bool get(const string& filename,const string& key,string& value);

    static void loadTable(const string& filename,unordered_map<string,string>& map);//helper function for sstable compaction


};