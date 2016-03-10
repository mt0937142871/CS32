//
//  DiskMultiMap.cpp
//  CS32PJ4
//
//  Created by Mute on 3/6/16.
//  Copyright (c) 2016 Mute. All rights reserved.
//

#include "DiskMultiMap.h"

DiskMultiMap::DiskMultiMap(){
    
}

DiskMultiMap::~DiskMultiMap(){
    if(m_file.isOpen())
        m_file.close();
}


bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets)
{
    if(m_file.isOpen())
        close();
    if(!m_file.createNew(filename))
        return false;
    
    m_bi.m_buckets = numBuckets;
    m_bi.m_next_free = sizeof(BasicInfo)+numBuckets*sizeof(unsigned int);
    m_bi.m_deleted_head = 0;
    m_bi.m_deleted_tail = 0;
    
    if(!m_file.write(numBuckets, 0) ||
       !m_file.write((unsigned int) sizeof(Node)+sizeof(unsigned int)*numBuckets, 4) ||
       !m_file.write((unsigned int) 0, 8))
        return false;
    
    
    
    for(int i = 0; i<numBuckets; i++){
        if(!m_file.write((unsigned int) 0 , i*sizeof(unsigned int) + sizeof(BasicInfo)))
            return false;
    }
    
    return true;
}

bool DiskMultiMap::openExisting(const std::string& filename){
    if(m_file.isOpen())
        close();
    if(!m_file.openExisting(filename))
        return false;
    
    if(!m_file.read(m_bi, 0))
       return false;

    return true;
}

void DiskMultiMap::close(){
    if(m_file.isOpen()){
        m_file.write(m_bi, 0);
        m_file.close();
    }
}

bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context){
    
    if(!m_file.isOpen())
        return false;
    
    if(key.length()>120 || value.length() > 120 || context.length()>120)
        return false;

    unsigned long hash = str_hash(key);

    Node n;
    Node newN;
    strcpy(newN.key, key.c_str());
    strcpy(newN.value, value.c_str());
    strcpy(newN.context, context.c_str());
    
    unsigned int bucketN = sizeof(BasicInfo)+(hash%m_bi.m_buckets)*sizeof(unsigned int);
    unsigned int offset;
    
    if(m_file.read(offset, bucketN))
    {
        while(offset != 0)
        {
            if(m_file.read(n, offset))
            {
                if(strcmp(n.key, newN.key)==0){
                    while(n.next_K != 0)
                    {
                        if(!m_file.read(n, n.next_K)){
                            cerr<<"Failed to read insert1."<<endl;
                            return false;
                        }
                    }
                    if(m_bi.m_deleted_head != 0){
                        n.next_K = m_bi.m_deleted_head;
                        if(!m_file.read(m_bi.m_deleted_head, m_bi.m_deleted_head)){
                            cerr<<"Failed to read insert2."<<endl;
                            return false;
                        }
                    }
                    else{
                        n.next_K = m_bi.m_next_free;
                        m_bi.m_next_free += sizeof(Node);
                    }
                    newN.offset = n.next_K;
                    m_file.write(n, n.offset);
                    if(!m_file.write(newN, n.next_K)){
                        cerr<<"Failed to write insert1."<<endl;
                        return false;
                    }
                    return true;
                }
                else if(n.next_H != 0)
                    offset = n.next_H;
                else{
                    if(m_bi.m_deleted_head != 0){
                        n.next_H = m_bi.m_deleted_head;
                        if(!m_file.read(m_bi.m_deleted_head, m_bi.m_deleted_head)){
                            cerr<<"Failed to read insert3."<<endl;
                            return false;
                        }
                    }
                    else{
                        n.next_H = m_bi.m_next_free;
                        m_bi.m_next_free += sizeof(Node);
                    }
                    newN.offset = n.next_H;
                    m_file.write(n, n.offset);
                    if(!m_file.write(newN, n.next_H)){
                        cerr<<"Failed to write insert2."<<endl;
                        return false;
                    }
                    return true;
                }
            }
            else {
                cerr<<"Failed to read insert4."<<endl;
                return false;
            }
        }
        newN.offset = m_bi.m_next_free;
        m_file.write(newN, newN.offset);
        m_file.write(newN.offset, bucketN);
        m_bi.m_next_free += sizeof(newN);
    }
    else{
        cerr<<"Failed to read insert5."<<endl;
        return false;
    }
    return true;
}


DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key){
    if(!m_file.isOpen())
        return Iterator();
    
    if(key.length()>120)
        return Iterator();
    
    unsigned long hash = str_hash(key);
    unsigned int bucketN = sizeof(BasicInfo)+(hash%m_bi.m_buckets)*sizeof(unsigned int);
    unsigned int offset;
    Node n;
    
    m_file.read(offset, bucketN);
    
    while(offset != 0){
        m_file.read(n, offset);
        if(strcmp(n.key, key.c_str())==0){
            return Iterator(&m_file, offset);
        }
        offset = n.next_H;
        
    }
    return Iterator();
    
}


