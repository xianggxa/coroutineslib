#pragma once

#include<functional>
#include<ucontext.h>
#include<list>
#include<queue>
#include<string.h>



namespace xiangrpc {
#define STACK_SIZE (1024*1024)

	class Coroutine;




	enum class CoroutineStatus
	{
		COROUTINE_SLEEP,
		COROUTINE_READY,
		COROUTINE_RUNNING,
		COROUTINE_SUSPEND

	};


	struct CoroNode {
		CoroNode* pre_ = nullptr;
		CoroNode* next_ = nullptr;
		Coroutine* cur_cor = nullptr;
		CoroNode(Coroutine* coroutine) :cur_cor(coroutine) {}
	};
	struct CoroList {



		CoroNode* head_ = nullptr;
		CoroNode* back_ = nullptr;
		void push_back(CoroNode* coronode) {
			if (back_ == nullptr) {
				head_ = coronode;
				back_ = coronode;
			}
			else {
				back_->next_ = coronode;
				coronode->pre_ = back_;
				back_ = back_->next_;
			}
		}
		void erase(CoroNode* coronode) {
			if (coronode == head_) {
				head_ = coronode->next_;
			}
			if (coronode == back_) {
				back_ = coronode->pre_;
			}
			if (coronode->pre_ != nullptr) {
				coronode->pre_->next_ = coronode->next_;
			}
			if (coronode->next_ != nullptr) {
				coronode->next_->pre_ = coronode->pre_;
			}

		}

	};
	class Coroutine {
	public:
		Coroutine(std::function<void()> fun);
		~Coroutine();
		static void Start(Coroutine* coroutine);	//��װִ�к���
		void SetRun(std::function<void()> fun) { run_ = fun; }
		void SetStatus(CoroutineStatus statu) { status_ = statu; }
		void SetStackCap(size_t stack_cap) { stack_cap_ = stack_cap; }
		void SetStackSize(size_t stack_size) { stack_size_ = stack_size; }
		void SetNode(CoroNode* coronode) { this->coronode = coronode; }
		CoroutineStatus GetStatus() { return status_; }
		ucontext_t* GetUcontext() { return &ctx_; }
		size_t GetStackSize() { return stack_size_; }
		char* GetStack() { return stack_; }
		size_t GetStackCap() { return stack_cap_; }
		CoroNode* GetCoronode() { return coronode; }
		void Resize() { stack_ = new char[stack_cap_]; }

	private:
		CoroutineStatus status_;				//Э��״̬
		std::function<void()> run_;				//ִ�к�������
		ucontext_t ctx_;						//Э��������
		size_t stack_size_;						//ջ���пռ��С
		size_t stack_cap_;						//ջ����
		char* stack_;							//ջ��ַ
												//Э��ջ�����г�ʱ��� ����ʱʹ�ù���ջ
		CoroNode* coronode;						//����Э���������ж�Ӧ��λ��



	};

		
		static thread_local ucontext_t sche_main_;

		static thread_local int sche_size_;						//Э������
		static thread_local int liv_num_;						//����Э����

		static thread_local std::queue<CoroNode*> unu_cor_;		//�������Э��
		static thread_local CoroList cred_cor_;					//�������д�����Э��
		static thread_local Coroutine* p_running;				//�������е�Э��

		static thread_local char sche_stack_[STACK_SIZE];		//����ջ


		static Coroutine* CreateCoroutine(std::function<void()> fun)//������Э��
		{
			Coroutine* cor = new Coroutine(fun);
			++sche_size_;
			++liv_num_;
			CoroNode* coronode = new CoroNode(cor);
			cred_cor_.push_back(coronode);
			coronode->cur_cor->SetNode(coronode);

			return coronode->cur_cor;


		}
		static Coroutine* AddTask(std::function<void()> fun)							//�������
		{
			if (!unu_cor_.empty()) {
				auto p = unu_cor_.front();
				unu_cor_.pop();
				++liv_num_;
				p->cur_cor->SetRun(fun);
				p->cur_cor->SetStatus(CoroutineStatus::COROUTINE_READY);
				p->cur_cor->SetNode(p);
				return p->cur_cor;

			}
			else return CreateCoroutine(fun);
		}

