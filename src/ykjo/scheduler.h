#ifndef YKJO_SCHEDULER_H
#define YKJO_SCHEDULER_H

#include <thread>
#include <memory>
#include <list>
#include <mutex>
#include <future>
#include "task.h"
#include "target_task.h"

namespace ykjo {

class Scheduler {
public:
  Scheduler() {}
  ~Scheduler();

  //  @brief  
  void Start();
  void Create();

  //  @brief  Timer 에 Task 를 추가 한다.
  //  @params Task* task        추가하고자 하는 Task 의 pointer
  //  @params uint32_t cycle    Task 를 실행할 주기
  void AddSchedule(const std::shared_ptr<Task>& task, const uint32_t& cycle);
  void AddTask(const std::shared_ptr<Task>& task, const uint32_t& cycle);

  //  @brief  Timer 의 종료를 대기 한다.
  void Join();

  //  @brief  Timer 의 Periodic job 을 종료 시킨다.
  void Stop();

private:
  //  @brief  Timer thread fuction
  //          등록된 Task 를 이용하여 Periodic job 을 수행한다.
  void Run();

private:
  //  Scheduler thread 의 stop signal 전달을 위한 promise
  std::promise<void>                                                              exit_signal_;
  //  Scheduler thread 의 stop 처리를 위한 future class
  std::future<void>                                                               main_future_;                                                          
  //  Scheduler 의 periodic job 수행을 위한 thread
  std::shared_ptr<std::thread>                                                    thread_;
  //  Task 를 저장하기 위한 list
  std::list< std::shared_ptr<TargetTask> >                                        tasks_;
  //  비동기로 실행하는 Task 관리를 위한 list
  std::list< std::pair< std::shared_ptr<TargetTask>, std::shared_future<int> > >  running_tasks_;
  //  Tasks 접근 제어를 위한 mutex
  std::mutex                                                                      mutex_;
};

} // namespace ykjo

#endif // YKJO_SCHEDULER_H

