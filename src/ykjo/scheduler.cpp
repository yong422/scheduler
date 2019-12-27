#include <unistd.h>
#include <iostream>
#include <typeinfo>
#include <ctime>
#include <chrono>
#include <thread>
#include "scheduler.h"

//  TargetTask

namespace ykjo {

bool TargetTask::IsExecutable(const uint32_t& now) noexcept
{
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

void TargetTask::set_end_time(const uint32_t& now) noexcept
{
  std::unique_lock<std::mutex> lock(mutex_, std::adopt_lock);
  end_time_ = now;
}

//  Scheduler

Scheduler::~Scheduler()
{
  Stop();
  Join();
}

void Scheduler::Create()
{
  Start();
}

void Scheduler::Start()
{
  main_future_ = exit_signal_.get_future();
  thread_ = std::shared_ptr<std::thread>(new std::thread([&](){Run();}));
}

void Scheduler::AddTask(const std::shared_ptr<Task>& task, const uint32_t& cycle)
{
  AddSchedule(task, cycle);
}

void Scheduler::AddSchedule(const std::shared_ptr<Task>& task, const uint32_t& cycle)
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
      if (it->second.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
        //  Task 가 완료된 경우 종료 시간을 설정한 후 Task 의 대기열에 추가
        it->first->set_end_time(std::time(nullptr));
        tasks_.push_back(it->first);
        //  실행중인 Task list 에서 삭제 처리
        running_tasks_.erase(it++);
      } else {
        ++it;
      }
    }
    if (main_future_.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready) break;
  }
}

void Scheduler::Stop()
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

void Scheduler::Join()
{
  if (thread_->joinable()) {
    //  Scheduler thread 가 동작 중인 경우에만 join
    thread_->join();
  }
  //  else
  //  이미 종료처리된 thread
}

} // namespace ykjo

