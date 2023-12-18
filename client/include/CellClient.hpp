#pragma once
#ifndef _CellClient_H_
#define _CellClient_H_
#include<INetEvent.hpp>

class CellClient
{
public:
          typedef  std::function<void(std::shared_ptr<_ServerSocket>, char*, int)> CellClientTask;
          typedef std::pair<std::shared_ptr<_ServerSocket>, CellClientTask> CellClientPackage;

          typedef std::vector<CellClientPackage> value_type;
          typedef typename value_type::iterator iterator;

public:
          CellClient() = default;
          CellClient(
                    IN std::shared_future<bool>& _future,
                    IN INetEvent* _netEvent,
                    IN long long timeout
          );
          virtual ~CellClient();

public:
          void startCellClient();
          size_t getConnectionLoad();
          void pushTemproary(IN std::shared_ptr<_ServerSocket> _pclient, IN CellClientTask&& _dosth);

private:
          int getLargestSocketValue();
          void initClientIOMultiplexing();
          int initClientSelectModel();

          void purgeCloseSocket(IN iterator _pserver);
          bool serverDataProcessingLayer(IN iterator _serverSocket);
          void serverMsgSendingThread(IN std::shared_future<bool>& _future);
          void serverMsgProcessingThread(IN std::shared_future<bool>& _future);
          void shutdownCellClient();

private:
          /*Client Pulse Timeout Setting*/
          long long _reportTimeSetting;

          /*threadpool*/
          std::thread th_serverMsgProcessing;
          std::thread th_serverMsgSending;

          /*client interface thread*/
          std::shared_future<bool> m_interfaceFuture;

          /*select network model*/
          fd_set m_fdread;
          timeval m_timeoutSetting{ 0/*0 s*/, 0 /*0 ms*/ };
          SOCKET m_largestSocket;

          /*select network model (prevent performance loss when there is no client join or leave)*/
          fd_set m_fdreadCache;                             //fd_set array backup
          bool m_isServerArrayChanged;

          /*
           * every  cell client have a temporary buffer
           * this  buffer just for temporary storage and all the items should be transfer to clientVec
           * add memory smart pointer to control memory allocation
          */
          std::mutex m_queueMutex;
          value_type m_temporaryServerPoolBuffer;

          /*
           * every cell client thread have one server connection pooly(permanent storage)
           * add memory smart pointer to control memory allocation
          */
          value_type m_ServerPoolVec;

          /*
           * cell client obj pass a leaving signal to the Connectioncontroller
           * WARNING!:this is a delegation structure, so using shared_ptr to protect its memory is unacceptable!
           */
          INetEvent* m_pNetEvent;

          _LoginData loginData{ _LoginData("client-loopback404", "1234567abc") };
};

#endif 