#include"coroutines.h"
#include<iostream>
#include<string>
#include<thread>
#include<unistd.h>
using namespace std;

void fun1(string &name) {

	for (int i = 0; i < 10; i++) {
		std::cout << i << " " << name <<std::endl;
		xiangrpc::Yield();
	}
}
void fun2(string &name) {

	for (int i = 0; i < 10; i++) {
		std::cout << i*2 << " " << name << std::endl;
		xiangrpc::Yield();
	}
}
void mainfun(const string &name) {
	auto p1 = xiangrpc::AddTask(bind(&fun1, name));
	auto p2 = xiangrpc::AddTask(bind(&fun2, name));
	while (p1->GetStatus() != xiangrpc::CoroutineStatus::COROUTINE_SLEEP || p2->GetStatus() != xiangrpc::CoroutineStatus::COROUTINE_SLEEP) {
		xiangrpc::Resume(p1);
		sleep(1);
		xiangrpc::Resume(p2);
	}

}




int main() {
	thread t1(mainfun, "t1");
	thread t2(mainfun, "t2");
	t1.join();
	t2.join();
	return 0;
}
/*
//跨线程

class CorotineTaskQueue {
public:
	static CorotineTaskQueue& getCorotineTaskQueue() {
		static CorotineTaskQueue instance;
		return instance;

	}
	void push(xiangrpc::Coroutine* coro) {
		lock_guard<mutex> lock(mute_);
		q_.push(coro);

	}
	int getsize() {
		lock_guard<mutex> lock(mute_);
		return q_.size();
	}

	xiangrpc::Coroutine* pop() {
		xiangrpc::Coroutine* re = nullptr;
		{
			lock_guard<mutex> lock(mute_);
			if (q_.size() > 0) {
				re = q_.front();
				q_.pop();
			}
		}
		return re;
	}
private:
	std::queue<xiangrpc::Coroutine*> q_;
	std::mutex mute_;
};

void fun1(string& name) {

	for (int i = 0; i < 10; i++) {
		std::cout << i << " " << name << std::endl;
		xiangrpc::Yield();
	}
}
void fun2(string& name) {

	for (int i = 0; i < 10; i++) {
		std::cout << i * 2 << " " << name << std::endl;
		xiangrpc::Yield();
	}
}
void mainfun(const string& name) {
	auto p1 = xiangrpc::AddTask(bind(&fun1, name));
	auto p2 = xiangrpc::AddTask(bind(&fun2, name));
	while (p1->GetStatus() != xiangrpc::CoroutineStatus::COROUTINE_SLEEP || p2->GetStatus() != xiangrpc::CoroutineStatus::COROUTINE_SLEEP) {
		//xiangrpc::Resume(p1);
		CorotineTaskQueue::getCorotineTaskQueue().push(p1);
		sleep(1);
		CorotineTaskQueue::getCorotineTaskQueue().push(p2);
		//xiangrpc::Resume(p2);
		//std::cout << "fuck" << std::endl;
	}

}
void mainfun2() {
	sleep(1);
	while (CorotineTaskQueue::getCorotineTaskQueue().getsize()) {
		auto coro = CorotineTaskQueue::getCorotineTaskQueue().pop();
		if (coro != nullptr && coro->GetStatus() != xiangrpc::CoroutineStatus::COROUTINE_SLEEP) {
			xiangrpc::Resume(coro);
		}
		sleep(1);
	}
}




int main() {
	thread t1(mainfun, "t1");
	thread t2(mainfun2);
	t1.join();
	t2.join();
	return 0;
}
*/

