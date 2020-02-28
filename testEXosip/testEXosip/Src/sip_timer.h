#ifndef _SIP_TIMER_H
#define _SIP_TIMER_H

enum e_SIP_TIMER_EVENT
{
    SIP_TIMER_KEEP_ALIVE_TIME_OUT = 0,
}SIP_TIMER_EVENT;

typedef int (*timer_handle)(int, void *); // int timerId, void* param
typedef struct _SIP_TIMER_OBJ * SIP_TIMER_HANDLE;

SIP_TIMER_HANDLE sip_timer_create(void);
void sip_timer_destroy(SIP_TIMER_HANDLE handle);

int sip_timer_set(SIP_TIMER_HANDLE handle, int timerId, int elapse, timer_handle func, void *arg);
int sip_timer_reset(SIP_TIMER_HANDLE handle, int timerId);
int sip_timer_kill(SIP_TIMER_HANDLE handle, int timerId);

#endif





