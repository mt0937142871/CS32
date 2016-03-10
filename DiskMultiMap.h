//
//  DiskMultiMap.h
//  CS32PJ4
//
//  Created by Mute on 3/6/16.
//  Copyright (c) 2016 Mute. All rights reserved.
//

#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"
using namespace std;


class DiskMultiMap
{
public:
    
    class Iterator
    {
    public:
        Iterator();
        Iterator(BinaryFile* bf, unsigned int cursor);
        // You may add additional constructors
        bool isValid() const;
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
        bool m_isValid;
        unsigned int m_cursor;
        BinaryFile* m_bf;

        struct Node{
            char key[121];
            char value[121];
            char context[121];
            unsigned int next_H=0;  //offset of next with same hash
            //value, different key.
            unsigned int next_K=0;  //offset of next with same key
            unsigned int offset=0;
        };
        // Your private member declarations will go here
    };
    
    DiskMultiMap();
    ~DiskMultiMap();
    bool createNew(const std::string& filename, unsigned int numBuckets);
    bool openExisting(const std::string& filename);
    void close();
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    
    void runthrough();
    
    
private:
    BinaryFile m_file;
    
    struct BasicInfo{
        unsigned int m_buckets;
        unsigned int m_next_free;
        unsigned int m_deleted_head;
        unsigned int m_deleted_tail;
    };
    
    BasicInfo m_bi;
    hash<string> str_hash;

    struct Node{
        char key[121];
        char value[121];
        char context[121];
        unsigned int next_H=0;  //offset of next with same hash
                                //value, different key.
        unsigned int next_K=0;  //offset of next with same key
        unsigned int offset=0;
    };
    
    void deleteNode(unsigned int offset);
    // Your private member declarations will go here
};

#endif // DISKMULTIMAP_H_