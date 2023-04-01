/************************************************************************
 * @file: rtos_health_monitor.c
 * @author: xxx
 * @brief: xxx
 * @version: 1.0.0
 * @LastEditTime: 2023-04-01 11:06:43
 * @attention:  1. OSIdleTaskTCB
 *              2. 
 *  top - 15:21:32 up 22:24,  0 users,  load average: 0.00, 0.00, 0.00
    Tasks:   5 total,   1 running,   4 sleeping,   0 stopped,   0 zombie
    %Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    KiB Mem :  8150768 total,  6332968 free,    69040 used,  1748760 buff/cache
    KiB Swap:  2097152 total,  2097152 free,        0 used.  7815560 avail Mem
 *************************************************************************/

#include <stdio.h>
#include "os.h"
#include "SEGGER_RTT.h"

OS_CPU_USAGE OSStatLoadAverage = 0;     /* CPU平均负载 */
static CPU_INT32U s_ReadyTaskNum = 0;
static CPU_INT32U s_ReadyTaskSampleCount = 0;

static char s_print_buf[256 + 1];
static int s_print_len = 0;

/**************************************************************************
 *                           系统CPU使用率测量
 * 1. OSStatTaskCPUUsage = 
***************************************************************************/

void  App_OS_StatTaskHook (void)
{
    static CPU_INT08U print_timer = 0;
    
    OS_TCB *p_tcb = NULL;
    p_tcb = OSTaskDbgListPtr;
    CPU_INT08U readyTaskNum = 0;
    CPU_INT08U delayedTaskNum = 0;
    CPU_INT08U suspendedTaskNum = 0;

    while (p_tcb != (OS_TCB *)0) {

        if(p_tcb->TaskState == OS_TASK_STATE_RDY) {
            readyTaskNum++;
        }
        else if(p_tcb->TaskState == OS_TASK_STATE_DLY) {
            delayedTaskNum++;
        }
        else if(p_tcb->TaskState & 0x04) {
            suspendedTaskNum++;
        }

        p_tcb = p_tcb->DbgNextPtr;
    }
    s_print_len = snprintf(&s_print_buf[0],sizeof(s_print_buf),                                     \
                        "Tasks: %d total, %d Ready %d sleeping %d Suspended\r\n",                   \
                        OSTaskQty,readyTaskNum,delayedTaskNum,suspendedTaskNum);
    
    s_print_len += snprintf(&s_print_buf[s_print_len],sizeof(s_print_buf),                          \
                        "Load Average:%d.%02d(1s)\r\n",                                           \
                        OSStatLoadAverage/100,OSStatLoadAverage%100);
    
    s_print_len += snprintf(&s_print_buf[s_print_len],sizeof(s_print_buf),                          \
                        "CPU: %d.%02d%%(%dms), %d.%02d%%(Max)\r\n",                                 \
                        OSStatTaskCPUUsage/100,OSStatTaskCPUUsage%100,1000/OSCfg_StatTaskRate_Hz,   \
                        OSStatTaskCPUUsageMax/100,OSStatTaskCPUUsageMax%100);
    
    
    s_print_len += snprintf(&s_print_buf[s_print_len],sizeof(s_print_buf),"\r\n\r\n");

    if ((++print_timer) & 0x04) {
        SEGGER_RTT_Write(0,(const void*)&s_print_buf[0],s_print_len);
    }
}

/**************************************************************************
 *                           系统CPU平均负载测量
 * 1. 通过统计单位时间内就绪态任务数量
***************************************************************************/

#if (OS_CFG_DBG_EN > 0u)

/* 假定1ms调用一次 */
void Calculate_LoadAverage(unsigned char cycle)
{
    CPU_INT08U i = 0;
    OS_RDY_LIST  *p_rdy_list = NULL;

    for (i = 0u; i < (OS_CFG_PRIO_MAX - 1); i++) {
        p_rdy_list = &OSRdyList[i];
        s_ReadyTaskNum += p_rdy_list->NbrEntries;
    }

    if ((++s_ReadyTaskSampleCount) >= cycle * 10)
    {
        OSStatLoadAverage = (s_ReadyTaskNum * 100) / s_ReadyTaskSampleCount;
        s_ReadyTaskNum = 0;
        s_ReadyTaskSampleCount = 0;
    }
}

#endif


void  App_OS_TimeTickHook (void)
{
    Calculate_LoadAverage(1);
}

