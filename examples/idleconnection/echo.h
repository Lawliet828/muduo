#ifndef MUDUO_EXAMPLES_IDLECONNECTION_ECHO_H
#define MUDUO_EXAMPLES_IDLECONNECTION_ECHO_H

#include <assert.h>
#include <stdio.h>

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
//#include <muduo/base/Types.h>

#include <boost/circular_buffer.hpp>
#include <unordered_set>

using namespace muduo;
using namespace muduo::net;

// RFC 862
class EchoServer : public muduo::net::TcpServer {
 public:
  EchoServer(muduo::net::EventLoop* loop,
             const muduo::net::InetAddress& listenAddr, int idleSeconds)
      : TcpServer(loop, listenAddr, "EchoServer"),
        connectionBuckets_(idleSeconds) {
    setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
    setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    loop->runEvery(1.0, std::bind(&EchoServer::onTimer, this));
    connectionBuckets_.resize(idleSeconds);
    dumpConnectionBuckets();
  }

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn) {
    LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected()) {
      EntryPtr entry(new Entry(conn));
      connectionBuckets_.back().insert(entry);  // 插入到队尾，这时候引用计数为2
      dumpConnectionBuckets();
      WeakEntryPtr weakEntry(entry);
      conn->setContext(weakEntry);
    } else {
      assert(conn->getContext().has_value());
      WeakEntryPtr weakEntry(std::any_cast<WeakEntryPtr>(conn->getContext()));
      LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
    }
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf, muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at "
             << time.toString();
    conn->send(msg);

    assert(conn->getContext().has_value());
    WeakEntryPtr weakEntry(std::any_cast<WeakEntryPtr>(conn->getContext()));
    EntryPtr entry(weakEntry.lock());
    if (entry) {
      connectionBuckets_.back().insert(entry);
      dumpConnectionBuckets();
    }
  }

  void onTimer() {
    // 相当于将tail位置原有的Bucket删除了，然后增加了一个空的Bucket
    connectionBuckets_.push_back(Bucket());
    dumpConnectionBuckets();
  }

  void dumpConnectionBuckets() const {
    LOG_INFO << "size = " << connectionBuckets_.size();
    int idx = 0;
    for (WeakConnectionList::const_iterator bucketI =
             connectionBuckets_.begin();
         bucketI != connectionBuckets_.end(); ++bucketI, ++idx) {
      const Bucket& bucket = *bucketI;
      printf("[%d] len = %zd : ", idx, bucket.size());
      for (const auto& it : bucket) {
        bool connectionDead = it->weakConn_.expired();
        printf("%p(%ld)%s, ", get_pointer(it), it.use_count(),
               connectionDead ? " DEAD" : "");
      }
      puts("");
    }
  }

  typedef std::weak_ptr<muduo::net::TcpConnection> WeakTcpConnectionPtr;

  struct Entry : public muduo::copyable {
    explicit Entry(const WeakTcpConnectionPtr& weakConn)
        : weakConn_(weakConn) {}

    ~Entry() {
      muduo::net::TcpConnectionPtr conn = weakConn_.lock();
      if (conn) {
        conn->shutdown();
      }
    }

    WeakTcpConnectionPtr weakConn_;
  };
  typedef std::shared_ptr<Entry> EntryPtr;
  typedef std::weak_ptr<Entry> WeakEntryPtr;
  typedef std::unordered_set<EntryPtr> Bucket;
  typedef boost::circular_buffer<Bucket> WeakConnectionList;

  WeakConnectionList connectionBuckets_;
};

#endif  // MUDUO_EXAMPLES_IDLECONNECTION_ECHO_H