int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context){
    if(!m_file.isOpen())
        return false;
    
    if(key.length()>120 || value.length() > 120 || context.length()>120)
        return false;
    
    Node n1;
    Node n2;
    Node n3;
    Node n4;
    Node nT;
    strcpy(nT.key, key.c_str());
    strcpy(nT.value, value.c_str());
    strcpy(nT.context, context.c_str());
    int count = 0;
    unsigned long hash = str_hash(key);
    unsigned int bucketN = sizeof(BasicInfo)+(hash%m_bi.m_buckets)*sizeof(unsigned int);
    unsigned int offset;
    
    if(m_file.read(offset, bucketN)){
        for(int i = 0; offset != 0; i++){
            if(m_file.read(n1, offset)){
                if(strcmp(n1.key, nT.key) == 0){
                    n2 = n1;
                    while(n1.next_K!=0 ){
                        if(m_file.read(n3, n1.next_K)){
                            if(strcmp(n3.value, nT.value) == 0 &&
                               strcmp(n3.context, nT.context) == 0){
                                unsigned int t = n1.next_K;
                                n1.next_K = n3.next_K;
                                m_file.write(n1, n1.offset);
                                deleteNode(t);
                                count++;
                            }
                            else
                                n1 = n3;
                        }
                        else{
                            cerr<<"Fail to read."<<endl;
                            return -1;
                        }
                    }
                    if(strcmp(n2.value, nT.value) == 0 &&
                       strcmp(n2.context, nT.context) == 0){
                        if(i == 0){
                            if(n2.next_K == 0){
                                if(!m_file.write(n2.next_H, bucketN)){
                                    cerr<<"Fail to write."<<endl;
                                    return -1;
                                }
                            }
                            else{
                                if(!m_file.read(n1, n2.next_K)){
                                    cerr<<"Fail to read."<<endl;
                                    return -1;
                                }
                                n1.next_H = n2.next_H;
                                if(!m_file.write(n2.next_K, bucketN)){
                                    cerr<<"Fail to write."<<endl;
                                    return -1;
                                }
                                if(!m_file.write(n1, n1.offset)){
                                    cerr<<"Fail to write."<<endl;
                                    return -1;
                                }
                            }
                            deleteNode(offset);
                            count++;
                            return count;
                        }
                        else{
                            if(n2.next_K == 0){
                                n4.next_H = n2.next_H;
                                if(!m_file.write(n4, n4.offset)){
                                    cerr<<"Fail to write."<<endl;
                                    return -1;
                                }

                            }
                            else{
                                if(!m_file.read(n1, n2.next_K)){
                                    cerr<<"Fail to read."<<endl;
                                    return -1;
                                }
                                n4.next_H = n1.offset;
                                n1.next_H = n2.next_H;
                            }
                            deleteNode(n2.offset);
                            count++;
                            return count;
                        }
                    }
                }
                else{
                    offset = n1.next_H;
                    n4 = n1;
                }
            }
            else{
                cerr<<"Failed to read."<<endl;
                return -1;
            }
        }
    }
    else{
        cerr<<"Failed to read."<<endl;
        return -1;
    }
    return count;
    
}

void DiskMultiMap::deleteNode(unsigned int offset){
    if(m_bi.m_deleted_head == 0)
    {
        m_bi.m_deleted_head = offset;
        m_bi.m_deleted_tail = offset;
    }
    else
    {
        if(m_file.write((unsigned int) 0, offset) &&
           m_file.write((unsigned int) offset, m_bi.m_deleted_tail)){
            m_bi.m_deleted_tail = offset;
        }
        else{
            cerr<<"Failed to write deleteNode."<<endl;
            return;
        }
    }
}

void DiskMultiMap::runthrough(){
    cout<<"offset\tkey\tvalue\tcont\tnext_H\tnext_K"<<endl;

    for(int i = 0; i<m_bi.m_buckets; i++){
        unsigned int offset;
        unsigned int index = sizeof(m_bi) + i*sizeof(unsigned int);
        m_file.read(offset, index);
        if(offset == 0)
            continue;
        Node n;
        m_file.read(n, offset);
        Node m;
        while(true){
            m=n;
            while(true){
                cout<<n.offset<<"\t"<<n.key << "\t" << n.value << "\t" << n.context << "\t" << n.next_H << "\t" << n.next_K<<endl;
                if(n.next_K!=0)
                    m_file.read(n, n.next_K);
                else
                    break;
            }
            if(m.next_H != 0)
                m_file.read(n, m.next_H);
            else break;
        }
            
        
    }
    for(int i = 0; i<16; i+=4){
        unsigned int n;
        m_file.read(n, i);
        cerr<<n<<endl;
    }
    
    
    for(int i = 0; i<m_bi.m_buckets; i++){
        
    }
    
    
}


/////////////////////////////////////////////////////////////////////////
//                                Iterator
/////////////////////////////////////////////////////////////////////////


DiskMultiMap::Iterator::Iterator()
:m_isValid(false), m_cursor(0)
{
    
}

DiskMultiMap::Iterator::Iterator(BinaryFile* bf, unsigned int cursor)
:m_cursor(cursor), m_bf(bf), m_isValid(true)
{
}


bool DiskMultiMap::Iterator::isValid() const{
    return m_isValid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++(){
    if(!m_isValid)
        return *this;
    
    Node n;
    m_bf->read(n, m_cursor);
    
    if(n.next_K == 0)
        m_isValid = false;
    else
        m_cursor = n.next_K;
    
    return *this;
}

MultiMapTuple DiskMultiMap::Iterator::operator*(){
    MultiMapTuple m;
    if(!m_isValid)
    {
        m.key = "";
        m.value = "";
        m.context = "";
        return m;
    }
    
    Node n;
    m_bf->read(n, m_cursor);
    m.key = n.key;
    m.value = n.value;
    m.context = n.context;
    return m;
}
