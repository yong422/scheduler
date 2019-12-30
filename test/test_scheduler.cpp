#include <unistd.h>
#include <iostream>
#include "gtest/gtest.h"
#include "scheduler.h"

int g_test_value = 0;
int g_test_value2 = 0;

class TestTask : public ykjo::Task {
public:
  TestTask() = default;
  virtual ~TestTask() = default;
  void execute() {
    ++g_test_value;
  };
};

class TestTask2 : public ykjo::Task {
public:
  TestTask2() = default;
  virtual ~TestTask2() = default;
  void execute() {
    ++g_test_value2;
  };
};

class TestDelayedTask : public ykjo::Task {
public:
  TestDelayedTask() = default;
  virtual ~TestDelayedTask() = default;
  void execute() {
    sleep(2);
  };
};


//  Scheduler 의 Task 등록시 발생될 수 있는 오류에 대한 예외처리 테스트
TEST(TestScheduler, TestException)
{
  ykjo::Scheduler scheduler;
  try {
    scheduler.AddSchedule(std::shared_ptr<ykjo::Task>(new TestTask), 0);
    EXPECT_TRUE(false);
  } catch (std::invalid_argument& e) {
    EXPECT_TRUE(true);
    EXPECT_STREQ(e.what(), "at least 1 secs");
  }

  try {
    std::shared_ptr<ykjo::Task> empty_task;
    scheduler.AddSchedule(empty_task, 1);
    EXPECT_TRUE(false);
  } catch (std::invalid_argument& e) {
    EXPECT_TRUE(true);
    EXPECT_STREQ(e.what(), "Task must not be null");
  }

}

// 주기가 다른 각각의 태스크를 스케쥴러에 등록하여 실행 테스트.
// 수정이 필요한 테스트코드.
TEST(TestScheduler, TestScheduler)
{
  ykjo::Scheduler scheduler;

  std::shared_ptr<ykjo::Task> task(new TestTask);
  std::shared_ptr<ykjo::Task> task2(new TestTask2);

  scheduler.AddSchedule(task, 1);
  scheduler.AddSchedule(task2, 2);
  EXPECT_EQ(g_test_value, 0);
  EXPECT_EQ(g_test_value2, 0);
  try {
    scheduler.Start();
  } catch (std::exception& e) {
    std::cout << "err > " << e.what() << std::endl;
  }
  // 200 ms 
  usleep(200000);
  EXPECT_EQ(g_test_value, 1);
  EXPECT_EQ(g_test_value2, 1);
  sleep(1);
  EXPECT_EQ(g_test_value, 2);
  EXPECT_EQ(g_test_value2, 1);
  sleep(1);
  EXPECT_EQ(g_test_value, 3);
  EXPECT_EQ(g_test_value2, 2);

  scheduler.Stop();
  scheduler.Join();
}

TEST(TestScheduler, TestStressScheduler)
{
  ykjo::Scheduler scheduler;

  std::shared_ptr<ykjo::Task> task(new TestTask);
  g_test_value = 0;
  for (int  i=1; i<1001; i++) {
    // 실행주기 60초의 task 를 1000개 등록한다.
    scheduler.AddSchedule(std::shared_ptr<ykjo::Task>(new TestTask), 60);
  }
  // TestTask 는 g_test_value 를 증가 시키며 1000개의 스레드가 하나씩 증가시키며 값은 1000.

  scheduler.Start();
  // 1초간 대기후 스케쥴러를 정지 시킨다.
  sleep(1);
  scheduler.Stop();
  scheduler.Join();
  // 정상적으로 스케쥴러내의 모든 task 가 동작했는지 확인한다.
  EXPECT_EQ(1000, g_test_value);
}

TEST(TestScheduler, TestDelayedTask)
{
  ykjo::Scheduler scheduler;
  std::shared_ptr<ykjo::Task> task(new TestDelayedTask);
  scheduler.AddTask(task, 60);

  // 최대실행시간 1초를 지난 task 에 대한 오류 알림 추가.
  //  TestDelayedTask 의 실행 소요시간은 2초.
  scheduler.set_maxmimum_execution_time(1);
  scheduler.Start();
  std::string error_msg = "";
  //  내부 오류 발생시 오류메시지를 전달받기 위한 콜백함수.
  scheduler.set_failure_callback([&error_msg](const std::string& msg) -> void { error_msg = msg; } );
  sleep(2);
  EXPECT_STRNE("", error_msg.c_str());

  scheduler.Stop();
  scheduler.Join();
}