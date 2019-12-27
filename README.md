# scheduler

Modern C++ scheduling library. Tasks run asynchronously.

```cpp

// Create class for task registration

class TestTask : public ykjo::Task {
public:
  TestTask() = default;
  virtual ~TestTask() = default;
  void execute() {
    std::cout << "TestTask run" << std::endl;
  };
};

```



```cpp
ykjo::Scheduler scheduler;

std::shared_ptr<ykjo::Task> task(new TestTask);

// cycle 1 sec
scheduler.AddSchedule(task, 1);

// thread start
scheduler.Start();

// wait

int exit_signal = getch();

scheduler.Stop();
scheduler.Join();

```
