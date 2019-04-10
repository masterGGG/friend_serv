#ifndef CENTER_CONFIG_MANAGER_H
#define CENTER_CONFIG_MANAGER_H

#include "config_manager.h"
#include <string>
using std::string;

// 非线程安全

class CenterConfigManager : public ConfigManager
{
public:
    virtual bool _LoadConfig(const char *source);
    virtual bool ReloadConfig(const char *source);
    virtual int ConfigGetIntVal(const char *key, int defult) const;
    virtual const char *ConfigGetStrVal(const char *key, const char *defult) const;
	virtual void SetConfigMap(string key, string value);

protected:
    bool GetConfigFromCenter(const char *source, map<string, string> &config_map);

private:
    //map<string, string> m_config_map;
};

#endif
