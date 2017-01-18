#ifndef THREADS_H
#define THREADS_H

#pragma once

#if defined(_MSC_VER)
#include <Windows.h>
#define THREAD_RET_TYPE DWORD __stdcall
typedef DWORD (__stdcall *ThreadFunc) (void* arg);

#else
#include <pthread.h>
#define THREAD_RET_TYPE void*
typedef void* (*ThreadFunc) (void* arg);

#endif

struct BNS_ThreadID{
#if defined(_MSC_VER)
	HANDLE handle;
	DWORD threadId;
#else
	pthread_t id;
#endif
	
};

BNS_ThreadID CreateThreadSimple(ThreadFunc* func, void* arg);

void JoinThread(BNS_ThreadID thread);

unsigned int IncrementAtomic32(volatile unsigned int* i);
int AddAtomic32(volatile int* i, int val);
int CompareAndSwap32(volatile int* i, int oldVal, int newVal);

// TODO: cross-platform 64-bit int type?
//void IncrementAtomic64(int* i);
//void AddAtomic64(int* i, int val);


struct Mutex{
#if defined(_MSC_VER)
	HANDLE handle;
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