//
//  IntelWeb.cpp
//  CS32PJ4
//
//  Created by Mute on 3/6/16.
//  Copyright (c) 2016 Mute. All rights reserved.
//

#include "IntelWeb.h"
#include <sstream>
#include <set>
#include <queue>



IntelWeb::IntelWeb()
{
    
}
IntelWeb::~IntelWeb(){
    
}

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems){
    close();
    
    m_prefix = filePrefix;
    m_MaxData = maxDataItems;
    if(m_KeyValueMap.createNew(m_prefix+"KeyValue", m_MaxData*1.5) &&
       m_ValueKeyMap.createNew(m_prefix+"ValueKey", m_MaxData*1.5))
        return true;
    close();
    return false;
    
    
}
bool IntelWeb::openExisting(const std::string& filePrefix){
    close();
    
    if(m_KeyValueMap.openExisting(filePrefix+"KeyValue") &&
       m_ValueKeyMap.openExisting(filePrefix+"ValueKey"))
        return true;
    close();
    return false;
}
void IntelWeb::close(){
    m_KeyValueMap.close();
    m_ValueKeyMap.close();
}
bool IntelWeb::ingest(const std::string& telemetryFile){
    
    ifstream inf(telemetryFile);
		  // Test for failure to open
    if ( ! inf)
    {
        cout << "Cannot open expenses file!" << endl;
        return false;
    }
    string s;
    
    while(getline(inf, s))
    {
        istringstream iss(s);
        vector<string> a{istream_iterator<string>{iss},
            istream_iterator<string>{}};
        
        m_KeyValueMap.insert(a[1], a[2], a[0]);
        m_ValueKeyMap.insert(a[2], a[1], a[0]);
        
    }
    return true;
}
unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
                   unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound,
                   std::vector<InteractionTuple>& interactions
                             ){
    //priority_queue<string> pq(string, badEntitiesFound, string);
    set<string> notTest;
    set<string> tested;
    
    for(int i = 0; i<indicators.size(); i++){
        bool visited = false;
        for(DiskMultiMap::Iterator it = m_KeyValueMap.search(indicators[i]); it.isValid(); ++it){
            visited = true;
            notTest.insert((*it).value);
        }
        for(DiskMultiMap::Iterator it = m_ValueKeyMap.search(indicators[i]); it.isValid(); ++it){
            visited = true;
            notTest.insert((*it).value);
        }
        if(visited)
            badEntitiesFound.push_back(indicators[i]);
        tested.insert(indicators[i]);
    }
    
    while(!notTest.empty()){
        int count = 0;
        string s = *(notTest.begin());
        set<string> pending;
        for(DiskMultiMap::Iterator it = m_KeyValueMap.search(s); it.isValid(); ++it){
            count++;
            if(tested.find((*it).value) == tested.end())
                pending.insert((*it).value);
        }
        for(DiskMultiMap::Iterator it = m_KeyValueMap.search(s); it.isValid(); ++it){
            count++;
            if(tested.find((*it).value) == tested.end())
                pending.insert((*it).value);
        }
        
    }
    
}
bool IntelWeb::purge(const std::string& entity){
    
}