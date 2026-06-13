#pragma once
#include<iostream>
#include <string>
#include <unordered_map>
using namespace std;


class SSTable
{
public:

    static pair<string,string> writeTable(const string& filename,const unordered_map<string,string>& memTable);

    static bool get(const string& filename,const string& key,string& value);

    static void loadTable(const string& filename,unordered_map<string,string>& map);//helper function for sstable compaction

    static pair<string,string>getMinMaxKey(const string& filename); //helper function to get min and max key


};