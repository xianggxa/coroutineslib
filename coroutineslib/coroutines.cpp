#include "coroutines.h"
namespace xiangrpc {
	Coroutine::Coroutine(std::function<void()> fun) :
		stack_size_(0),
		stack_cap_(0),
		stack_(nullptr),
		status_(CoroutineStatus::COROUTINE_READY)   //Э�̳�ʼΪ׼��״̬
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
		coroutine->SetStatus(CoroutineStatus::COROUTINE_SLEEP);	//�����������Э������Ϊ����״̬

		AddFreeCoroutine(coroutine->GetCoronode());		//������Э�̷������
		DeleLivnum();
		SetRunning(nullptr);
		CoroutineCheck();
	}

}