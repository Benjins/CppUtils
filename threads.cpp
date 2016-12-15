#include "threads.h"

#if defined(_MSC_VER)

ThreadID CreateThreadSimple(ThreadFunc* func, void* arg){
	ThreadID id ={};
	id.handle = CreateThread(NULL, 0, RunClientFunction, arg, 0, &id.threadId);
	return id;
}

void JoinThread(ThreadID thread){
	WaitForSingleObject(thread.handle, INFINITE);
}

void CreateMutex(Mutex* mutx){
	mutx->handle = CreateMutex(NULL, false, NULL);
}

void LockMutex(Mutex* mutx){
	WaitForSingleObject(mutx->handle, INFINITE);
}

void UnlockMutex(Mutex* mutx){
	ReleaseMutex(mutx->handle);
}

void DestroyMutex(Mutex* mutx){
	CloseHandle(mutx->handle);
	mutx->handle = 0;
}

int IncrementAtomic32(int* i){
	InterlockedIncrement(i);
}

int AddAtomic32(int* i, int val){
	InterlockedAdd(i, val);
}

int CompareAndSwap32(int* i, int oldVal, int newVal){
	int ret;
	InterlockedCompareExchange(i, oldVal, newVal, &ret);
	return ret;
}


#elif defined(__APPLE__)
	// Apple specific stuff
#else
	
ThreadID CreateThreadSimple(ThreadFunc* func, void* arg){
	ThreadID tid = {};
    int err = pthread_create(&tid.id, NULL, func, arg);
	return tid;
}

void JoinThread(ThreadID thread){
	pthread_join(thread.id, NULL);
}

void CreateMutex(Mutex* mutx){
	pthread_mutex_init(&mutx->mutx, NULL);
}

void LockMutex(Mutex* mutx){
	pthread_mutex_lock(&mutx->mutx);
}

void UnlockMutex(Mutex* mutx){
	pthread_mutex_unlock(&mutx->mutx);
}

void DestroyMutex(Mutex* mutx){
	pthread_mutex_destroy(&mutx->mutx, NULL);
}

int IncrementAtomic32(int* i){
	
}

int AddAtomic32(int* i, int val){
	
}

int CompareAndSwap32(int* i, int oldVal, int newVal){
	
}

#endif



#if defined(THREADS_TEST_MAIN)

int main(){
	
	
	
	return -1;
}

#endif
