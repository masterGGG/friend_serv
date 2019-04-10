#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <map>
#include <string>
#include "center_config_manager.h"
#include "common.h"
#include "net_utils.h"
#include "proto.h"
#include "singleton.h"
#include "config_proto_handler.h"


bool CenterConfigManager::GetConfigFromCenter(const char * local_path, map<string, string> &config_map)
{
	//通过域名获取IP地址
	vector<string> center_ips;
	int center_port = 19155;
	string host_name = "service-config.taomee.com";
	if(!Common::get_ip_by_host(host_name, center_ips)) {
		ERROR_LOG("Get ips failes from %s", host_name.c_str());
		return false;
	}

	//建立连接
	ConfigSyncProtoHandler *p; 
	{
		p = Singleton<ConfigSyncProtoHandler>::GetInstance();
		if (!p || !p->Init(center_ips, center_port)) {
			ERROR_LOG("Get config failed.");
			return false;
		}
	}

	// 再根据server_id，以及本服务的dir获取本服务的
    char pwd[1024] = {'\0'};
    getcwd(pwd, sizeof(pwd)/sizeof(char) - 1);

    if(local_path != NULL) {
		Common::local_path = local_path;
    }else {
		Common::local_path = pwd;
    }

	if (!p->CGetProcessIdentity(Common::local_path, config_map)) {
		ERROR_LOG("Get config failed.");
		return false;
	}
    return true;
}

bool CenterConfigManager::_LoadConfig(const char *source)
{
    m_config_map.clear();
    return GetConfigFromCenter(source, m_config_map);
}

bool CenterConfigManager::ReloadConfig(const char * source)
{
    map<string, string> new_config_map;

    if (!GetConfigFromCenter(source, new_config_map)) {
        return false;
    }

    m_config_map = new_config_map;
    return true;
}

int CenterConfigManager::ConfigGetIntVal(const char *key, int defult) const
{
    map<string, string>::const_iterator it;
    it = m_config_map.find(key);
    if (it == m_config_map.end())
        return defult;

    return atoi((*it).second.c_str());
}


const char *CenterConfigManager::ConfigGetStrVal(const char *key, const char *val) const
{
    map<string, string>::const_iterator it;
    it = m_config_map.find(key);
    if (it == m_config_map.end())
        return val;

    return (*it).second.c_str();
}

void CenterConfigManager::SetConfigMap(string key, string value)
{
	m_config_map[key] = value;
	return;
}
