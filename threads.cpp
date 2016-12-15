#include "threads.h"
#include "assert.h"

#if defined(_MSC_VER)

ThreadID CreateThreadSimple(ThreadFunc func, void* arg){
	ThreadID id ={};
	id.handle = CreateThread(NULL, 0, func, arg, 0, &id.threadId);
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

unsigned int IncrementAtomic32(volatile unsigned int* i){
	return InterlockedIncrement(i);
}

int AddAtomic32(volatile int* i, int val){
	return InterlockedAdd((volatile LONG*)i, val);
}

static_assert(sizeof(LONG) == sizeof(int), "MSVC, why...");

int CompareAndSwap32(volatile int* i, int oldVal, int newVal){
	return InterlockedCompareExchange((volatile LONG*)i, oldVal, newVal);
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

int IncrementAtomic32(volatile int* i){
	
}

int AddAtomic32(volatile int* i, int val){
	
}

int CompareAndSwap32(volatile int* i, int oldVal, int newVal){
	
}

#endif



#if defined(THREADS_TEST_MAIN)

int val1 = 0;
int val2 = 0;

THREAD_RET_TYPE ThreadFunc1(void* arg){
	int val = (int)arg;
	val1 = val;
	
	return 0;
}

THREAD_RET_TYPE ThreadFunc2(void* arg){
	int val = (int)arg;
	val2 = val;
	
	return 0;
}

static const int counterSize = 1024*32;
unsigned int counter = 0;

THREAD_RET_TYPE ThreadCounterFunc(void* arg){
	
	for (int i = 0; i < counterSize; i++){
		IncrementAtomic32(&counter);
	}
	
	return 0;
}

int main(){
	
	ASSERT(val1 == 0);
	ASSERT(val2 == 0);
	
	ThreadID id1 = CreateThreadSimple(ThreadFunc1, (void*)4);
	ThreadID id2 = CreateThreadSimple(ThreadFunc2, (void*)7);
	
	JoinThread(id1);
	JoinThread(id2);
	
	ASSERT(val1 == 4);
	ASSERT(val2 == 7);
	
	ASSERT(counter == 0);
	ThreadID tids[] = 
	{
		CreateThreadSimple(ThreadCounterFunc, NULL),
		CreateThreadSimple(ThreadCounterFunc, NULL),
		CreateThreadSimple(ThreadCounterFunc, NULL),
		CreateThreadSimple(ThreadCounterFunc, NULL),
		CreateThreadSimple(ThreadCounterFunc, NULL),
		CreateThreadSimple(ThreadCounterFunc, NULL)
	};
	
	for (int i = 0; i < BNS_ARRAY_COUNT(tids); i++){
		JoinThread(tids[i]);
	}
	
	ASSERT(counter == counterSize * BNS_ARRAY_COUNT(tids));
	
	return 0;
}

#endif
