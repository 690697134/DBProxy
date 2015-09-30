#ifndef _CLIENT_H
#define _CLIENT_H

#include <memory>
#include <deque>
#include <string>
#include "NetThreadSession.h"

struct parse_tree;
class BaseWaitReply;
class SSDBProtocolResponse;

/*  ����������Ŀͻ���(�����߳�)�����Ự    */
class ClientExtNetSession : public ExtNetSession
{
public:
    ClientExtNetSession(BaseLogicSession::PTR logicSession);

    ~ClientExtNetSession();

private:
    virtual int     onMsg(const char* buffer, int len) override;

private:
    parse_tree*     mRedisParse;
    std::string     mCache;
};

/*  ����������Ŀͻ���(�߼��߳�)�߼���Ự    */
class ClientLogicSession : public BaseLogicSession, public std::enable_shared_from_this<ClientLogicSession>
{
public:
    ClientLogicSession();
    /*  ����ȴ��������Ѿ���ɵ������Լ�ȷ�ϳ�������� */
    void            processCompletedReply();
private:
    virtual void    onEnter() override;
    virtual void    onClose() override;
    virtual void    onMsg(const char* buffer, int len) override;

private:
    void            pushSSDBStrListReply(const std::vector < const char* > &strlist);
    void            pushSSDBErrorReply(const char* error);
    void            pushRedisErrorReply(const char* error);
    void            pushRedisStatusReply(const char* status);

private:
    bool            procSSDBAuth(SSDBProtocolResponse*, std::string* requestStr);
    bool            procSSDBPing(SSDBProtocolResponse*, std::string* requestStr);
    bool            procSSDBMultiSet(SSDBProtocolResponse*, std::string* requestStr);
    bool            procSSDBCommandOfMultiKeys(std::shared_ptr<BaseWaitReply>, SSDBProtocolResponse*, std::string* requestStr, const char* command);
    bool            procSSDBSingleCommand(SSDBProtocolResponse*, std::string* requestStr);

    bool            processRedisSingleCommand(parse_tree* parse, std::string* requestStr);
    bool            processRedisMset(parse_tree* parse, std::string* requestStr);
    bool            processRedisCommandOfMultiKeys(std::shared_ptr<BaseWaitReply> w, parse_tree* parse, std::string* requestStr, const char* command);

private:
    std::deque<std::shared_ptr<BaseWaitReply>>      mPendingReply;
    bool                                            mNeedAuth;
    bool                                            mIsAuth;
    string                                          mPassword;
};

#endif