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
#include <algorithm>


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
    set<string> notTest;
    set<string> tested;
    set<InteractionTuple, classcomp> tuples;
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
        if(visited){
            badEntitiesFound.push_back(indicators[i]);
            notTest.insert(indicators[i]);
        }
        tested.insert(indicators[i]);
    }
    
    while(!notTest.empty()){
        int prevalence = 0;
        string s = *(notTest.begin());
        set<string> pending;
        set<InteractionTuple, classcomp > pendingT;
        for(DiskMultiMap::Iterator it = m_KeyValueMap.search(s); it.isValid(); ++it){
            prevalence++;
            if(tested.find((*it).value) == tested.end()){
                pending.insert((*it).value);
                pendingT.insert(InteractionTuple((*it).key, (*it).value, (*it).context));
            }
        }
        for(DiskMultiMap::Iterator it = m_ValueKeyMap.search(s); it.isValid(); ++it){
            prevalence++;
            if(tested.find((*it).value) == tested.end()){
                pending.insert((*it).value);
                pendingT.insert(InteractionTuple((*it).value, (*it).key, (*it).context));
            }
        }
        if(prevalence < minPrevalenceToBeGood){
            notTest.insert(pending.begin(), pending.end());
            tuples.insert(pendingT.begin(), pendingT.end());
            badEntitiesFound.push_back(s);
        }
        notTest.erase(s);
        tested.insert(s);
    }

    copy(tuples.begin(), tuples.end(), std::back_inserter(interactions));
    sort(badEntitiesFound.begin(), badEntitiesFound.end());
    sort(interactions.begin(), interactions.end(), InteractionTupleCmp);
    return (unsigned int)badEntitiesFound.size();
}
bool IntelWeb::purge(const std::string& entity){
    DiskMultiMap::Iterator it = m_KeyValueMap.search(entity);
    if(!it.isValid())
        return false;
    
    for(; it.isValid(); ++it){
        MultiMapTuple mt = *it;
        DiskMultiMap::Iterator it2 = m_ValueKeyMap.search(mt.value);
        for(; it2.isValid();){
            MultiMapTuple mt2 = *it2;
            if(mt2.value == mt.key){
                if(m_KeyValueMap.erase(mt2.value, mt2.key, mt2.context)&&
                   m_ValueKeyMap.erase(mt2.key, mt2.value, mt2.context)){
                    it2 = m_ValueKeyMap.search(mt.value);
                    continue;
                }
                else
                    return false;
            }
            ++it2;
        }
    }
    it = m_KeyValueMap.search(entity);
    while(it.isValid()){
        string key = (*it).key;
        string value = (*it).value;
        string context = (*it).context;
        if(!m_KeyValueMap.erase(key,value,context))
            return false;
        it = m_KeyValueMap.search(entity);
    }
        
    return true;
}