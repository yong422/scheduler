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

## error message handling

```cpp
#include <unistd.h>
#include <string>
#include <iostream>
#include "ykjo/scheduler.h"

ykjo::Scheduler scheduler;
std::string error_msg = "";

// cycle 1 sec
scheduler.AddSchedule(task, 1);

scheduler.set_maxmimum_execution_time(2);
scheduler.Start();
scheduler.set_failure_callback([&error_msg](const std::string& msg) -> void {error_msg = msg;} );
// thread start
scheduler.Start();

while (!error_msg.empty()) {
  sleep(1);
}
std::cout << "error > " << error_msg << std::endl;

scheduler.Join();
scheduler.Stop();

```

## License

<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

The code is licensed under the [MIT License](http://opensource.org/licenses/MIT):

Copyright &copy; 2019 [Yongkyu Jo](https://github.com/yong422)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Execute unit tests

### dependencies

- conan (cpp package manager)
- googletest (unit test)

To compile and run the tests, you need to execute

```bash
$ mkdir -p build
$ cd  build
$ cmake ../
$ make run

=============================================================================

[==========] Running 4 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 4 tests from TestScheduler
[ RUN      ] TestScheduler.TestException
[       OK ] TestScheduler.TestException (0 ms)
[ RUN      ] TestScheduler.TestScheduler
[       OK ] TestScheduler.TestScheduler (2203 ms)
[ RUN      ] TestScheduler.TestStressScheduler
[       OK ] TestScheduler.TestStressScheduler (1003 ms)
[ RUN      ] TestScheduler.TestDelayedTask
[       OK ] TestScheduler.TestDelayedTask (2000 ms)
[----------] 4 tests from TestScheduler (5206 ms total)

[----------] Global test environment tear-down
[==========] 4 tests from 1 test case ran. (5206 ms total)
[  PASSED  ] 4 tests.
build & run
```