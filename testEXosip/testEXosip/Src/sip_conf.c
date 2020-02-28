#include <pthread.h>
#include "sip_conf.h"
#include <stdlib.h>


typedef struct _SIP_CONF_OBJECT
{
    pthread_mutex_t config_mutex;
    SIP_CONFIG config;
    char send_buf[4096];
    void *user_info;
}SIP_CONF_OBJECT;

SIP_CONF_HANDLE sip_conf_create(void *user_info)
{
    SIP_CONF_OBJECT * handle = (SIP_CONF_OBJECT *)calloc(1, sizeof(SIP_CONF_OBJECT));
    if (NULL == handle)
    {
        debug_err("not enough memory \n");
        return NULL;
    }
    pthread_mutex_init(&handle->config_mutex, NULL);
    if (NULL != user_info)
    {
        handle->user_info = user_info;
    }

    return handle;
create_failed:
    sip_conf_destroy(handle);
    return NULL;
}
void sip_conf_destroy(SIP_CONF_HANDLE handle)
{
    if (NULL != handle)
    {
        pthread_mutex_destroy(&handle->config_mutex);
        free(handle);
        handle = NULL;
    }
}

int sip_conf_start(SIP_CONF_HANDLE handle, SIP_CONFIG *pConfig)
{
    if ((NULL == handle) || (NULL == pConfig))
    {
        debug_err("invalid param");
        return -1;
    }
    memcpy(&handle->config, pConfig, sizeof(handle->config));

    return 0;
}

int sip_conf_get_config(SIP_CONF_HANDLE handle, SIP_CONFIG *pConfig)
{
    if ((NULL == handle) || (NULL == pConfig))
    {
        debug_err("invalid param");
        return -1;
    }
    memcpy(pConfig, &handle->config, sizeof(handle->config));

    return 0;
}

