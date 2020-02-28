#ifndef __SIP_THREAD_H
#define __SIP_THREAD_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "sip_common.h"

typedef struct _SIP_THREAD_OBJECT * SIP_THREAD_HANDLE;

SIP_THREAD_HANDLE sip_thread_create(THREAD_FUNC func, void *user_info);
void sip_thread_destroy(SIP_THREAD_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif

