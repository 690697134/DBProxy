#ifndef _BASE_WAIT_REPLY_H
#define _BASE_WAIT_REPLY_H

#include <string>
#include <memory>
#include <vector>

class ClientLogicSession;
class SSDBProtocolResponse;
struct parse_tree;

class BaseWaitReply
{
public:
    typedef std::shared_ptr<BaseWaitReply>  PTR;
    typedef std::weak_ptr<BaseWaitReply>    WEAK_PTR;

    BaseWaitReply(ClientLogicSession* client);
    ClientLogicSession*  getClient();

    virtual ~BaseWaitReply();
public:
    /*  �յ�db�������ķ���ֵ */
    virtual void    onBackendReply(int64_t dbServerSocketID, const char* buffer, int len) = 0;
    /*  ������db���������������ݺ󣬵��ô˺������Ժϲ�����ֵ�����͸��ͻ���  */
    virtual void    mergeAndSend(ClientLogicSession*) = 0;

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
            responseBinary = nullptr;
            ssdbReply = nullptr;
            redisReply = nullptr;
            forceOK = false;
        }

        int64_t                 dbServerSocketID;       /*  �˵ȴ���response���ڵ�db��������id    */
        std::string*            responseBinary;         /*  ԭʼ��(δ����)response���� */
        SSDBProtocolResponse*   ssdbReply;              /*  �����õ�ssdb response*/
        parse_tree*             redisReply;
        bool                    forceOK;                /*  �Ƿ�ǿ�����óɹ�    */
    };

    std::vector<PendingResponseStatus>  mWaitResponses; /*  �ȴ��ĸ�������������ֵ��״̬  */

    ClientLogicSession*                 mClient;
    std::string*                        mErrorCode;
};

#endif