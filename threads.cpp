#include "threads.h"
#include "assert.h"

#if defined(_MSC_VER)

BNS_ThreadID CreateThreadSimple(ThreadFunc func, void* arg){
	BNS_ThreadID id ={};
	id.handle = CreateThread(NULL, 0, func, arg, 0, &id.threadId);
	return id;
}

void JoinThread(BNS_ThreadID thread){
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

#else
	
BNS_ThreadID CreateThreadSimple(ThreadFunc func, void* arg){
	BNS_ThreadID tid = {};
    int err = pthread_create(&tid.id, NULL, func, arg);
	return tid;
}

void JoinThread(BNS_ThreadID thread){
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
	pthread_mutex_destroy(&mutx->mutx);
}

unsigned int IncrementAtomic32(volatile unsigned int* i){
	return __sync_fetch_and_add(i, 1);
}

int AddAtomic32(volatile int* i, int val){
	return __sync_fetch_and_add(i, val); 
}

int CompareAndSwap32(volatile int* i, int oldVal, int newVal){
	return __sync_val_compare_and_swap(i, oldVal, newVal);
}

#endif



#if defined(THREADS_TEST_MAIN)

#include "assert.cpp"

int val1 = 0;
int val2 = 0;

THREAD_RET_TYPE ThreadFunc1(void* arg){
	int val = (int)(size_t)arg;
	val1 = val;
	
	return 0;
}

THREAD_RET_TYPE ThreadFunc2(void* arg){
	int val = (int)(size_t)arg;
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
	
	BNS_ThreadID id1 = CreateThreadSimple(ThreadFunc1, (void*)4);
	BNS_ThreadID id2 = CreateThreadSimple(ThreadFunc2, (void*)7);
	
	JoinThread(id1);
	JoinThread(id2);
	
	ASSERT(val1 == 4);
	ASSERT(val2 == 7);
	
	ASSERT(counter == 0);
	BNS_ThreadID tids[] = 
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
