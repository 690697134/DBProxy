#include <assert.h>

#include "Client.h"
#include "RedisRequest.h"
#include "RedisParse.h"

#include "RedisWaitReply.h"

RedisSingleWaitReply::RedisSingleWaitReply(ClientLogicSession* client) : BaseWaitReply(client)
{
}

/*  TODO::�������ظ����ǵ�һ��pending reply����ô���Բ��û����ֱ�ӷ��͸��ͻ���(�����ڴ濽��)  */
void RedisSingleWaitReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
    assert(mWaitResponses.size() == 1);
    for (auto& v : mWaitResponses)
    {
        if (v.dbServerSocketID == dbServerSocketID)
        {
            v.responseBinary = new std::string(buffer, len);
            break;
        }
    }
}

void RedisSingleWaitReply::mergeAndSend(ClientLogicSession* client)
{
    if (mErrorCode != nullptr)
    {
        RedisErrorReply tmp(client, mErrorCode->c_str());
        BaseWaitReply* f = &tmp;
        f->mergeAndSend(client);
    }
    else
    {
        client->send(mWaitResponses.front().responseBinary->c_str(), mWaitResponses.front().responseBinary->size());
    }
}

RedisStatusReply::RedisStatusReply(ClientLogicSession* client, const char* status) : BaseWaitReply(client), mStatus(status)
{
}

void RedisStatusReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
}

void RedisStatusReply::mergeAndSend(ClientLogicSession* client)
{
    std::string tmp = "+" + mStatus;
    tmp += "\r\n";
    client->send(tmp.c_str(), tmp.size());
}

RedisErrorReply::RedisErrorReply(ClientLogicSession* client, const char* error) : BaseWaitReply(client), mErrorCode(error)
{
}

void RedisErrorReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
}

void RedisErrorReply::mergeAndSend(ClientLogicSession* client)
{
    std::string tmp = "-ERR " + mErrorCode;
    tmp += "\r\n";
    client->send(tmp.c_str(), tmp.size());
}

RedisWrongTypeReply::RedisWrongTypeReply(ClientLogicSession* client, const char* wrongType, const char* detail) :
    BaseWaitReply(client), mWrongType(wrongType), mWrongDetail(detail)
{
}

void RedisWrongTypeReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
}

void RedisWrongTypeReply::mergeAndSend(ClientLogicSession* client)
{
    std::string tmp = "-WRONGTYPE " + mWrongType + " " + mWrongDetail;
    tmp += "\r\n";
    client->send(tmp.c_str(), tmp.size());
}

RedisMgetWaitReply::RedisMgetWaitReply(ClientLogicSession* client) : BaseWaitReply(client)
{
}

void RedisMgetWaitReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
    for (auto& v : mWaitResponses)
    {
        if (v.dbServerSocketID == dbServerSocketID)
        {
            if (mWaitResponses.size() != 1)
            {
                v.redisReply = parse_tree_new();
                char** t = (char**)&buffer;
                parse(v.redisReply, t, (char*)(buffer + len));
            }
            else
            {
                v.responseBinary = new std::string(buffer, len);
            }

            break;
        }
    }
}

void RedisMgetWaitReply::mergeAndSend(ClientLogicSession* client)
{
    if (mErrorCode != nullptr)
    {
        RedisErrorReply tmp(client, mErrorCode->c_str());
        BaseWaitReply* f = &tmp;
        f->mergeAndSend(client);
    }
    else
    {
        if (mWaitResponses.size() == 1)
        {
            client->send(mWaitResponses.front().responseBinary->c_str(), mWaitResponses.front().responseBinary->size());
        }
        else
        {
            struct Bytes
            {
                const char* str;
                size_t len;
            };

            static vector<Bytes> vs;
            vs.clear();

            for (auto& v : mWaitResponses)
            {
                for (size_t i = 0; i < v.redisReply->reply->elements; ++i)
                {
                    vs.push_back({ v.redisReply->reply->element[i]->str, v.redisReply->reply->element[i]->len});
                }
            }

            static RedisProtocolRequest strsResponse;
            strsResponse.init();

            for (auto& v : vs)
            {
                strsResponse.appendBinary(v.str, v.len);
            }

            strsResponse.endl();
            client->send(strsResponse.getResult(), strsResponse.getResultLen());
        }
    }
}

RedisMsetWaitReply::RedisMsetWaitReply(ClientLogicSession* client) : BaseWaitReply(client)
{
}

void RedisMsetWaitReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
    for (auto& v : mWaitResponses)
    {
        if (v.dbServerSocketID == dbServerSocketID)
        {
            /*  ֻ��Ҫǿ�����óɹ�������Ҫ�����κ�reply����     */
            v.forceOK = true;
            break;
        }
    }
}

void RedisMsetWaitReply::mergeAndSend(ClientLogicSession* client)
{
    if (mErrorCode != nullptr)
    {
        RedisErrorReply tmp(client, mErrorCode->c_str());
        BaseWaitReply* f = &tmp;
        f->mergeAndSend(client);
    }
    else
    {
        /*  mset���ǳɹ�,����Ҫ�ϲ���˷�������reply   */
        const char* OK = "+OK\r\n";
        static int OK_LEN = strlen(OK);

        client->send(OK, OK_LEN);
    }
}

RedisDelWaitReply::RedisDelWaitReply(ClientLogicSession* client) : BaseWaitReply(client)
{
}

void RedisDelWaitReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
    for (auto& v : mWaitResponses)
    {
        if (v.dbServerSocketID == dbServerSocketID)
        {
            if (mWaitResponses.size() != 1)
            {
                v.redisReply = parse_tree_new();
                char* parsePos = (char*)buffer;
                parse(v.redisReply, &parsePos, (char*)(buffer + len));
            }
            else
            {
                v.responseBinary = new std::string(buffer, len);
            }

            break;
        }
    }
}

void RedisDelWaitReply::mergeAndSend(ClientLogicSession* client)
{
    if (mErrorCode != nullptr)
    {
        RedisErrorReply tmp(client, mErrorCode->c_str());
        BaseWaitReply* f = &tmp;
        f->mergeAndSend(client);
    }
    else
    {
        if (mWaitResponses.size() == 1)
        {
            client->send(mWaitResponses.front().responseBinary->c_str(), mWaitResponses.front().responseBinary->size());
        }
        else
        {
            int64_t num = 0;

            for (auto& v : mWaitResponses)
            {
                num += v.redisReply->reply->integer;
            }

            char tmp[1024];
            int len = sprintf(tmp, ":%lld\r\n", num);
            client->send(tmp, len);
        }
    }
}