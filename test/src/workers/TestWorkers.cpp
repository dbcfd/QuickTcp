#include "workers/Task.h"
#include "workers/Worker.h"
#include "workers/WorkerPool.h"

#include <chrono>

#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

using namespace quicktcp::workers;

class LongRunningTask : public Task
{
public:
    LongRunningTask(const unsigned int extraTime = 0) : mExtraTime(extraTime)
    {

    }

    virtual void performTask()
    {
        std::this_thread::sleep_for(std::chrono::seconds(5 + mExtraTime));
    }
private:
    unsigned int mExtraTime;
};

class ShortRunningTask : public Task
{
public:
    ShortRunningTask()
    {

    }

    virtual void performTask()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
};
    
TEST(WORKERS_TEST, CONSTRUCTOR_DESTRUCTOR)
{	
    Worker::GetTaskFunction gtFunc = [](size_t) -> std::shared_ptr<Task> { return std::shared_ptr<Task>(nullptr); };
    Worker::WorkCompleteFunction wcFunc = [](size_t) -> void {};
    std::shared_ptr<Task> lrt, srt;
    Worker* worker;
    WorkerPool* pool;
    ASSERT_NO_THROW(lrt = std::shared_ptr<Task>(new LongRunningTask()));
    ASSERT_NO_THROW(srt = std::shared_ptr<Task>(new ShortRunningTask()));
    ASSERT_NO_THROW(worker = new Worker(gtFunc, wcFunc, 0));
    ASSERT_NO_THROW(pool = new WorkerPool(2));

    ASSERT_NO_THROW(lrt.reset());
    ASSERT_NO_THROW(srt.reset());
    ASSERT_NO_THROW(delete worker);
    ASSERT_NO_THROW(delete pool);
}

TEST(WORKERS_TEST, TASK_METHODS)
{
    {
        ShortRunningTask task;
        task.perform();
        EXPECT_TRUE(task.hasBeenPerformed());
    }
    {
        LongRunningTask task;
        std::thread thread(std::bind(&Task::perform, &task));
        
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        task.waitForCompletion();
        EXPECT_TRUE(task.hasBeenPerformed());
        end = std::chrono::system_clock::now();
        EXPECT_LE(4, std::chrono::duration_cast<std::chrono::seconds>(end - start).count());

        thread.join();
    }
}

TEST(WORKERS_TEST, WORKER_METHODS)
{
    Worker::WorkCompleteFunction wcFunc = [](size_t) -> void {};
    {
        std::shared_ptr<Task> shortTask(new ShortRunningTask());
        std::shared_ptr<Task> longTask(new LongRunningTask());
        Worker::GetTaskFunction gtFunc = [&shortTask, &longTask](size_t) -> std::shared_ptr<Task> { 
            static int callCount = 0;
            std::shared_ptr<Task> task = shortTask;
            if(0 == callCount) task = longTask;
            ++callCount;
            return task;
        };
        Worker worker(gtFunc, wcFunc, 0);
        EXPECT_FALSE(longTask->isComplete());
        worker.shutdown();
        longTask->waitForCompletion();
        EXPECT_TRUE(longTask->isComplete());
        EXPECT_FALSE(shortTask->isComplete());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        EXPECT_FALSE(shortTask->isComplete());
    }

    {
        std::shared_ptr<Task> shortTask(new ShortRunningTask());
        std::shared_ptr<Task> longTask(new LongRunningTask());
        Worker::GetTaskFunction gtFunc = [&shortTask, &longTask](size_t) -> std::shared_ptr<Task> { 
            static int callCount = 0;
            std::shared_ptr<Task> task = shortTask;
            if(0 != callCount) task = longTask;
            ++callCount;
            return task;
        };
        Worker worker(gtFunc, wcFunc, 0);
        shortTask->waitForCompletion();
        EXPECT_TRUE(shortTask->isComplete());
        std::this_thread::sleep_for(std::chrono::seconds(1)); //give our long task time to get going
        worker.shutdown();
        EXPECT_FALSE(longTask->isComplete());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        EXPECT_TRUE(longTask->isComplete());
    }
}

TEST(WORKERS_TEST, WORKERPOOL_METHODS)
{
    WorkerPool pool(2);
    std::shared_ptr<Task> lr1(new LongRunningTask(5)), lr2(new LongRunningTask());
    std::shared_ptr<Task> sr1(new ShortRunningTask()), sr2(new ShortRunningTask());

    pool.addWork(nullptr);
    pool.addWork(lr1);
    pool.addWork(sr1);
    pool.addWork(lr2);
    pool.addWork(sr2);
    sr1->waitForCompletion();
    EXPECT_TRUE(sr1->isComplete());
    EXPECT_FALSE(sr2->isComplete());
    lr2->waitForCompletion();
    EXPECT_TRUE(lr2->isComplete());
    EXPECT_FALSE(sr2->isComplete());
    sr2->waitForCompletion();
    EXPECT_TRUE(sr2->isComplete());
    EXPECT_FALSE(lr1->isComplete());
    lr1->waitForCompletion();
}