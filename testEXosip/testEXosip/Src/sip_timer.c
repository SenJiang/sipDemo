#include <stdlib.h>
#include <unistd.h>
#import  <pthread.h>
#include <string.h>
#include "sip_thread.h"
#include "sip_timer.h"


    
#define TIMER_ID_AUTO       0
#define MAX_TIMER_CNT       20
    
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

enum e_TIMER_TYPE
{
    TIMER_TYPE_ONESHOT,
    TIMER_TYPE_INTERVAL,
}TIMER_TYPE;

typedef struct _SIP_TIMER_INFO
{
    int timerId;
    uint64_t baseTime;
    uint64_t elapse;
    timer_handle handle;
    void *arg;
} SIP_TIMER_INFO;
typedef struct _TIMER_ID_POOL
{
    int id; // 0 ~ MAX_TIMER_CNT
    int state; // 0=free 1=used
} TIMER_ID_POOL;

typedef struct _SIP_TIMER_OBJ
{
    uint64_t cpu_tick;
    SIP_TIMER_INFO *timer[MAX_TIMER_CNT];
    SIP_THREAD_HANDLE  thread_tick;
    SIP_THREAD_HANDLE  thread_run;
    TIMER_ID_POOL timer_id_pool[MAX_TIMER_CNT];
    pthread_mutex_t timer_mutex;
    pthread_mutex_t cpu_tick_mutex;
}SIP_TIMER_OBJ;

static void sip_timer_center(void *user_info);
static void sip_timer_run(long user_info);
static uint64_t sip_timer_get_cpu_tick(SIP_TIMER_HANDLE handle);

SIP_TIMER_HANDLE sip_timer_create(void)
{
    SIP_TIMER_OBJ * handle = (SIP_TIMER_OBJ *)calloc(1, sizeof(SIP_TIMER_OBJ));
    if (NULL == handle)
    {
        debug_err("not enough memory \n");
        return NULL;
    }
    pthread_mutex_init(&handle->timer_mutex, NULL);
    pthread_mutex_init(&handle->cpu_tick_mutex, NULL);
    handle->thread_tick = sip_thread_create(sip_timer_center, (void *)handle);
    if (NULL == handle->thread_tick)
    {
        debug_err("sip_thread_create failed \n");
        goto create_failed;
    }
    handle->thread_run = sip_thread_create(sip_timer_run, (void *)handle);
    if (NULL == handle->thread_run)
    {
        debug_err("sip_thread_create failed \n");
        goto create_failed;
    }
    memset(&handle->timer_id_pool, 0x00, sizeof(handle->timer_id_pool));
    // debug_info("sizeof(handle->timer_id_pool):%d\n", sizeof(handle->timer_id_pool));


    return handle;

create_failed:
    sip_timer_destroy(handle);

    return NULL;
}

void sip_timer_destroy(SIP_TIMER_HANDLE handle)
{
    int i = 0;
   if (NULL != handle)
   {
       if(NULL != handle->thread_tick)
       {
            sip_thread_destroy(handle->thread_tick);
           handle->thread_tick = NULL;
       }
       if(NULL != handle->thread_run)
       {
           sip_thread_destroy(handle->thread_run);
           handle->thread_run = NULL;
       }
       for(i = 0; i < MAX_TIMER_CNT; i++)
       {
           if(NULL != handle->timer[i])
           {
                free(handle->timer[i]);
           }
       }
       pthread_mutex_destroy(&handle->cpu_tick_mutex);
       pthread_mutex_destroy(&handle->timer_mutex);
       free(handle);
   }
}

static void sip_timer_center(void *user_info)
{
    SIP_TIMER_OBJ *handle = (SIP_TIMER_OBJ * )user_info;

    handle->cpu_tick = 0;
    while (1)
    {
        usleep( 100000 ); //100ms

        pthread_mutex_lock(&handle->cpu_tick_mutex);
        ++handle->cpu_tick;
        pthread_mutex_unlock(&handle->cpu_tick_mutex);
    }

}

