#ifndef _BACKEND_CLIENT_H
#define _BACKEND_CLIENT_H

#include <memory>
#include <queue>
#include <vector>
#include <string>
#include "NetThreadSession.h"

class BaseWaitReply;
struct parse_tree;

/*  ����db��������(�����߳�-�����)�Ự  */
class BackendExtNetSession : public ExtNetSession
{
public:
    BackendExtNetSession(BaseLogicSession::PTR logicSession);
    ~BackendExtNetSession();

private:
    virtual int     onMsg(const char* buffer, int len) override;

private:
    parse_tree*     mRedisParse;
    std::string     mCache;
};

class ClientLogicSession;

/*  ����db��������(�߼��߳�-�߼���)�Ự    */
class BackendLogicSession : public BaseLogicSession
{
public:
    void            pushPendingWaitReply(std::weak_ptr<BaseWaitReply>);
    void            setID(int id);
    int             getID() const;

private:
    virtual void    onEnter() override;
    virtual void    onClose() override;
    virtual void    onMsg(const char* buffer, int len) override;
    
private:
    std::queue<std::weak_ptr<BaseWaitReply>>    mPendingWaitReply;
    int                                         mID;
};

extern std::vector<BackendLogicSession*>    gBackendClients;

BackendLogicSession*    findBackendByID(int id);
#endif