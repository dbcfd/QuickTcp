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
    ASSERT_NO_THROW(LongRunningTask());
    ASSERT_NO_THROW(ShortRunningTask());
    ASSERT_NO_THROW(Worker());
    ASSERT_NO_THROW(WorkerPool(2));
}

TEST(WORKERS_TEST, TASK_METHODS)
{
    ShortRunningTask task;
    EXPECT_FALSE(task.completedSuccessfully());
    task.perform();
    EXPECT_TRUE(task.completedSuccessfully());
    task.reset();
    EXPECT_FALSE(task.completedSuccessfully());
}

TEST(WORKERS_TEST, WORKER_METHODS)
{
    {
        Worker worker;
        LongRunningTask task;
        EXPECT_FALSE(task.completedSuccessfully());
        worker.provideWork(&task, [](Worker*) -> void {} );
        EXPECT_FALSE(task.completedSuccessfully());
        task.waitForCompletion();
        EXPECT_TRUE(task.completedSuccessfully());
        worker.shutdown();
        task.reset();
        worker.provideWork(&task, [](Worker*) -> void {} );
        EXPECT_FALSE(task.completedSuccessfully());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        EXPECT_FALSE(task.completedSuccessfully());
    }

    {
        Worker worker;
        ShortRunningTask task;
        EXPECT_FALSE(task.completedSuccessfully());
        worker.provideWork(&task, [](Worker*) -> void {} );
        task.waitForCompletion();
        EXPECT_TRUE(task.completedSuccessfully());
        task.reset();
        worker.provideWork(&task, [](Worker*) -> void {} );
        worker.shutdown();
        task.waitForCompletion();
        EXPECT_TRUE(task.completedSuccessfully());
    }
}

TEST(WORKERS_TEST, WORKERPOOL_METHODS)
{
    WorkerPool pool(2);
    LongRunningTask lr1(2), lr2;
    ShortRunningTask sr1, sr2;

    pool.addWork(nullptr);
    pool.addWork(&lr1);
    pool.addWork(&sr1);
    pool.addWork(&lr2);
    pool.addWork(&sr2);
    sr1.waitForCompletion();
    EXPECT_TRUE(sr1.completedSuccessfully());
    EXPECT_FALSE(sr2.completedSuccessfully());
    lr2.waitForCompletion();
    EXPECT_TRUE(lr2.completedSuccessfully());
    EXPECT_FALSE(sr2.completedSuccessfully());
    sr2.waitForCompletion();
    EXPECT_TRUE(sr2.completedSuccessfully());
    EXPECT_FALSE(lr1.completedSuccessfully());
    lr1.waitForCompletion();
}