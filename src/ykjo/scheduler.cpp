// Copyright (c) 2019 Yongkyu Jo
//
#include <unistd.h>
#include <iostream>
#include <typeinfo>
#include <cstdint>
#include <mutex>
#include <ctime>
#include <chrono>
#include <thread>
#include "scheduler.h"

//  TargetTask

namespace ykjo {

//  @class  TargetTask
//  @brief  Timer 클래스에서 Task 를 관리하기 위한 클래스
//          Timer 의 schedule 을 통해 등록한 Task 와 cycle 을 저장한다.
//          Timer 에서 등록된 Task 의 periodic job 을 수행하기 위하여 사용한다.
class TargetTask {
 public:
  TargetTask(const std::shared_ptr<Task>& task, uint32_t cycle=1):task_(task), cycle_(cycle) {}
  ~TargetTask() = default;

  //  @brief  Task 가 실행주기가 되었는지 확인한다.
  //  @params  uint32_t now 현재시간(unix time)
  //  @return 실행주기가 되었을 경우 true
  //          실행주기가 되지 않은 경우 false 
  bool IsExecutable(const uint32_t& now) noexcept {
    std::unique_lock<std::mutex> lock(mutex_, std::adopt_lock);
    bool result = false;
    if (now - start_time_ >= cycle_) {
      //  Task 의 종료시간이 이전 실행시간보다 크거나 같을 경우 실행 완료 처리가 된 Task.
      if (end_time_ >= start_time_) {
        start_time_ = now;
        result = true;
      }
      //  else
      //  종료시간이 실행시간보다 전일 경우
      //  종료처리 되지 않은 Task 이므로 다음 주기의 Task 를 실행하지 않는다.
    }
    return result;
  }
  //  @brief  Task 가 종료된 시간을 설정한다.
  //  @params  uint32_t now  Task 가 종료된 unix time
  void set_end_time(const uint32_t& now) noexcept {
    std::unique_lock<std::mutex> lock(mutex_, std::adopt_lock);
    end_time_ = now;
  }

  void set_flag_generated_maxtime(const bool& value) noexcept { flag_generated_maxtime_ = value; }
  bool flag_generated_maxtime() noexcept { return flag_generated_maxtime_; }

  //  getter

  //  @brief  해당 Task 가 실행중일 경우 실행
  uint32_t GetRunningTime(const uint32_t& current_time = static_cast<uint32_t>(std::time(nullptr))) {
    if (!start_time_) return 0UL;
    else              return current_time - start_time_;
  }

  std::shared_ptr<Task> task() const { return task_; }

 private:
  bool flag_generated_maxtime_ = false;
  //  Task to be run by scheduler
  std::shared_ptr<Task> task_;
  //  task executation cycle
  //  최소 기본설정 cycle = 1 secs
  uint32_t        cycle_      = 1;
  //  작업의 마지막 시작 시간
  uint32_t        start_time_ = 0;
  //  작업의 마지막 종료 시간
  uint32_t        end_time_   = 0;
  std::mutex      mutex_;
};

//  Scheduler

Scheduler::Scheduler()
{
  failure_callback_ = [&](const std::string& message) {};
  tasks_.clear();
  running_tasks_.clear();
}

Scheduler::~Scheduler()
{
  Stop();
  Join();
}

void Scheduler::Create() throw()
{
  Start();
}

void Scheduler::Start() throw()
{
  main_future_ = exit_signal_.get_future();
  thread_ = std::shared_ptr<std::thread>(new std::thread([&](){Run();}));
  if (!thread_) {
    throw std::bad_alloc();
  }
}

void Scheduler::AddTask(const std::shared_ptr<Task>& task, const uint32_t& cycle) noexcept(false)
{
  AddSchedule(task, cycle);
}

void Scheduler::AddSchedule(const std::shared_ptr<Task>& task, const uint32_t& cycle) noexcept(false)
{
  std::unique_lock<std::mutex> lock(mutex_, std::adopt_lock);
  if (!task) {
    throw std::invalid_argument("Task must not be null");
  }
  if (!cycle) {
    throw std::invalid_argument("at least 1 secs");
  }
  tasks_.push_back(std::shared_ptr<TargetTask>(new TargetTask(task, cycle)));
}

void Scheduler::Run()
{
  while (true) {
    uint32_t now = std::time(nullptr);
    std::unique_lock<std::mutex> lock(mutex_, std::adopt_lock);
    for (auto it = tasks_.begin(); it != tasks_.end();) {
      //  등록된 Task 를 실행할지의 여부를 확인 후 비동기로 Task 를 실행 한다.
      if ((*it)->IsExecutable(now)) {
        std::shared_future<int> fu(std::async(std::launch::async, [](std::shared_ptr<TargetTask> target_task){
          target_task->task()->execute();
          return 1;
        }, *it));
        //  비동기 처리한 Task 의 실행결과를 받기위해 추가
        running_tasks_.push_back(std::make_pair(*it, fu));
        //  Task 의 실행 대기를 위한 list 에서 삭제 처리
        tasks_.erase(it++);
      } else {
        ++it;
      }
    }
    mutex_.unlock();
    for (auto it = running_tasks_.begin(); it != running_tasks_.end();) {
      //  비동기로 실행중인 Task 에 대하여 완료 여부 확인
      if (it->second.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
        //  Task 가 완료된 경우 종료 시간을 설정한 후 Task 의 대기열에 추가
        it->first->set_end_time(std::time(nullptr));
        it->first->set_flag_generated_maxtime(false);
        tasks_.push_back(it->first);
        //  실행중인 Task list 에서 삭제 처리
        running_tasks_.erase(it++);
      } else {
        //  실행중인 Task 의 실행 소요시간이 최대 설정시간보다 클 경우
        //  등록된 callback function 에 오류메시지를 전달한다.
        if (maxmimum_execution_time() && 
            !(it->first->flag_generated_maxtime()) && 
            it->first->GetRunningTime() >= maxmimum_execution_time()) {
          failure_callback_("execution time is greater than setted maximum execution time [" + std::string(typeid(*(it->first->task())).name()) + "]");
          // 반복 발생을 제외하기 위한 예외처리.
          it->first->set_flag_generated_maxtime(true);
        }
        ++it;
      }
    }
    if (main_future_.wait_for(std::chrono::microseconds(10)) == std::future_status::ready) break;
  }
}

void Scheduler::Stop() noexcept(false)
{
  try {
    exit_signal_.set_value();
  } catch (std::future_error& err) {
    //  Scheduler 가 이미 종료된 상태를 제외한 exception 을 throw 한다.
    if (err.code() != std::future_errc::promise_already_satisfied) {
      throw std::future_error(err);
    }
    //  else
    //  Scheduler thread 가 이미 종료된 상태이므로 정상 종료 처리.
  }
}

void Scheduler::Join() noexcept
{
  if (thread_ && thread_->joinable()) {
    //  Scheduler thread 가 동작 중인 경우에만 join
    thread_->join();
  }
  //  else
  //  이미 종료처리된 thread
}

} // namespace ykjo

