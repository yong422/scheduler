// Copyright (c) 2019 Yongkyu Jo
//
#ifndef YKJO_TASK_H
#define YKJO_TASK_H

namespace ykjo {

//  @class  Task
//  @brief  Task class for registration with the scheduler
class Task {
 public:
  Task() = default;
  ~Task() = default;
  //  execute by scheduler
  virtual void execute(void) = 0;
};

} // namespace ykjo

#endif // YKJO_TASK_H

