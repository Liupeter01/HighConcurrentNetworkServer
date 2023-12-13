#pragma once
#ifndef _HCNSCELLTASK_H_
#define _HCNSCELLTASK_H_
#include<list>
#include<thread>
#include<mutex>
#include<functional>
#include<algorithm>

class HCNSTaskDispatcher
{
		  typedef std::function<void()> CellTask;
public:
		  HCNSTaskDispatcher() {}
		  virtual ~HCNSTaskDispatcher() {
					m_taskThread.join();
		  }

public:
		  //void addTemproaryTask(HCNSCellTask* _cellTask);
		  void addTemproaryTask(CellTask&& _cellTask);
		  void startCellTaskDispatch();

private:
		  void taskProcessingThread();
		  void purgeRemoveTaskList();

private:
		  /*temproary and permanent storage for tasks*/
		  std::mutex m_temproaryMutex;
		  std::list<CellTask> m_temproaryTaskList;
		  std::list<CellTask> m_mainTaskList;

		  /*start a thread for task processing*/
		  std::thread m_taskThread;
};

#endif