#include "coroutines.h"
namespace xiangrpc {
	Coroutine::Coroutine(std::function<void()> fun) :
		stack_size_(0),
		stack_cap_(0),
		stack_(nullptr),
		status_(CoroutineStatus::COROUTINE_READY)   //协程初始为准备状态
	{
		run_ = fun;
	}

	Coroutine::~Coroutine()
	{
		delete stack_;
	}

	void Coroutine::Start(Coroutine* coroutine)
	{

		coroutine->run_();
		coroutine->SetStatus(CoroutineStatus::COROUTINE_SLEEP);	//函数运行完后将协程设置为休眠状态

		AddFreeCoroutine(coroutine->GetCoronode());		//将休眠协程放入队列
		DeleLivnum();
		SetRunning(nullptr);
		CoroutineCheck();
	}

}