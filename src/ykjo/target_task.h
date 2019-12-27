#ifndef YKJO_TARGET_TASK_H
#define YKJO_TARGET_TASK_H

#include <memory>

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
  bool IsExecutable(const uint32_t& now) noexcept;

  //  @brief  Task 가 종료된 시간을 설정한다.
  //  @params  uint32_t now  Task 가 종료된 unix time
  void set_end_time(const uint32_t& now) noexcept;

  std::shared_ptr<Task> task() const { return task_; }

 private:
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

} // namespace ykjo

#endif // YKJO_TARGET_TASK_H
