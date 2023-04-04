/************************************************************************
 * @file: sys_arch.c
 * @author: SeeDeer
 * @brief: 提供 lwIP 代码和底层操作系统内核之间的通用接口
 * @version: 1.0.0
 * @attention: 1.参考 STABLE-2_0_3_RELEASE/doc/sys_arch.txt 说明实现,
 *               不知为何，2.0.3后续版本sys_arch.txt文件就消失了
 *             
 *************************************************************************/

#include "lwip/sys.h"

/* Is called to initialize the sys_arch layer. */
void sys_init(void)
{

}

/**
 * @ingroup sys_time
 * Returns the current time in milliseconds,
 * may be the same as sys_jiffies or at least based on it.
 * Don't care for wraparound, this is only used for time diffs.
 * Not implementing this function means you cannot use some modules (e.g. TCP
 * timestamps, internal timeouts for NO_SYS==1).
 */
u32_t sys_now(void)
{
    return osKernelMicroSecSysTick(osKernelSysTick());
}

/**
 * @ingroup sys_misc
 * Sleep for specified number of ms
 */
// void sys_msleep(u32_t ms)
// {
//     OS_ERR os_err;
//     OS_TICK dly = osKernelSysTickMicroSec(ms);
//     OSTimeDly(dly,OS_OPT_TIME_DLY,&os_err);
// }

/******************************************************************************/
/*                      Critical Region Protection                            */
/******************************************************************************/
#if (defined (SYS_LIGHTWEIGHT_PROT)  &&  (SYS_LIGHTWEIGHT_PROT != 0))

static CPU_SR  cpu_sr = (CPU_SR)0;
static CPU_REG32 sys_arch_protect_nesting = 0;

sys_prot_t sys_arch_protect(void)
{
    if((sys_arch_protect_nesting++) == 0)
    {
        CPU_CRITICAL_ENTER();
    }
}

void sys_arch_unprotect(sys_prot_t pval)
{
    LWIP_UNUSED_ARG(pval);
    if((--sys_arch_protect_nesting) == 0)
    {
        CPU_CRITICAL_EXIT();
    }
}
#endif

/******************************************************************************/
/*                             sys_thread_new                                 */
/******************************************************************************/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    (void)name;

    osThreadDef_t os_thread_def_lwip_thread = {
        .pthread = thread,
        .tpriority = prio,
        .stacksize = stacksize,
    };

    return osThreadCreate(&os_thread_def_lwip_thread,arg);
}

/**
 * @ingroup sys_mutex
 * Create a new mutex.
 * Note that mutexes are expected to not be taken recursively by the lwIP code,
 * so both implementation types (recursive or non-recursive) should work.
 * The mutex is allocated to the memory that 'mutex'
 * points to (which can be both a pointer or the actual OS structure).
 * If the mutex has been created, ERR_OK should be returned. Returning any
 * other error will provide a hint what went wrong, but except for assertions,
 * no real error handling is implemented.
 * 
 * @param mutex pointer to the mutex to create
 * @return ERR_OK if successful, another err_t otherwise
 */
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);

    if (osMutexCreate((const osMutexDef_t *)mutex) == NULL) {
        return ERR_MEM;
    }

    return ERR_OK;
}
/**
 * @ingroup sys_mutex
 * Blocks the thread until the mutex can be grabbed.
 * @param mutex the mutex to lock
 */
void sys_mutex_lock(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    osStatus osStat = osMutexWait((osMutexId)mutex, osWaitForever);
    LWIP_ASSERT("osStat == osOK", osStat == osOK);
}
/**
 * @ingroup sys_mutex
 * Releases the mutex previously locked through 'sys_mutex_lock()'.
 * @param mutex the mutex to unlock
 */
void sys_mutex_unlock(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    osStatus osStat = osMutexRelease((osMutexId)mutex);
    LWIP_ASSERT("osStat == osOK", osStat == osOK);
}
/**
 * @ingroup sys_mutex
 * Deallocates a mutex.
 * @param mutex the mutex to delete
 */
