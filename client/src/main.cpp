#include <HelloClient.h>

constexpr int g_ClientNumber(10000);
constexpr int g_ThreadNumber(4);

int main()
{
          std::promise<bool> m_interfacePromise;
          std::shared_future<bool> m_interfaceFuture(m_interfacePromise.get_future());
          _LoginData loginData[10]{ _LoginData("client-loopback404", "1234567abc") };
          HelloClient* clientPool[g_ClientNumber];
          std::thread th_send[g_ThreadNumber];
          std::thread th_stop([](std::promise<bool>& _promise) {
                    while (true)
                    {
                              char _Message[256]{ 0 };
                              std::cin.getline(_Message, 256);
                              if (!strcmp(_Message, "exit")) {
                                        std::cout << "Shutdown All Connections!" << std::endl;
                                        _promise.set_value(false);                            //set symphore value to inform other thread
                                        return;
                              }
                              else {
                                        std::cout << "Invalid Command Input!" << std::endl;
                              }
                    }
           }, std::ref(m_interfacePromise));


          for (int i = 0; i < g_ThreadNumber; ++i)
          {
                    th_send[i] = std::thread([&](HelloClient* clientArray[g_ClientNumber], std::shared_future<bool>& _future, int id) {
                              for (int i = id * (g_ClientNumber / g_ThreadNumber); i < (id + 1) * (g_ClientNumber / g_ThreadNumber); ++i) 
                              {
                                        clientPool[i] = new HelloClient;
                                        clientPool[i]->connectServer(inet_addr("127.0.0.1"), 4567);
                              }
                              while (true)
                              {
                                        if (_future.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
                                                  if (!_future.get()) {
                                                            break;
                                                  }
                                        }
                                        for (int i = id * (g_ClientNumber / g_ThreadNumber); i < (id + 1) * (g_ClientNumber / g_ThreadNumber); ++i) {
                                                  clientPool[i]->sendDataToServer(clientPool[i]->getClientSocket(), loginData, sizeof(loginData));
                                        }
                              }

                    }, clientPool, std::ref(m_interfaceFuture), i);
          }
          th_stop.join();
          for (int i = 0; i < g_ThreadNumber; ++i) {
                    th_send[i].join();
          }
          for (int i = 0; i < g_ClientNumber; ++i) {
                    delete clientPool[i];
          }
          return 0;
}