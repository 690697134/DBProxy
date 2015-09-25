#include <assert.h>

#include "Client.h"
#include "RedisRequest.h"
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
            v.reply = new std::string(buffer, len);
            break;
        }
    }
}

void RedisSingleWaitReply::mergeAndSend(ClientLogicSession* client)
{
    if (!mIsError)
    {
        client->send(mWaitResponses.front().reply->c_str(), mWaitResponses.front().reply->size());
    }
    else
    {
        /*  TODO::��������response����,���ж��ʹ�ã������ι���   */
        RedisErrorReply tmp(client, "error");   /*todo::ʹ�ô����빹��*/
        BaseWaitReply* f = &tmp;
        f->mergeAndSend(client);
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

RedisErrorReply::RedisErrorReply(ClientLogicSession* client, const char* error) : BaseWaitReply(client), mError(error)
{
}

void RedisErrorReply::onBackendReply(int64_t dbServerSocketID, const char* buffer, int len)
{
}

void RedisErrorReply::mergeAndSend(ClientLogicSession* client)
{
    std::string tmp = "-ERR " + mError;
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