void sys_mutex_free(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    osStatus osStat = osMutexDelete((osMutexId)mutex);
    LWIP_ASSERT("osStat == osOK", osStat == osOK);
}

/******************************************************************************/
/*           semaphores                                                       */
/******************************************************************************/
/**
  * @brief  Create a new semaphore.
  * @param[in]  sem pointer to the semaphore to create.
  * @param[in]  count initial count of the semaphore.
  * @retval ERR_OK if successful, another err_t otherwise.
  * @attention 1.The semaphore is allocated to the memory that 'sem'
  *              points to (which can be both a pointer or the actual OS structure).
  *            2.The "count" argument specifies the initial state of the semaphore 
  *              (which is either 0 or 1).
  */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    osSemaphoreId semId = osSemaphoreCreate((const osSemaphoreDef_t *)sem, (int32_t)count);
    LWIP_ASSERT("semId != NULL", semId != NULL);

    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    osSemaphoreDelete((osSemaphoreId)sem);
}

void sys_sem_signal(sys_sem_t *sem)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    osSemaphoreRelease((osSemaphoreId)sem);
}

/**
 * @ingroup sys_sem
 *  Blocks the thread while waiting for the semaphore to be signaled. If the
 * "timeout" argument is non-zero, the thread should only be blocked for the
 * specified time (measured in milliseconds). If the "timeout" argument is zero,
 * the thread should be blocked until the semaphore is signalled.
 * 
 * The return value is SYS_ARCH_TIMEOUT if the semaphore wasn't signaled within
 * the specified time or any other value if it was signaled (with or without
 * waiting).
 * Notice that lwIP implements a function with a similar name,
 * sys_sem_wait(), that uses the sys_arch_sem_wait() function.
 * 
 * @param sem the semaphore to wait for
 * @param timeout timeout in milliseconds to wait (0 = wait forever)
 * @return SYS_ARCH_TIMEOUT on timeout, any other value on success
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);

    u32_t tout = ((timeout == 0) ? osWaitForever : timeout);
    if (osSemaphoreWait((osSemaphoreId)sem, tout) == -1) {
        return SYS_ARCH_TIMEOUT;
    }

/* Old versions of lwIP required us to return the time waited.
    This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
    here is enough. */
    return 1;
}



/**
 * @ingroup sys_mbox
 * Creates an empty mailbox for maximum "size" elements. Elements stored
 * in mailboxes are pointers. You have to define macros "_MBOX_SIZE"
 * in your lwipopts.h, or ignore this parameter in your implementation
 * and use a default size.
 * If the mailbox has been created, ERR_OK should be returned. Returning any
 * other error will provide a hint what went wrong, but except for assertions,
 * no real error handling is implemented.
 * 
 * @param mbox pointer to the mbox to create
 * @param size (minimum) number of messages in this mbox
 * @return ERR_OK if successful, another err_t otherwise
 */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("size > 0", size > 0);
    OS_ERR os_err;

    OSQCreate((OS_Q *)mbox,(void *)0,size,&os_err);
    if (os_err != OS_ERR_NONE) {
        return ERR_MEM;
    }

    return ERR_OK;
}
/**
 * @ingroup sys_mbox
 * Post a message to an mbox - may not fail
 * -> blocks if full, only to be used from tasks NOT from ISR!
 * 
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    OS_ERR os_err;

    OSQPost((OS_Q *)mbox,(void *)msg,sizeof(void *),OS_OPT_POST_FIFO,&os_err);
    LWIP_ASSERT("os_err == OS_ERR_NONE", os_err == OS_ERR_NONE);
}
/**
 * @ingroup sys_mbox
 * Try to post a message to an mbox - may fail if full.
 * Can be used from ISR (if the sys arch layer allows this).
 * Returns ERR_MEM if it is full, else, ERR_OK if the "msg" is posted.
 * 
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    OS_ERR os_err;

    OSQPost((OS_Q *)mbox,(void *)msg,sizeof(void *),OS_OPT_POST_FIFO,&os_err);
    if (os_err != OS_ERR_NONE) {
        return ERR_MEM;
    }

    return ERR_OK;
}
/**
 * @ingroup sys_mbox
 * Try to post a message to an mbox - may fail if full.
 * To be be used from ISR.
 * Returns ERR_MEM if it is full, else, ERR_OK if the "msg" is posted.
 * 
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL)
 */
