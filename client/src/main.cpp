#include"HelloClient.h"

constexpr unsigned int g_ClientNumber(1000);
constexpr unsigned int g_ThreadNumber(4);

int main() 
{
          HelloClient* clientPool[g_ClientNumber];
          std::thread th_send[g_ThreadNumber];
          for (int i = 0; i < g_ThreadNumber; ++i) 
          {
                    th_send[i] = std::thread([&](HelloClient *clientArray[g_ClientNumber], int id) {
                              std::string clientName = "client-loopback404" + id;
                              std::string clientPass = "1234567abc" + id;
                              _LoginData loginData(clientName, clientPass);
                              for (int i = id * (g_ClientNumber / g_ThreadNumber); i < (id+1)*(g_ClientNumber / g_ThreadNumber); ++i) {
                                        clientPool[i] = new HelloClient;
                                        clientPool[i]->connectServer(inet_addr("127.0.0.1"), 4567);
                              }
                              while (true){
                                        for (int i = 0; i < g_ClientNumber; ++i) {
                                                  clientPool[i]->sendDataToServer(clientPool[i]->getClientSocket(), &loginData, sizeof(loginData));
                                        }
                              }
                              for (int i = 0; i < g_ClientNumber; ++i) {
                                        delete clientPool[i];
                              }
                    }, clientPool,i);
          }
          for (int i = 0; i < g_ThreadNumber; ++i) {
                    th_send[i].join();
          }
          return 0;
}