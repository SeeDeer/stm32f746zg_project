/************************************************************************
 * @file: sys_arch.h
 * @author: SeeDeer
 * @brief: 提供 lwIP 代码和底层操作系统内核之间的通用接口
 * @version: 1.0.0
 * @attention:
 *************************************************************************/
#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include "cmsis_os.h"

typedef u32_t sys_prot_t;

typedef osSemaphoreDef_t sys_sem_t;
#define sys_sem_valid(sem)             (((sem) != NULL) && (sem->Type == OS_OBJ_TYPE_SEM))
#define sys_sem_set_invalid(sem)       do { if((sem) != NULL) { sem->Type = OS_OBJ_TYPE_NONE; }}while(0)

typedef osMutexDef_t sys_mutex_t;
#define sys_mutex_valid(mutex)         (((mutex) != NULL) && (mutex->Type == OS_OBJ_TYPE_MUTEX))
#define sys_mutex_set_invalid(mutex)   do { if((mutex) != NULL) { mutex->Type = OS_OBJ_TYPE_NONE; }}while(0)

typedef struct os_q sys_mbox_t;
#define sys_mbox_valid(mbox)           (((mbox) != NULL) && (mbox->Type == OS_OBJ_TYPE_Q))
#define sys_mbox_set_invalid(mbox)     do { if((mbox) != NULL) { mbox->Type = OS_OBJ_TYPE_NONE; }}while(0)

typedef osThreadId sys_thread_t;

#endif /* LWIP_ARCH_SYS_ARCH_H */