err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    while (1)
    {
        /* 尚未实现, 目前好像没用到接口 */
    }
}
/**
 * @ingroup sys_mbox
 * Blocks the thread until a message arrives in the mailbox, but does
 * not block the thread longer than "timeout" milliseconds (similar to
 * the sys_arch_sem_wait() function). If "timeout" is 0, the thread should
 * be blocked until a message arrives. The "msg" argument is a result
 * parameter that is set by the function (i.e., by doing "*msg =
 * ptr"). The "msg" parameter maybe NULL to indicate that the message
 * should be dropped.
 * The return values are the same as for the sys_arch_sem_wait() function:
 * SYS_ARCH_TIMEOUT if there was a timeout, any other value if a messages
 * is received.
 * 
 * Note that a function with a similar name, sys_mbox_fetch(), is
 * implemented by lwIP. 
 * 
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message (0 = wait forever)
 * @return SYS_ARCH_TIMEOUT on timeout, any other value if a message has been received
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    OS_ERR os_err;
    void *msg_dummy;

    if (msg == NULL) {
        msg = &msg_dummy;
    }

    OS_MSG_SIZE msg_size = 0;
    OS_TICK tick = osKernelSysTickMicroSec(timeout);

    void *message = OSQPend((OS_Q *)mbox,tick,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&os_err);
    if ((message == NULL) || (os_err != OS_ERR_NONE)) {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }

    *msg = message;

/* Old versions of lwIP required us to return the time waited.
    This is not the case any more. Just returning != SYS_ARCH_TIMEOUT
    here is enough. */
    return 1;
}

/**
 * @ingroup sys_mbox
 * This is similar to sys_arch_mbox_fetch, however if a message is not
 * present in the mailbox, it immediately returns with the code
 * SYS_MBOX_EMPTY. On success 0 is returned.
 * To allow for efficient implementations, this can be defined as a
 * function-like macro in sys_arch.h instead of a normal function. For
 * example, a naive implementation could be:
 * \#define sys_arch_mbox_tryfetch(mbox,msg) sys_arch_mbox_fetch(mbox,msg,1)
 * although this would introduce unnecessary delays.
 * 
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @return 0 (milliseconds) if a message has been received
 *         or SYS_MBOX_EMPTY if the mailbox is empty
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    OS_ERR os_err;
    void *msg_dummy;

    if (msg == NULL) {
        msg = &msg_dummy;
    }

    OS_MSG_SIZE msg_size = 0;

    void *message = OSQPend((OS_Q *)mbox,0,OS_OPT_PEND_NON_BLOCKING,&msg_size,NULL,&os_err);
    /* 应该区分下错误和队列为空的情况? */
    if ((message == NULL) || (os_err != OS_ERR_NONE)) {
        *msg = NULL;
        return SYS_MBOX_EMPTY;
    }

    *msg = message;

    return 0;
}

/**
 * @ingroup sys_mbox
 * Deallocates a mailbox. If there are messages still present in the
 * mailbox when the mailbox is deallocated, it is an indication of a
 * programming error in lwIP and the developer should be notified.
 * 
 * @param mbox mbox to delete
 */
void sys_mbox_free(sys_mbox_t *mbox)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);

    OS_ERR os_err;
    OSQDel((OS_Q *)mbox,OS_OPT_DEL_ALWAYS,&os_err);
    LWIP_ASSERT("os_err == OS_ERR_NONE", os_err == OS_ERR_NONE);
}

