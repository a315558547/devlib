#include "../include/zabbix_sender.h"
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <time.h>
#include <sstream>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

using std::string;

const char header[] = { 'Z', 'B', 'X', 'D', '\1' };
const string request = "sender data";

ZabbixSender::ZabbixSender(const string& serverHost, int serverPort)
{
    this->m_serverHost = serverHost;
    this->m_serverPort = serverPort;
}

ZabbixSender::~ZabbixSender()
{

}

int sendZabbixReport(const string& servHost, int servPort, const ptree& sendData, string& errMsg)
{
    // init socket
    boost::asio::io_service ioServ;
    boost::asio::ip::tcp::socket sock(ioServ);
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(servHost), servPort);

    boost::system::error_code ec;
    sock.connect(ep, ec);
    if (ec)
    {
        errMsg = ec.message();
        return -1;
    }

    // assenmble json string
    ptree sendDatas;
    sendDatas.push_back(std::make_pair("", sendData));

    ptree reqJson;
    reqJson.put("clock", time(NULL));
    reqJson.put("request", request);

    reqJson.add_child("data", sendDatas);

    std::ostringstream stream;
    write_json(stream, reqJson, false);

    string reqStr = stream.str();

    // assemble send buffer
    uint64_t dataLen = reqStr.size();
    uint64_t sendBuffLen = 5 + 8 + dataLen;
    char* sendBuff = new char[sendBuffLen];
    memset(sendBuff, 0, sendBuffLen);
    memcpy(sendBuff, header, 5);
    memcpy(sendBuff + 5, &dataLen, 8);
    memcpy(sendBuff + 5 + 8, reqStr.c_str(), dataLen);

    // send data
    uint64_t sendCount = 0;
    while (sendCount < sendBuffLen)
    {
        sendCount += boost::asio::write(sock, boost::asio::buffer(sendBuff + sendCount, (size_t)(sendBuffLen - sendCount)), boost::asio::transfer_all(), ec);
        if (ec)
        {
            delete sendBuff;
            errMsg = ec.message();
            return -1;
        }
    }

    delete sendBuff;

    // received data
    char recvHeader[13];
    memset(recvHeader, 0, 13);
    boost::asio::read(sock, boost::asio::buffer(recvHeader, 13), boost::asio::transfer_all(), ec);
    if (ec)
    {
        errMsg = ec.message();
        return -1;
    }

    if (memcmp(header, recvHeader, 5) != 0)
    {
        errMsg = "received bad message header!";
        return -1;
    }

    memcpy(&dataLen, recvHeader + 5, 8);
    char* recvBuff = new char(dataLen);
    memset(recvBuff, 0, dataLen);
    boost::asio::read(sock, boost::asio::buffer(recvBuff, dataLen), boost::asio::transfer_all(), ec);
    if (ec)
    {
        delete recvBuff;
        errMsg = ec.message();
        return -1;
    }

    // parse response
    ptree rspJson;
    std::stringstream rspSteam(std::string(recvBuff, (uint32_t)dataLen));
    read_json(rspSteam, rspJson);
    auto rspText = rspJson.get_child_optional("response");
    if (rspText)
    {
        std::cout << rspText->data() << std::endl;
    }

    auto rspInfo = rspJson.get_child_optional("info");
    if (rspInfo)
    {
        std::string rspStr = rspInfo->data();
        std::vector<std::string> resultArray;
        boost::split(resultArray, rspStr, boost::is_any_of(";"));

        for (std::vector<std::string>::iterator resultIter = resultArray.begin();
             resultIter != resultArray.end(); resultIter++)
        {
            std::vector<std::string> detailInfos;
            boost::split(detailInfos, *resultIter, boost::is_any_of(":"));

            if (detailInfos.size() == 2)
            {
                if (boost::trim_copy(detailInfos[0]) == "failed")
                {
                    int failCount = atoi(detailInfos[1].c_str());
                    std::cout << failCount << std::endl;
                }
            }
        }
    }

    delete recvBuff;

    return 0;
}

int ZabbixSender::send(const string& vHost, const string& key, const string& value, uint64_t clock, string& errMsg)
{
    ptree dataJson;
    dataJson.put("clock", clock);
    dataJson.put("host", vHost);
    dataJson.put("key", key);
    dataJson.put("value", value);

    return sendZabbixReport(m_serverHost, m_serverPort, dataJson, errMsg);
}

int ZabbixSender::send(const string& vHost, const string& key, int value, uint64_t clock, string& errMsg)
{
    ptree dataJson;
    dataJson.put("clock", clock);
    dataJson.put("host", vHost);
    dataJson.put("key", key);
    dataJson.put("value", value);

    return sendZabbixReport(m_serverHost, m_serverPort, dataJson, errMsg);
}
