//
//  IntelWeb.h
//  CS32PJ4
//
//  Created by Mute on 3/6/16.
//  Copyright (c) 2016 Mute. All rights reserved.
//

#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "provided-DiskMultiMap.h"
#include <string>
#include <vector>
using namespace std;
class IntelWeb
{
public:
    IntelWeb();
    ~IntelWeb();
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close();
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& interactions
                       );
    bool purge(const std::string& entity);
    
private:
    DiskMultiMap m_KeyValueMap;
    DiskMultiMap m_ValueKeyMap;
    unsigned int m_MaxData;
    string m_prefix;
    // Your private member declarations will go here
};

#endif // INTELWEB_H_
