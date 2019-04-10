#ifndef _UTILS_H_20161019
#define _UTILS_H_20161019

#include <string>
#include <vector>
#include "common.h"

using std::string;
using std::vector;

namespace Common
{
	extern string local_path;
	extern string local_ip;

    string get_local_ip();

	bool get_ip_by_host(string host, vector<string> &ips);
};

#endif
