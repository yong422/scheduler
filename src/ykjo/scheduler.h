
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// Copyright (c) 2019 Yongkyu Jo
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef YKJO_SCHEDULER_H
#define YKJO_SCHEDULER_H

#include <thread>
#include <memory>
#include <list>
#include <mutex>
#include <future>
#include <functional>
#include "task.h"

namespace ykjo {

class TargetTask;

//  @class  ykjo::Scheduler
//  @brief  
class Scheduler {
public:
  Scheduler();
  ~Scheduler();

  //  thread function

  //  @brief  Scheduler 를 실행한다.
  //  @throw  내부 scheduler thread 를 실행한다.
  void Start() throw();
  void Create() throw();

  //  @brief  Scheduler 가 종료될때까지 대기 한다.
  void Join() noexcept;
  //  @brief  Timer 의 Periodic job 을 종료 시킨다.
  void Stop() noexcept(false);

  //  setter

  //  @brief  실행되는 전체 Task 의 감시 주기를 설정 한다. (milli seconds)
  //          10 milli seconds 가 기본값이며 그 이하는 설정 불가.
  //          Scheduler 에 등록된 Tasks 의 주기가 짧은 경우 실시간 체크를 위해서는 짧은 시간을 설정. (리소스 사용량이 올라감)
  //          Tasks 에 설정된 실행 주기가 길 경우 해당 값을 500ms 이상으로 설정하여 리소스 낭비를 제거.
  void set_scheduling_cycle(const uint32_t& scheduling_cycle) noexcept {
    if (kMimimumSchedulingCycle <= scheduling_cycle) {
      scheduling_cycle_ = scheduling_cycle;
    }
  }

  //  @brief  실행되는 개별 Task 의 감시 타임아웃을 설정 한다. (milli seconds)
  //          1 milli seconds 가 기본값이며 그 이하는 설정 불가.
  void set_wait_time_for_async_task(const uint32_t& wait_time_for_async_task) noexcept {
    if (kMimimumWaitTimeForAsyncTask <= wait_time_for_async_task) {
      wait_time_for_async_task_ = wait_time_for_async_task;
    }
  }

  //  @brief  실행되는 Task 의 최대 실행시간 검증을 위한 시간값 설정(secs)
  //          0 이 기본값이며, 0 으로 설정 된 경우 실행시간은 제한이 없다.
  void set_maxmimum_execution_time(const uint32_t& maxmimum_execution_time) noexcept { 
    maxmimum_execution_time_ = maxmimum_execution_time; 
  }

  //  @brief  Scheduler 내부에서 오류 메시지가 발생될 경우 메시지를 처리 하기 위한 callback 등록 함수
  //          등록하는 callback 함수의 형태는 void(std::string&) => [&](const std::string&) -> void {}
  void set_failure_callback(std::function< void(const std::string&) > callback_function) noexcept { 
    failure_callback_ = callback_function; 
  }

  //  @brief  Timer 에 Task 를 추가 한다.
  //  @params Task* task        추가하고자 하는 Task 의 pointer
  //  @params uint32_t cycle    Task 를 실행할 주기
  void AddSchedule(const std::shared_ptr<Task>& task, const uint32_t& cycle) noexcept(false);
  void AddTask(const std::shared_ptr<Task>& task, const uint32_t& cycle) noexcept(false);

  //  getter

  //  @biref  Scheduler 에 등록된 Task 의 정상적인 실행시간의 최대값.
  //          해당 값이 설정된 경우, 해당 값을 기준을 초과하는 경우 오류메시지를 발생 시킨다.
  uint32_t maxmimum_execution_time()  const { return maxmimum_execution_time_; }

  uint32_t scheduling_cycle()         const { return scheduling_cycle_; }
  uint32_t wait_time_for_async_task() const { return wait_time_for_async_task_; }

  uint32_t GetAllTasksCount() const;
  uint32_t GetRunningTasksCount() const;
  uint32_t GetWaittingTasksCount() const;

private:
  //  @brief  Timer thread fuction
  //          등록된 Task 를 이용하여 Periodic job 을 수행한다.
  void Run();

public:
  static const uint32_t                                                           kMimimumSchedulingCycle       = 10;
  static const uint32_t                                                           kMimimumWaitTimeForAsyncTask  = 1;

private:
  //  Scheduler 내부 오류 발생시 오류 메시지 처리를 위한 callback function
  std::function< void(const std::string&) >                                       failure_callback_;
  // 등록된 Task 의 최대 실행시간을 확인한다.
  uint32_t                                                                        maxmimum_execution_time_ = 0;
  // 등록 및 실행중인 Task 의 감시 주기이며 해당 시간을 기준으로
  // 대기중인 Task 의 실행 가능 여부, 비동기 실행중인 Task 의 종료 여부를 확인 하는 시간 값 (milli seconds)
  uint32_t                                                                        scheduling_cycle_ = kMimimumSchedulingCycle;
  // 비동기로 동작되는 Task 의 종료여부를 확인하기 위해 대기하는 시간 값 (milli seconds)
  uint32_t                                                                        wait_time_for_async_task_ = kMimimumWaitTimeForAsyncTask;
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

