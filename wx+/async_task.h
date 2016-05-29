// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0        *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved        *
// **************************************************************************

#ifndef ASYNC_TASK_H_839147839170432143214321
#define ASYNC_TASK_H_839147839170432143214321

#include <functional>
#include <zen/thread.h>
#include <zen/scope_guard.h>
#include <zen/stl_tools.h>
#include <wx/timer.h>


namespace zen
{
/*
Run a task in an async thread, but process result in GUI event loop
-------------------------------------------------------------------
1. put AsyncGuiQueue instance inside a dialog:
    AsyncGuiQueue guiQueue;

2. schedule async task and synchronous continuation:
    guiQueue.processAsync(evalAsync, evalOnGui);
*/

namespace impl
{
struct Task
{
    virtual ~Task() {}
    virtual bool resultReady   () const = 0;
    virtual void evaluateResult()       = 0;
};


template <class ResultType, class Fun>
class ConcreteTask : public Task
{
public:
    template <class Fun2>
    ConcreteTask(std::future<ResultType>&& asyncResult, Fun2&& evalOnGui) : asyncResult_(std::move(asyncResult)), evalOnGui_(std::forward<Fun2>(evalOnGui)) {}

    bool resultReady   () const override { return isReady(asyncResult_); }
    void evaluateResult()       override
    {
        evalResult(IsSameType<ResultType, void>());
    }

private:
    void evalResult(FalseType /*void result type*/) { evalOnGui_(asyncResult_.get()); }
    void evalResult(TrueType  /*void result type*/) { asyncResult_.get(); evalOnGui_(); }

    std::future<ResultType> asyncResult_;
    Fun evalOnGui_; //keep "evalOnGui" strictly separated from async thread: in particular do not copy in thread!
};


class AsyncTasks
{
public:
    AsyncTasks() {}

    template <class Fun, class Fun2>
    void add(Fun&& evalAsync, Fun2&& evalOnGui)
    {
        using ResultType = decltype(evalAsync());
        tasks.push_back(std::make_unique<ConcreteTask<ResultType, Fun2>>(zen::runAsync(std::forward<Fun>(evalAsync)), std::forward<Fun2>(evalOnGui)));
    }
    //equivalent to "evalOnGui(evalAsync())"
    //  -> evalAsync: the usual thread-safety requirements apply!
    //  -> evalOnGui: no thread-safety concerns, but must only reference variables with greater-equal lifetime than the AsyncTask instance!

    void evalResults() //call from gui thread repreatedly
    {
        if (!inRecursion) //prevent implicit recursion, e.g. if we're called from an idle event and spawn another one within the callback below
        {
            inRecursion = true;
            ZEN_ON_SCOPE_EXIT(inRecursion = false);

            std::vector<std::unique_ptr<Task>> readyTasks; //Reentrancy; access to AsyncTasks::add is not protected! => evaluate outside erase_if

            erase_if(tasks, [&](std::unique_ptr<Task>& task)
            {
                if (task->resultReady())
                {
                    readyTasks.push_back(std::move(task));
                    return true;
                }
                return false;
            });

            for (auto& task : readyTasks)
                task->evaluateResult();
        }
    }

    bool empty() const { return tasks.empty(); }

private:
    AsyncTasks           (const AsyncTasks&) = delete;
    AsyncTasks& operator=(const AsyncTasks&) = delete;

    bool inRecursion = false;
    std::vector<std::unique_ptr<Task>> tasks;
};
}


class AsyncGuiQueue : private wxEvtHandler
{
public:
    AsyncGuiQueue() { timer.Connect(wxEVT_TIMER, wxEventHandler(AsyncGuiQueue::onTimerEvent), nullptr, this); }

    template <class Fun, class Fun2>
    void processAsync(Fun&& evalAsync, Fun2&& evalOnGui)
    {
        asyncTasks.add(std::forward<Fun >(evalAsync),
                       std::forward<Fun2>(evalOnGui));
        if (!timer.IsRunning())
            timer.Start(50 /*unit: [ms]*/);
    }

private:
    void onTimerEvent(wxEvent& event) //schedule and run long-running tasks asynchronously
    {
        asyncTasks.evalResults(); //process results on GUI queue
        if (asyncTasks.empty())
            timer.Stop();
    }

    impl::AsyncTasks asyncTasks;
    wxTimer timer; //don't use wxWidgets' idle handling => repeated idle requests/consumption hogs 100% cpu!
};

}

#endif //ASYNC_TASK_H_839147839170432143214321
