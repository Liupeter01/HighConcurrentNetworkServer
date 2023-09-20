#include"HelloClient.h"

constexpr int g_ClientNumber(4000);
constexpr int g_ThreadNumber(4);

int main()
{
          _LoginData loginData("client-loopback404", "1234567abc");
          HelloClient* clientPool[g_ClientNumber];
          std::thread th_send[g_ThreadNumber];

          for (int i = 0; i < g_ThreadNumber; ++i)
          {
                    th_send[i] = std::thread([&](HelloClient* clientArray[g_ClientNumber], int id) {
                              for (int i = id * (g_ClientNumber / g_ThreadNumber); i < (id + 1) * (g_ClientNumber / g_ThreadNumber); ++i) {
                                        clientPool[i] = new HelloClient;
                                        clientPool[i]->connectServer(inet_addr("127.0.0.1"), 4567);
                              }
                              while (true)
                              {
                                        for (int i = id * (g_ClientNumber / g_ThreadNumber); i < (id + 1) * (g_ClientNumber / g_ThreadNumber); ++i) {
                                                  clientPool[i]->sendDataToServer(clientPool[i]->getClientSocket(), &loginData, sizeof(loginData));
                                        }
                              }
                              }, clientPool, i);
          }
          for (int i = 0; i < g_ThreadNumber; ++i) {
                    th_send[i].join();
          }
          for (int i = 0; i < g_ClientNumber; ++i) {
                    delete clientPool[i];
          }
          return 0;
}