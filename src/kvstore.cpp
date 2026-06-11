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
static constexpr const char* TOMBSTONE = "__TOMBSTONE__";

    

KVStore::KVStore(const string& filename){
        walFile=filename;
        rebuildkeydir();
    }

unordered_map<string,uint64_t>& KVStore::getKeyDir() {
    return keyDir;
}

bool KVStore::set(const string& key, const string& value){
        //locking mtx
        
        unique_lock<shared_mutex> lock(sm);
        //this_thread::sleep_for(chrono::seconds(2));
        ofstream out(walFile, ios::binary | ios::app);

        if (!out)
            return false;

        uint32_t keyLen = key.size();
        uint32_t valueLen = value.size();

        uint64_t offset = out.tellp();//tellp gives the position of write pointer

        //cout << "Writing " << key<< " at offset "<< offset << endl;

        out.write((char*)&keyLen, sizeof(keyLen));
        out.write((char*)&valueLen, sizeof(valueLen));

        out.write(&key[0], keyLen);//.WRITE IS A BINARTY function whcih dpes not undersatnd string so we give key.data which gives pointer poointing to start of the line there by write len characteds

        out.write(&value[0], valueLen);

        out.flush();//writrs to file from buffer

        keyDir[key] = offset; //in memory key directory

        return true;

    }

bool KVStore::get(const string& key, string& value){
        
        shared_lock<shared_mutex> lock(sm);
        //this_thread::sleep_for(chrono::seconds(2));
        if(keyDir.find(key)==keyDir.end())return false;
        uint64_t offset=keyDir[key];

        ifstream in(walFile,ios::binary);

        in.seekg(offset);
        uint32_t keylen;
        uint32_t  vallen;


        in.read((char*)&keylen,sizeof(keylen));
        in.read((char*)&vallen,sizeof(vallen));

        string storedkey(keylen,'\0');
        string storedval(vallen,'\0');
        in.read(&storedkey[0],keylen);
        in.read(&storedval[0],vallen);
        value=storedval;
        if(storedval == TOMBSTONE){
        return false;}

        return true;


    }

bool KVStore::del(const string& key){
       
        return set(key,TOMBSTONE);
    }

void KVStore::printKeyDir(){
          cout << "\nKeyDir\n";

    for (auto& p : keyDir) {
        cout << p.first << " -- " << p.second << '\n';
    }
    }
void KVStore::rebuildkeydirinternal(){
         keyDir.clear();
        ifstream in(walFile,ios::binary);
        if(!in)return;

      

        while(true){
              uint64_t  offset= in.tellg();
              uint32_t keylen;
              uint32_t vallen;
              if(!in.read((char*)&keylen,sizeof(keylen)))break;
              if(!in.read((char*)&vallen,sizeof(vallen)))break;

              string key(keylen,'\0');
              string val(vallen,'\0');

              if(!in.read(&key[0],keylen)) break;
              if(!in.read(&val[0],vallen))break;
              keyDir[key]=offset;

              //cout << "Recovered "<< key<< " at "<< offset<< endl;


        }
    }
void KVStore::rebuildkeydir(){

        unique_lock<shared_mutex> lock(sm);
        rebuildkeydirinternal();
       
    }
void KVStore::compact(){
       
        unique_lock<shared_mutex> lock(sm);
        unordered_map<string,uint64_t>newkvd;
        ofstream out("compact.log",ios::binary);
        ifstream in(walFile,ios::binary);
        for(const auto &p:keyDir){
            
            uint64_t compactoffset =out.tellp();

            uint64_t offset=p.second;
            in.seekg(offset);
              uint32_t keylen;
              uint32_t vallen;
              if(!in.read((char*)&keylen,sizeof(keylen)))break;
              if(!in.read((char*)&vallen,sizeof(vallen)))break;

              string key(keylen,'\0');
              string val(vallen,'\0');

              if(!in.read(&key[0],keylen)) break;
              if(!in.read(&val[0],vallen))break;
              if(val!=TOMBSTONE){

                        out.write((char*)&keylen, sizeof(keylen));
                        out.write((char*)&vallen, sizeof(vallen));

                        out.write(&key[0], keylen);//.WRITE IS A BINARTY function whcih dpes not undersatnd string so we give key.data which gives pointer poointing to start of the line there by write len characteds
                        out.write(&val[0], vallen);

                        out.flush();//writrs to file from buffer

                        newkvd[key] = compactoffset; //in memory key directory

              }

        }
        out.close();
        in.close();
        remove(walFile.c_str());
        rename("compact.log", walFile.c_str());
        keyDir=newkvd;
        //rebuildkeydirinternal();

    }
