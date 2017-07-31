#ifndef _BASE_WAIT_REPLY_H
#define _BASE_WAIT_REPLY_H

#include <string>
#include <memory>
#include <vector>
#include <mutex>

#include "Client.h"

class SSDBProtocolResponse;
struct parse_tree;

struct BackendParseMsg
{
    typedef std::shared_ptr<BackendParseMsg> PTR;

    BackendParseMsg()
    {
    }

    std::shared_ptr<parse_tree>     redisReply;
    std::shared_ptr<std::string>    responseMemory;
};

class BaseWaitReply
{
public:
    typedef std::shared_ptr<BaseWaitReply>  PTR;
    typedef std::weak_ptr<BaseWaitReply>    WEAK_PTR;

    BaseWaitReply(const ClientSession::PTR& client);
    virtual ~BaseWaitReply();

    const ClientSession::PTR&  getClient() const;

public:
    /*  �յ�db�������ķ���ֵ */
    virtual void    onBackendReply(int64_t dbServerSocketID, const BackendParseMsg::PTR&) = 0;
    /*  ������db���������������ݺ󣬵��ô˺������Ժϲ�����ֵ�����͸��ͻ���  */
    virtual void    mergeAndSend(const ClientSession::PTR&) = 0;

public:
    /*  ����Ƿ����еȴ���db���������ѷ�������    */
    bool            isAllCompleted() const;
    /*  ���һ���ȴ���db������    */
    void            addWaitServer(int64_t serverSocketID);

    bool            hasError() const;
    /*  ���ó��ִ���    */
    void            setError(const char* errorCode);

protected:
    struct PendingResponseStatus
    {
        PendingResponseStatus()
        {
            dbServerSocketID = 0;
            forceOK = false;
        }

        int64_t                                 dbServerSocketID;       /*  �˵ȴ���response���ڵ�db��������id    */
        std::shared_ptr<std::string>            responseBinary;
        std::shared_ptr<SSDBProtocolResponse>   ssdbReply;              /*  �����õ�ssdb response*/
        std::shared_ptr<parse_tree>             redisReply;
        bool                                    forceOK;                /*  �Ƿ�ǿ�����óɹ�    */
    };

    std::vector<PendingResponseStatus>      mWaitResponses; /*  �ȴ��ĸ�������������ֵ��״̬  */

    const ClientSession::PTR                mClient;
    std::string                             mErrorCode;
};

#endif