#ifndef THREADS_H
#define THREADS_H

#pragma once

#if defined(_MSC_VER)
#include <Windows.h>
#define THREAD_RET_TYPE DWORD WINAPI

#elif defined(__APPLE__)
#error "Apple thread primitives not supported"

#else
#include <pthread.h>
#define THREAD_RET_TYPE void*

#endif

typedef THREAD_RET_TYPE (ThreadFunc)(void* arg);

struct ThreadID{
#if defined(_MSC_VER)
	Handle handle;
	DWORD threadId;
#elif defined(__APPLE__)
	// Apple specific stuff
#else
	pthread_t id;
#endif
	
};

ThreadID CreateThreadSimple(ThreadFunc* func, void* arg);

void JoinThread(ThreadID thread);

int IncrementAtomic32(int* i);
int AddAtomic32(int* i, int val);
int CompareAndSwap32(int* i, int oldVal, int newVal);

// TODO: cross-platform 64-bit int type?
//void IncrementAtomic64(int* i);
//void AddAtomic64(int* i, int val);


struct Mutex{
#if defined(_MSC_VER)
	Handle mutx;
#elif defined(__APPLE__)
	// Apple specific stuff
#else
	pthread_mutex_t mutx;
#endif
};

void CreateMutex(Mutex* mutx);
void LockMutex(Mutex* mutx);
void UnlockMutex(Mutex* mutx);
void DestroyMutex(Mutex* mutx);

struct MutexScopeLock{
    Mutex* mutex;

    explicit MutexScopeLock(Mutex* mutx){
        mutex = mutx;
        LockMutex(mutex);
    }

    ~MutexScopeLock(){
        UnlockMutex(mutex);
    }
}; 

#define SCOPED_MUTEX(mtx) MutexScopeLock mutex_lock_ ## __LINE__ (mtx)

#endif