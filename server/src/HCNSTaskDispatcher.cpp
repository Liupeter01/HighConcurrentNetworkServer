#include<HCNSTaskDispatcher.h>

HCNSTaskDispatcher::HCNSTaskDispatcher()
          :m_interfaceFuture(this->m_interfacePromise.get_future())
{
}

HCNSTaskDispatcher::~HCNSTaskDispatcher()
{
          m_taskThread.join();
}

/*------------------------------------------------------------------------------------------------------
* producer thread add HCNSCellTask* _cellTask into the m_temproaryTaskList
* @description: perfect forwarding a righr value reference to enhance performance!
* @function: void addTemproaryClient(CellTask && _cellTask)
* @param:[IN] CellTask && _cellTask
*------------------------------------------------------------------------------------------------------*/
void 
HCNSTaskDispatcher::addTemproaryTask(CellTask && _cellTask)
{
          std::lock_guard<std::mutex> _lckg(this->m_temproaryMutex);
          m_temproaryTaskList.push_back(
                std::forward<CellTask>(_cellTask)
          );
}

/*------------------------------------------------------------------------------------------------------
* processing celltask(consumer thread)
* @function: void taskProcessingThread()
*------------------------------------------------------------------------------------------------------*/
void 
HCNSTaskDispatcher::taskProcessingThread()
{
          while (true)
          {
                    if (this->m_interfaceFuture.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
                              if (!this->m_interfaceFuture.get()) {
                                        break;
                              }
                    }
                    /*size of temporary buffer is valid*/
                    if (this->m_temproaryTaskList.size())
                    {
                              std::lock_guard<std::mutex> _lckg(this->m_temproaryMutex);
                              for (auto ib = this->m_temproaryTaskList.begin(); ib != this->m_temproaryTaskList.end(); ++ib) {
                                        this->m_mainTaskList.push_back(std::move(*ib));
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
                    std::for_each(this->m_mainTaskList.begin(), this->m_mainTaskList.end(),
                              [](CellTask& _cellTask) {
                                        /*call std::function<void()>*/
                                        _cellTask();
                              }
                    );
                    this->m_mainTaskList.clear();
          }
}

/*------------------------------------------------------------------------------------------------------
* start processing celltask(consumer thread)
* @function: void startCellTaskDispatch()
*------------------------------------------------------------------------------------------------------*/
void 
HCNSTaskDispatcher::startCellTaskDispatch()
{
          m_taskThread = std::thread(std::mem_fn(&HCNSTaskDispatcher::taskProcessingThread), this);
}

/*------------------------------------------------------------------------------------------------------
* shutdown task dispatcher system(use promise and future to end while-loop)
* @function: void shutdownTaskDispatcher()
*------------------------------------------------------------------------------------------------------*/
void 
HCNSTaskDispatcher::shutdownTaskDispatcher()
{
          //set symphore value to end this thread
          this->m_interfacePromise.set_value(false);
}

/*------------------------------------------------------------------------------------------------------
* remove container and erase all the structures
* @function: void purgeRemoveTaskList()
*------------------------------------------------------------------------------------------------------*/
void 
HCNSTaskDispatcher::purgeRemoveTaskList()
{
			this->m_mainTaskList.clear();
			this->m_temproaryTaskList.clear();
}