/*elapse(单位ms)*/
int sip_timer_set(SIP_TIMER_HANDLE handle, int timerId, int elapse, timer_handle func, void *arg)
{
    if (NULL == handle)
    {
        debug_err("[%s:%d] handle == NULL\n",__FUNCTION__,__LINE__);
        return -1;
    }

    int i = 0;

    pthread_mutex_lock(&handle->timer_mutex);
    //remove old timer
    for(i = 0; i < MAX_TIMER_CNT; i++)
    {
        if(NULL != handle->timer[i])
        {
            if(handle->timer[i]->timerId == timerId)
            {
                debug_info("free timer[%d],id:%d\n", i, timerId);
                free(handle->timer[i]);
                handle->timer[i] = NULL;
                break;
            }
        }
    }
    //add new timer
    for(i = 0; i < MAX_TIMER_CNT; i++)
    {
        if(NULL == handle->timer[i])
        {
            debug_info("set  timer[%d],id:%d\n", i, timerId);
            handle->timer[i] = (SIP_TIMER_INFO *)malloc(sizeof(SIP_TIMER_INFO));
            if(NULL == handle->timer[i])
            {
                pthread_mutex_unlock(&handle->timer_mutex);
                debug_err("[error]malloc NULL\n");
                return -1;
            }
            handle->timer[i]->timerId = timerId;
            handle->timer[i]->baseTime = sip_timer_get_cpu_tick(handle);
            handle->timer[i]->elapse = (uint64_t )(elapse / 100);
            handle->timer[i]->handle = func;
            handle->timer[i]->arg = arg;
            break;
        }
    }
    if(i >= MAX_TIMER_CNT)
    {
        debug_info("timer exceed max users!!\n");
    }
    pthread_mutex_unlock(&handle->timer_mutex);

    return 0;
}

int sip_timer_reset(SIP_TIMER_HANDLE handle, int timerId)
{
    if (NULL == handle)
    {
        debug_err("handle == NULL\n");
        return -1;
    }

    int i = 0,is_found = 0;

    pthread_mutex_lock( &handle->timer_mutex );
    for( i = 0; i < MAX_TIMER_CNT; i++ )
    {
        if(NULL != handle->timer[i])
        {
            if( handle->timer[i]->timerId == timerId )
            {
                debug_info("reset  timer,id:%d\n", timerId);
                is_found = 1;
                handle->timer[i]->baseTime = sip_timer_get_cpu_tick(handle);
                break;;
            }
        }
    }
    if(0 == is_found)
    {
        debug_err(" no found timer,id:%d\n", timerId);
    }
    pthread_mutex_unlock( &handle->timer_mutex );

    return 0;
}

int sip_timer_kill(SIP_TIMER_HANDLE handle, int timerId)
{
     if (NULL == handle)
    {
         debug_err("handle == NULL\n");
        return -1;
    }
    int i = 0,is_found = 0;

    pthread_mutex_lock(&handle->timer_mutex);
    for(i = 0; i < MAX_TIMER_CNT; i++)
    {
        if(NULL != handle->timer[i])
        {
            if(handle->timer[i]->timerId == timerId)
            {
                debug_info("kill  timer[%d],id:%d\n", i, timerId);
                is_found = 1;
                free(handle->timer[i]);
                handle->timer[i] = NULL;
                break;
            }
        }

    }
    if(0 == is_found)
    {
        debug_err("no found timer,id:%d\n", timerId);
    }
    pthread_mutex_unlock(&handle->timer_mutex);

    return 0;
}

static void sip_timer_run(long user_info)
{
    SIP_TIMER_OBJ *handle = (SIP_TIMER_OBJ * )user_info;

    while (1)
    {
        int i = 0;
        uint64_t curTime = sip_timer_get_cpu_tick(handle);

        //pthread_mutex_lock(&handle->timer_mutex);
        for( i = 0; i < MAX_TIMER_CNT; i++ )
        {
            if(NULL != handle->timer[i])
            {
                if( (curTime - handle->timer[i]->baseTime) >= handle->timer[i]->elapse)
                {
                    // Exec handle
                    debug_info("timer[%d].id:%d timeout\n", i, handle->timer[i]->timerId);
                    handle->timer[i]->handle(handle->timer[i]->timerId, handle->timer[i]->arg);
                    free(handle->timer[i]);
                    handle->timer[i] = NULL;
                }
            }
    }
    //pthread_mutex_unlock(&handle->timer_mutex);
    usleep(100000);
}
}

static uint64_t sip_timer_get_cpu_tick(SIP_TIMER_HANDLE handle)
{
    if (NULL == handle)
    {
        debug_err("handle == NULL\n");
        return -1;
    }

    uint64_t cpuTick = 0;

    pthread_mutex_lock(&handle->cpu_tick_mutex);
    cpuTick = handle->cpu_tick;
    pthread_mutex_unlock(&handle->cpu_tick_mutex);

    return cpuTick;
}



