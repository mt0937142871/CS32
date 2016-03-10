//
//  IntelWeb.cpp
//  CS32PJ4
//
//  Created by Mute on 3/6/16.
//  Copyright (c) 2016 Mute. All rights reserved.
//

#include "IntelWeb.h"






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
    
}
unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
                   unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound,
                   std::vector<InteractionTuple>& interactions
                             ){
    
}
bool IntelWeb::purge(const std::string& entity){
    
}