
#include "Semaphore.h"

#ifndef _WIN32
//#if 1
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> 

// add by 周满满
// mac os 平台下， 替代linux的sem_timedwait函数
#ifdef __APPLE__
#include <pthread.h>
#include <errno.h>
#include <mach/clock.h>
#include <mach/mach.h>
struct CSGX__sem_timedwait_Info
{
    pthread_mutex_t MxMutex;
    pthread_cond_t MxCondition;
    pthread_t MxParent;
    struct timespec MxTimeout;
    bool MxSignaled;
};

void *CSGX__sem_timedwait_Child(void *MainPtr)
{
    CSGX__sem_timedwait_Info *TempInfo = (CSGX__sem_timedwait_Info *)MainPtr;
    
    pthread_mutex_lock(&TempInfo->MxMutex);
    
    // Wait until the timeout or the condition is signaled, whichever comes first.
    int Result;
    do
    {
        Result = pthread_cond_timedwait(&TempInfo->MxCondition, &TempInfo->MxMutex, &TempInfo->MxTimeout);
        if (!Result)  break;
    }
    while (1);
    
    if (errno == ETIMEDOUT && !TempInfo->MxSignaled)
    {
        TempInfo->MxSignaled = true;
        pthread_kill(TempInfo->MxParent, SIGALRM);
    }
    
    pthread_mutex_unlock(&TempInfo->MxMutex);
    
    return NULL;
}

int CSGX__ClockGetTimeRealtime(struct timespec *ts)
{
    clock_serv_t cclock;
    mach_timespec_t mts;
    
    if (host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock) != KERN_SUCCESS)
        return -1;
    if (clock_get_time(cclock, &mts) != KERN_SUCCESS)  return -1;
    if (mach_port_deallocate(mach_task_self(), cclock) != KERN_SUCCESS)
        return -1;
    
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
    
    return 0;
}
#endif //__APPLE__


struct OS_Semaphore_Priv
{
	sem_t hSem;
};

OS_Semaphore::OS_Semaphore(int initial_value)
:m_Priv (NULL)
{
	Init(initial_value);
}

OS_Semaphore::~OS_Semaphore()
{
	if(m_Priv)
	{
		OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
		sem_destroy(&priv->hSem);
		delete priv;
	}
}

int OS_Semaphore::Init(int initial_value)
{
	OS_Semaphore_Priv* priv = new OS_Semaphore_Priv;
	if(!priv) return -1;
	m_Priv = priv;

	if(sem_init(&priv->hSem, 1, initial_value) < 0)
	{
		delete priv;
		m_Priv = NULL;
		return -1;
	}

	return 0;
}

int OS_Semaphore::Wait()
{
	OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
	if(!priv) return -1;

	if(sem_wait(&priv->hSem) < 0)
	{
		return -1;
	}

	return 0;
}

int OS_Semaphore::Wait(int ms)
{
	OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
	if(!priv) return -1;

	timeval tv_now;
	gettimeofday(&tv_now, NULL);

	timespec ts;
	ts.tv_sec = tv_now.tv_sec;
	ts.tv_nsec = tv_now.tv_usec * 1000;

	int ns = ts.tv_nsec + (ms % 1000)  * 1000000;
	ts.tv_nsec = ns % 1000000000;
	ts.tv_sec += ns / 1000000000;
	ts.tv_sec += ms / 1000;

#ifdef __APPLE__
    CSGX__sem_timedwait_Info sem_tiem_info;
    sem_tiem_info.MxTimeout = ts;
    CSGX__sem_timedwait_Child(&sem_tiem_info);
#else
    if(sem_timedwait(&priv->hSem, &ts) != 0)
    {
        return -1;
    }
#endif
	

	return 0;;
}

void OS_Semaphore::Post()
{
	OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
	if(!priv) return;

	sem_post(&priv->hSem);
}
#endif // ! _WIN32


