#ifndef MBACCOUNT_PROTO_HANDLER_H
#define MBACCOUNT_PROTO_HANDLER_H

#include "service_group.h"
#include "proto_handler.h"

#include <fstream>
#include <string>
#include <map>
using std::ofstream;
using std::string;
using std::map;

class ConfigSyncProtoHandler : public SyncProtoHandler
{
public:
    ConfigSyncProtoHandler(){}
    virtual ~ConfigSyncProtoHandler(){}

	bool Init(const vector<string> &ips, int port);
    bool CGetProcessIdentity(string path, map<string, string> &config_map);
};

#endif
