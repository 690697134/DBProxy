#ifndef _BASE_WAIT_REPLY_H
#define _BASE_WAIT_REPLY_H

#include <string>
#include <memory>
#include <vector>

class ClientLogicSession;
class SSDBProtocolResponse;

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
    void            setError();

protected:

    struct PendingResponseStatus
    {
        PendingResponseStatus()
        {
            dbServerSocketID = 0;
            reply = nullptr;
            ssdbReply = nullptr;
        }

        int64_t                 dbServerSocketID;       /*  �˵ȴ���response���ڵ�db��������id    */
        std::string*            reply;                  /*  response */
        SSDBProtocolResponse*   ssdbReply;              /*  �����õ�ssdb response*/
    };

    std::vector<PendingResponseStatus>  mWaitResponses; /*  �ȴ��ĸ�������������ֵ��״̬  */

    ClientLogicSession*                 mClient;
    bool                                mIsError;       /*todo::ʹ���ַ�����Ϊ������*/
};

#endif