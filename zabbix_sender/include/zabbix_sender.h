#ifndef ZABBIX_SENDER_H_H_
#define ZABBIX_SENDER_H_H_

#include <string>

using std::string;

class ZabbixSender
{
    ZabbixSender(const string& serverHost, int serverPort);
    ~ZabbixSender();

    /**
     *
     * @param vHost
     * @param key
     * @param value
     * @param clock unit seconds
     * @return
     */
    int send(const string& vHost, const string& key, const string& value, uint64_t clock, string& errMsg);

    /**
     *
     * @param vHost
     * @param key
     * @param value
     * @param clock unit seconds
     * @return
     */
    int send(const string& vHost, const string& key, int value, uint64_t clock, string& errMsg);

private:
    string m_serverHost;
    int m_serverPort;
};

#endif