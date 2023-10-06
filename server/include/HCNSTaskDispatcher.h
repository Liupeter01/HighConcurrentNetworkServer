#pragma once
#ifndef _HCNSCELLTASK_H_
#define _HCNSCELLTASK_H_
#include<thread>
#include<mutex>
#include<functional>
#include<list>

class HCNSCellTask 
{
public:
		  HCNSCellTask();
		  virtual ~HCNSCellTask();

public:
		  virtual void excuteTask() = 0;
};

class HCNSTaskDispatcher
{
public:
		  HCNSTaskDispatcher();
		  virtual ~HCNSTaskDispatcher();

public:
		  void addTemproaryTask(HCNSCellTask* _cellTask);
		  void startCellTaskDispatch();

private:
		  void taskProcessingThread();
		  void purgeRemoveTaskList();

private:
		  /*temproary and permanent storage for tasks*/
		  std::mutex m_temproaryMutex;
		  std::list<HCNSCellTask*> m_temproaryTaskList;
		  std::list<HCNSCellTask*> m_mainTaskList;

		  /*start a thread for task processing*/
		  std::thread m_taskThread;
};

#endif