#include <pthread.h>
#include "sip_common.h"
#include "sip_thread.h"
#include <stdlib.h>

typedef struct _SIP_THREAD_OBJECT
{
    pthread_t tid;
} SIP_THREAD_OBJECT;

SIP_THREAD_HANDLE sip_thread_create(THREAD_FUNC func, void *user_info)
{
    SIP_THREAD_OBJECT * handle = (SIP_THREAD_OBJECT *)calloc(1, sizeof(SIP_THREAD_OBJECT));
    if (NULL == handle)
    {
        debug_err("not enough memory \n");
        return NULL;
    }
    if (-1 == pthread_create(&handle->tid, NULL, func, (void*)user_info))
    {
        debug_err("pthread create failed");
        goto create_failed;
    }

    return handle;
create_failed:
    sip_thread_destroy(handle);
    
    return NULL;
}

void sip_thread_destroy(SIP_THREAD_HANDLE handle)
{
    if (NULL != handle)
    {
        pthread_join(handle->tid, NULL);
        free(handle);
    }
}

