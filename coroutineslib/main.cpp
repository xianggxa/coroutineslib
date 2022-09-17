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

