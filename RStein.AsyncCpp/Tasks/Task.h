#pragma once
#include "../AsyncPrimitives/CancellationToken.h"
#include "TaskState.h"
#include "Detail/TaskHelpers.h"
#include <any>
#include <exception>
#include <memory>
#include <functional>


namespace RStein::AsyncCpp::Tasks
{


  template<typename TFunc> 
  class Task
  {
    
    public:

        using Func_Type = TFunc;
        using TypedTaskSharedState = Detail::TypedTaskSharedState<TFunc>;
        using Ret_Type = typename TypedTaskSharedState::Function_Ret_Type;
        
       explicit Task(TFunc func) :  _sharedTaskState{std::make_shared<TypedTaskSharedState>(std::move(func),
                                                                                                            false,
                                                                                                            CancellationToken::None())}
       {
         
       }

      Task(TFunc func, const AsyncPrimitives::CancellationToken::CancellationTokenPtr& cancellationToken) : _sharedTaskState{std::make_shared<TypedTaskSharedState>(std::move(func),
                                                                                                                                                                                    false,
                                                                                                                                                                                    cancellationToken)}
       {
         
       }

      
      Task(const Task& other) = default;
      Task(Task&& other) noexcept = default;
      Task& operator=(const Task& other) = default;
      Task& operator=(Task&& other) noexcept = default;

      ~Task() = default;
      unsigned long Id() const;
      void Start();
      bool IsCanceled() const;
      bool IsCompleted() const;
      bool IsFaulted() const;
      TaskState State() const;     
     

      void Wait() const;

      template<typename TResultCopy = Ret_Type>
      typename std::enable_if<!is_same<TResultCopy, void>::value, Ret_Type>::type GetResult()
      {
        return _sharedTaskState->GetResult();
      }


      //TODO: Add Scheduler overloads
      //TODO: Use template instead of the func
      /*
      Task ContinueWith(std::function<void(const Task& task)>&& continuation);
      template<typename TCr>
      Task ContinueWith(std::function<TCr(const Task& task)>&& continuation);*/

      std::exception_ptr Exception() const;

    protected:
      Task();

      using TaskSharedStatePtr = std::shared_ptr<TypedTaskSharedState>;
      TaskSharedStatePtr _sharedTaskState;

    private:    
    Task(TaskSharedStatePtr taskSharedState) : _sharedTaskState(taskSharedState)
    {
      
    }
    void addContinuation(Task& continuationTask) const;
    
  };

  template<typename TFunc>
  unsigned long Task<TFunc>::Id() const
  {
    return _sharedTaskState->Id();
  }
  template<typename TFunc>
  void Task<TFunc>::Start()
  {
    _sharedTaskState->RunTaskFunc();
  }

  template<typename TFunc>
  bool Task<TFunc>::IsCanceled() const
  {
    return _sharedTaskState->State() == TaskState::Canceled;
  }

  template<typename TFunc>
  bool Task<TFunc>::IsCompleted() const
  {
    auto taskState = _sharedTaskState->State();
    return taskState == TaskState::RunToCompletion ||
           taskState == TaskState::Faulted ||
            taskState == TaskState::Canceled;

  }

  template<typename TFunc>
  bool Task<TFunc>::IsFaulted() const
  {
    return _sharedTaskState->State() == TaskState::Faulted;
  }

  template<typename TFunc>
  TaskState Task<TFunc>::State() const
  {
    return _sharedTaskState->State();
  }

  template<typename TFunc>
  void Task<TFunc>::Wait() const
  {
    return _sharedTaskState->Wait();
  }

  /*template<typename TResult>
  Task<TFunc> Task<TFunc>::ContinueWith(std::function<void(const Task& task)>&& continuation)
  {
    if (!continuation)
    {
      throw std::invalid_argument("continuation");
    }

    Task continuationTask{[continuation = std::move(continuation), thisCopy=*this]{continuation(thisCopy);}};
    addContinuation(continuationTask);  
    return continuationTask;
  }*/

  template<typename TFunc>
  std::exception_ptr Task<TFunc>::Exception() const
  {
    return _sharedTaskState->Exception();
  }

  template<typename TFunc>
  Task<TFunc>::Task() : _sharedTaskState{}
  {

  }

  template<typename TFunc>
  void Task<TFunc>::addContinuation(Task& continuationTask) const
  {
    _sharedTaskState->AddContinuation([continuationTask] () mutable{continuationTask.Start();});
  }

  //template<typename TResult>
  //Task<TFunc> Task<TFunc>::Run(std::function<void()>&& action)
  //{
  //  return Run(std::move(action), CancellationToken::None());
  //}

  //template<typename TResult>
  //Task<TFunc> Task<TFunc>::Run(std::function<void()>&& action, const CancellationToken::CancellationTokenPtr& cancellationToken)
  //{
  //   auto task = Task{std::move(action), cancellationToken};
  //   task.Start();
  //   return task;
  //}

  class TaskFactory
  {
    public:
     template<typename TFunc>
     static auto Run(TFunc&& func)
      {      
        Task task{std::forward<TFunc>(func)};
        task.Start();
        return task;
      }    
  };

}
