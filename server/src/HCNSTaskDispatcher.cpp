#include<HCNSTaskDispatcher.h>

HCNSCellTask::HCNSCellTask()
{

}

HCNSCellTask::~HCNSCellTask()
{

}

HCNSTaskDispatcher::HCNSTaskDispatcher()
{
}

HCNSTaskDispatcher::~HCNSTaskDispatcher()
{
          m_taskThread.join();
}

/*------------------------------------------------------------------------------------------------------
* producer thread add HCNSCellTask* _cellTask into the m_temproaryTaskList
* @function: void addTemproaryClient(HCNSCellTask* _cellTask)
* @param: HCNSCellTask* _cellTask
*------------------------------------------------------------------------------------------------------*/
void HCNSTaskDispatcher::addTemproaryTask(HCNSCellTask* _cellTask)
{
          std::lock_guard<std::mutex> _lckg(this->m_temproaryMutex);
          m_temproaryTaskList.push_back(_cellTask);
}

/*------------------------------------------------------------------------------------------------------
* processing celltask(consumer thread)
* @function: void taskProcessingThread()
*------------------------------------------------------------------------------------------------------*/
void HCNSTaskDispatcher::taskProcessingThread()
{
          while (true)
          {
                    /*size of temporary buffer is valid*/
                    if (this->m_temproaryTaskList.size())
                    {
                              std::lock_guard<std::mutex> _lckg(this->m_temproaryMutex);
                              for (auto ib = this->m_temproaryTaskList.begin(); ib != this->m_temproaryTaskList.end(); ++ib) {
                                        this->m_mainTaskList.push_back(*ib);
                              }
                              this->m_temproaryTaskList.clear();
                    }

                    /*currently, the size of maintasklist container is zero, continue */
                    if (!this->m_mainTaskList.size()) {

                              /*suspend this thread for 1 millisecond in order to block this thread from occupying cpu cycle*/
                              std::this_thread::sleep_for(std::chrono::milliseconds(1));
                              continue;
                    }

                    /*deal with maintask*/
                    for (auto ib = this->m_mainTaskList.begin(); ib != this->m_mainTaskList.end(); ib++)
                    {
                              (*ib)->excuteTask();
                    }
                    this->m_mainTaskList.clear();
          }
}

/*------------------------------------------------------------------------------------------------------
* start processing celltask(consumer thread)
* @function: void startCellTaskDispatch()
*------------------------------------------------------------------------------------------------------*/
void HCNSTaskDispatcher::startCellTaskDispatch()
{
          m_taskThread = std::thread(std::mem_fn(&HCNSTaskDispatcher::taskProcessingThread), this);
}

/*------------------------------------------------------------------------------------------------------
* remove container and erase all the structures
* @function: void purgeRemoveTaskList()
*------------------------------------------------------------------------------------------------------*/
void HCNSTaskDispatcher::purgeRemoveTaskList()
{

}