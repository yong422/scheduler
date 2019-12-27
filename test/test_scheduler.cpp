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
    std::cout << "TestTask run" << std::endl;
    ++g_test_value;
  };
};

class TestTask2 : public ykjo::Task {
public:
  TestTask2() = default;
  virtual ~TestTask2() = default;
  void execute() {
    std::cout << "TestTask2 run" << std::endl;
    ++g_test_value2;
  };
};

TEST(TestScheduler, TestScheduler)
{
  ykjo::Scheduler scheduler;

  std::shared_ptr<ykjo::Task> task(new TestTask);
  std::shared_ptr<ykjo::Task> task2(new TestTask2);

  scheduler.AddSchedule(task, 1);
  scheduler.AddSchedule(task2, 2);
  EXPECT_EQ(g_test_value, 0);
  EXPECT_EQ(g_test_value2, 0);
  scheduler.Start();
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

  int i = getch();
  scheduler.Stop();
  scheduler.Join();
}
