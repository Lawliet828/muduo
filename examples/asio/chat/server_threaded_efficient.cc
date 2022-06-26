#include <stdio.h>
#include <unistd.h>

#include <mutex>
#include <set>

#include "examples/asio/chat/codec.h"
#include "muduo/base/Logging.h"
#include "muduo/base/noncopyable.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

using namespace muduo;
using namespace muduo::net;

/**
 * 借shared_ptr实现copy on write
 * 
 * shared_ptr是引用计数智能指针，如果当前只有一个观察者，那么引用计数为1,可以用shared_ptr::unique()来判断
 * 对于write端，如果发现引用计数为1，这时可以安全地修改对象，不必担心有人在读它。
 * 对于read端，在读之前把引用计数加1，读完之后减1，这样可以保证在读的期间其引用计数大于1，可以阻止并发写。
 * 
 * 比较难的是，对于write端，如果发现引用计数大于1，该如何处理?
 * 既然要更新数据，肯定要加锁，如果这时候其他线程正在读，那么不能在原来的数据上修改，得创建一个副本，在副本上修改，修改完了再替换。
 * 如果没有用户在读，那么可以直接修改。
 */

class ChatServer : noncopyable
{
 public:
  ChatServer(EventLoop* loop,
             const InetAddress& listenAddr)
  : server_(loop, listenAddr, "ChatServer"),
    codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)),
    connections_(new ConnectionList)
  {
    server_.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
  }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void start()
  {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    std::lock_guard<std::mutex> lock(mutex_);
    if (!connections_.unique())
    {
      // new ConnectionList(*connections_)这段代码拷贝了一份ConnectionList
      connections_.reset(new ConnectionList(*connections_));
    }
    assert(connections_.unique());

    // 在复本上修改，不会影响读者，所以读者在遍历列表的时候，不需要用mutex保护
    if (conn->connected()) {
      connections_->insert(conn);
    } else {
      connections_->erase(conn);
    }
  }

  typedef std::set<TcpConnectionPtr> ConnectionList;
  typedef std::shared_ptr<ConnectionList> ConnectionListPtr;

  void onStringMessage(const TcpConnectionPtr&,
                       const string& message,
                       Timestamp)
  {
    // 引用计数加1，mutex保护的临界区大大缩短
    ConnectionListPtr connections = getConnectionList();
    // 可能大家会有疑问，不受mutex保护，写者更改了连接列表怎么办？
    // 实际上，写者是在另一个复本上修改，所以无需担心。
    for (ConnectionList::iterator it = connections->begin();
        it != connections->end();
        ++it)
    {
      codec_.send(get_pointer(*it), message);
    }
    // 当connections这个栈上的变量销毁的时候，引用计数减1
  }

  ConnectionListPtr getConnectionList()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_;
  }

  TcpServer server_;
  LengthHeaderCodec codec_;
  std::mutex mutex_;
  ConnectionListPtr connections_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 1)
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    ChatServer server(&loop, serverAddr);
    if (argc > 2)
    {
      server.setThreadNum(atoi(argv[2]));
    }
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s port [thread_num]\n", argv[0]);
  }
}

