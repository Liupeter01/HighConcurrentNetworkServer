#pragma once
#ifndef _CELLCLIENT_H_
#define _CELLCLIENT_H_
#include<ServerSocket.hpp>

class CellClient
{
public:
          typedef  std::function<void(const std::shared_ptr<_ServerSocket>&, char*, int)> CellClientTask;
public:
          CellClient() = default;
          CellClient(long long _timeout);
          virtual ~CellClient();

public:
          static SOCKET createClientSocket(
                    IN int af = AF_INET,
                    IN int type = SOCK_STREAM,
                    IN int protocol = IPPROTO_TCP
          );

public:
          void connectToServer(IN unsigned long _ipAddr, IN unsigned short _ipPort);
          void addExcuteMethod(IN CellClientTask&& _cellClientTask =
                    [](const std::shared_ptr<_ServerSocket>& _serverSocket, char* _szSendBuf, int _szBufferSize)->void {
                              _serverSocket->sendDataToServer(_szSendBuf, _szBufferSize);
                    }
          );

          void startExcuteMethod(char* _szSendBuf, int _szBufferSize);

private:
          void initClientIOMultiplexing();
          int initClientSelectModel();

          void serverMsgProcessingThread();
          bool serverDataProcessingLayer();
          void readMessageHeader(IN _PackageHeader* _header);
          void readMessageBody(IN _PackageHeader* _buffer);

private:
          /*flush send buffer and send data to server as a pulse package*/
          long long _flushSendBufferTimeout;

          /*client interface thread*/
          std::promise<bool> m_interfacePromise;
          std::shared_future<bool> m_interfaceFuture;

          /*select network model*/
          fd_set m_fdread;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };

          std::shared_ptr<_ServerSocket> _serverSocket;
          CellClientTask _func;

#if _WIN32 
          WSADATA m_wsadata;
#endif // _WINDOWS 
};

#endif 