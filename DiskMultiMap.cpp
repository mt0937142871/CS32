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
       !m_file.write((unsigned int) 12+numBuckets, 4) ||
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
    if(m_file.isOpen())
        m_file.close();
}

bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context){
    
    if(!m_file.isOpen())
        return false;
    
    if(key.length()>120 || value.length() > 120 || context.length()>120)
        return false;

    
    unsigned long hash = str_hash(key);

    Node n;
    Node newN;
    newN.key = new char[key.length()+1];
    strcpy(newN.key, key.c_str());
    newN.value = new char[value.length()+1];
    strcpy(newN.value, value.c_str());
    newN.context = new char[context.length()+1];
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
    }
    else{
        cerr<<"Failed to read insert5."<<endl;
        return false;
    }
    return true;
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
    nT.key = new char[key.length()+1];
    strcpy(nT.key, key.c_str());
    nT.value = new char[value.length()+1];
    strcpy(nT.value, value.c_str());
    nT.context = new char[context.length()+1];
    strcpy(nT.context, context.c_str());
    int count = 0;
    unsigned long hash = str_hash(key);
    unsigned int bucketN = 12+(hash%m_bi.m_buckets)*sizeof(unsigned int);
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
                                deleteNode(offset);
                                m_file.write((unsigned int) 0, bucketN);
                                return count;
                            }
                            else{
                                m_file.write(n2.next_K, bucketN);
                                deleteNode(offset);
                            }
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