		static void Resume(Coroutine* coroutine)											//�ָ�Э���ֳ�
		{

			if (coroutine == nullptr)return;
			switch (coroutine->GetStatus()) {
			case CoroutineStatus::COROUTINE_READY: {									//Э��׼��״̬��Э�̽��г�ʼ��
				ucontext_t* ctx = coroutine->GetUcontext();
				getcontext(ctx);
				ctx->uc_stack.ss_sp = sche_stack_;									//��Э�̵�ַ��Ϊ����ջ��ַ
				ctx->uc_stack.ss_size = STACK_SIZE;
				ctx->uc_link = &sche_main_;
				p_running = coroutine;												//���Ϊ��ǰ�������е�Э��
				coroutine->SetStatus(CoroutineStatus::COROUTINE_RUNNING);
				makecontext(ctx, (void(*)())Coroutine::Start, 1, coroutine);	//ΪЭ������ִ�к���
				swapcontext(&sche_main_, ctx);										//���浱ǰ�����Ĳ��л�ΪЭ��������
				break;
			}
			case CoroutineStatus::COROUTINE_SUSPEND: {								//Э���ж�״̬
				memcpy(sche_stack_ + STACK_SIZE - coroutine->GetStackSize(), coroutine->GetStack(), coroutine->GetStackSize());																	//memcpy�ӵ͵�ַ���ߵ�ַ����
				p_running = coroutine;
				coroutine->SetStatus(CoroutineStatus::COROUTINE_RUNNING);
				ucontext_t* ctx = coroutine->GetUcontext();
				swapcontext(&sche_main_, ctx);
				break;
			}
			default:
				break;

			}



		}
		static void AddFreeCoroutine(CoroNode* coronode) { unu_cor_.push(coronode); }
		static void end()
		{

			for (auto p = cred_cor_.head_; p != nullptr; p = p->next_) {
				delete p->cur_cor;
				delete p;
			}
		}

		static void CoroutineCheck()														//������Э�̽϶�ʱ��������
		{
			if (liv_num_ <= sche_size_ / 4 && liv_num_ > 100) {	//������Э�̽϶�ʱ��������
				int num = sche_size_ / 2;
				while (num--) {
					CoroNode* coronode = unu_cor_.front();
					unu_cor_.pop();
					cred_cor_.erase(coronode);
					delete coronode->cur_cor;
					delete coronode;
				}
			}
		}
		static void DeleLivnum() { liv_num_--; }
		static void SetRunning(Coroutine* coroutine) { p_running = coroutine; }
		static ucontext_t* GetMainctx() { return &sche_main_; }
		static void Yield()																//�л�Э��
		{
			Coroutine* coroutine = p_running;
			if (coroutine == nullptr)return;
			char dummy = 0;
			if (coroutine->GetStackCap() < sche_stack_ + STACK_SIZE - &dummy) {	//��Э��ԭ������С������ʱ
				delete coroutine->GetStack();
				coroutine->SetStackCap(sche_stack_ + STACK_SIZE - &dummy);
				coroutine->Resize();

			}
			coroutine->SetStackSize(sche_stack_ + STACK_SIZE - &dummy);
			memcpy(coroutine->GetStack(), &dummy, coroutine->GetStackSize());	//��Э��ջ���Ƶ�����ջ
			coroutine->SetStatus(CoroutineStatus::COROUTINE_SUSPEND);
			p_running = nullptr;
			ucontext_t* ctx = coroutine->GetUcontext();
			swapcontext(ctx, &sche_main_);



		}
	

}
	

/*
----------------------
|					 |											|  height
|--------------------|-->   stack + STACK_SIZE	-				|
|////////////////////|							|				|
|////////////////////|-->   used				|				|
|////////////////////|							|->STACK_SIZE	|
|--------------------|-->	dummy				|				|
|					 |							|				|
|--------------------|-->	stack				-				|  low
|																V
|
Э�̵Ķѿռ����Ϊջ�ռ�Ĵ洢���ռ䲼����ͬ

stack ��Ϊջ�ռ��Ϸ��������
����stack[k] Ϊstack�����׵�ַ��ƫ��������
���stackΪ���������ջ�ռ��ڵĵ͵�ַ

*/