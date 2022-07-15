#include"coroutines.h"
#include<iostream>


void fun1(Schedule* s) {

	for (int i = 0; i < 5; i++) {
		std::cout << i << std::endl;
		s->Yield();
	}
}
void fun2(Schedule* s) {

	for (int i = 0; i < 10; i++) {
		std::cout << i*2 << std::endl;
		s->Yield();
	}
}

int main() {
	Schedule* s = new Schedule();
	Coroutine* c1 = s->AddTask(std::bind(&fun1, s));
	Coroutine* c2 = s->AddTask(std::bind(&fun2, s));
	while (c1->GetStatus() != CoroutineStatus::COROUTINE_SLEEP || c2->GetStatus() != CoroutineStatus::COROUTINE_SLEEP) {
		
		s->Resume(c1);
		
		s->Resume(c2);
		
	}

}