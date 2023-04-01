
#include "cmsis_os.h"

static OS_MEM s_OsThreadTcb_MemHandle;
static OS_TCB s_OsThreadTcb_MemPool[osFeature_MaxThreadNum];

/* 线程栈内存池, 内存块大小取 OS_THREAD_STATCK_DEFAULT_SIZE */
#define OS_THREAD_STATCK_DEFAULT_SIZE       (64)  // 单位:字
static OS_MEM s_OsThreadStack_MemHandle;
static CPU_STK s_OsThreadStack_MemPool[osFeature_MaxThreadNum * OS_THREAD_STATCK_DEFAULT_SIZE];

static void StartThread(void const *argument);

/**
  * @brief  Initialize the RTOS Kernel for creating objects.
  * @retval status code that indicates the execution status of the function.
  * @note MUST REMAIN UNCHANGED: \b osKernelInitialize shall be consistent in every CMSIS-RTOS.
  */
osStatus osKernelInitialize(void)
{
    OS_ERR os_err;

    OSInit(&os_err);
    if (os_err != OS_ERR_NONE) {
        return osErrorOS;
    }

#if (OS_CFG_SCHED_ROUND_ROBIN_EN > 0u)
    /* 开启时间片功能, 时间片设置1个tick */
    OSSchedRoundRobinCfg(OS_TRUE,1,&os_err);
    if (os_err != OS_ERR_NONE) {
        return osErrorOS;
    }
#endif

#if (OS_CFG_APP_HOOKS_EN > 0u)
    App_OS_SetAllHooks();
#endif

    OSMemCreate(&s_OsThreadTcb_MemHandle,
                (CPU_CHAR   *)"OsThreadTcb",
                (void       *)&s_OsThreadTcb_MemPool[0],
                (OS_MEM_QTY  )sizeof(s_OsThreadTcb_MemPool)/sizeof(s_OsThreadTcb_MemPool[0]),
                (OS_MEM_SIZE )sizeof(s_OsThreadTcb_MemPool[0]),
                (OS_ERR     *)&os_err);
    if (os_err != OS_ERR_NONE) {
        return osErrorOS;
    }

    OSMemCreate(&s_OsThreadStack_MemHandle,
                (CPU_CHAR   *)"OsThreadStack",
                (void       *)&s_OsThreadStack_MemPool[0],
                (OS_MEM_QTY  )osFeature_MaxThreadNum,
                (OS_MEM_SIZE )(OS_THREAD_STATCK_DEFAULT_SIZE * sizeof(CPU_STK)),
                (OS_ERR     *)&os_err);
    if (os_err != OS_ERR_NONE) {
        return osErrorOS;
    }

    /* 创建一个最高优先级开始任务,用于初始化任务统计相关信息 */
    osThreadDef(StartThread,0,0,0);
    osThreadCreate(osThread(StartThread),NULL);

    return osOK;
}

/**
  * @brief  Start the RTOS Kernel.
  * @retval status code that indicates the execution status of the function.
  * @note MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
  */
osStatus osKernelStart(void)
{
    OS_ERR os_err;

    OSStart(&os_err);
    if (os_err != OS_ERR_NONE) {
        return osErrorOS;
    }

    return osOK;
}

/**
  * @brief  Check if the RTOS kernel is already started.
  * @retval 0 RTOS is not started, 1 RTOS is started.
  * @note MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
  */
int32_t osKernelRunning(void)
{
    return OSRunning;
}

/**
  * @brief  Create a thread and add it to Active Threads and set it to state READY.
  * @param[in]  thread_def: thread definition referenced with \ref osThread.
  * @param[in]  argument: pointer that is passed to the thread function as start argument.
  * @retval thread ID for reference by other functions or NULL in case of error.
  */
osThreadId osThreadCreate(const osThreadDef_t *thread_def, void *argument)
{
    OS_ERR os_err;

    OS_TCB *p_tcb = (OS_TCB *)OSMemGet((OS_MEM *)&s_OsThreadTcb_MemHandle,&os_err);
    if (p_tcb == NULL) {
        return NULL;
    }

    CPU_STK *p_stk_base = (CPU_STK *)OSMemGet((OS_MEM *)&s_OsThreadStack_MemHandle,&os_err);
    if (p_stk_base == NULL) {
        return NULL;
    }

    OSTaskCreate((OS_TCB *)p_tcb,
                (CPU_CHAR   *)"",
                (OS_TASK_PTR )thread_def->pthread,
                (void       *)argument,
                (OS_PRIO     )thread_def->tpriority,
                (CPU_STK    *)p_stk_base,
                (CPU_STK_SIZE)(OS_THREAD_STATCK_DEFAULT_SIZE)/10,
                (CPU_STK_SIZE)(OS_THREAD_STATCK_DEFAULT_SIZE),
                (OS_MSG_QTY  )0,
                (OS_TICK     )0,
                (void       *)0,
                (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR     *)&os_err);
    if (os_err != OS_ERR_NONE) {
        return NULL;
    }

    return p_tcb;
}

/**
  * @brief  Return the thread ID of the current running thread.
  * @param[in]  thread_id: thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
  * @retval thread ID for reference by other functions or NULL in case of error.
  * @note MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
  */
osThreadId osThreadGetId(void)
{
    return (osThreadId)OSTCBCurPtr;
}

/**
  * @brief  Terminate execution of a thread and remove it from Active Threads.
  * @param[in]  thread_id: thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
  * @retval status code that indicates the execution status of the function.
  * @note MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
  */
osStatus osThreadTerminate(osThreadId thread_id)
{
    OS_ERR os_err;

    OS_TCB *p_tcb = (OS_TCB *)thread_id;

    OSTaskDel(p_tcb,&os_err);

    OSMemPut((OS_MEM  *)&s_OsThreadStack_MemHandle,(void *)p_tcb->StkBasePtr,&os_err);
    OSMemPut((OS_MEM  *)&s_OsThreadTcb_MemHandle,(void *)p_tcb,&os_err);

    return osOK;
}

static void StartThread(void const *argument)
{
    (void)argument;
    OS_ERR os_err;

    CPU_Init();
    OS_CPU_SysTickInitFreq((CPU_INT32U)osSystemCoreFrequency);

#if OS_CFG_STAT_TASK_EN == DEF_ENABLED
    OSStatTaskCPUUsageInit(&os_err); /* Compute CPU capacity with no task running */
    OSStatReset(&os_err);
#endif

    OS_TRACE_INIT();


    
    OSTaskDel(NULL,&os_err);
}

