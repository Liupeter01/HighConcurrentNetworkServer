#pragma once
#ifndef _HCNSCELLTASK_H_
#define _HCNSCELLTASK_H_
#include<list>
#include<thread>
#include<mutex>
#include<future>
#include<functional>
#include<algorithm>

class HCNSTaskDispatcher
{
		  typedef std::function<void()> CellTask;
public:
		  HCNSTaskDispatcher();
		  virtual ~HCNSTaskDispatcher();

public:
		  //void addTemproaryTask(HCNSCellTask* _cellTask);
		  void addTemproaryTask(CellTask&& _cellTask);
		  void startCellTaskDispatch();
		  void shutdownTaskDispatcher();

private:
		  void taskProcessingThread();
		  void purgeRemoveTaskList();

private:
		  /* interface symphoare control and thread creation*/
		  std::promise<bool> m_interfacePromise;
		  std::shared_future<bool> m_interfaceFuture;

		  /*temproary and permanent storage for tasks*/
		  std::mutex m_temproaryMutex;
		  std::list<CellTask> m_temproaryTaskList;
		  std::list<CellTask> m_mainTaskList;

		  /*start a thread for task processing*/
		  std::thread m_taskThread;
};

#endif