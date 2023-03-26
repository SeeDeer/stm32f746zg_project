
#include "os.h"
#include "os_app_hooks.h"
#include "stm32f7xx.h"

#define START_TASK_PRIO         (0)
#define TEST1_TASK_PRIO         (15)
#define TEST2_TASK_PRIO         (15)
#define TEST3_TASK_PRIO         (15)

static  OS_TCB      s_Start_Task_TCB;
static  OS_TCB      s_Test1_Task_TCB;
static  OS_TCB      s_Test2_Task_TCB;
static  OS_TCB      s_Test3_Task_TCB;

static  CPU_STK     s_Start_Task_Stk[128];
static  CPU_STK     s_Test1_Task_Stk[128];
static  CPU_STK     s_Test2_Task_Stk[128];
static  CPU_STK     s_Test3_Task_Stk[128];

extern uint32_t SystemCoreClock;

static void Start_Task_Func(void *p_arg);
extern void Test1_Task_Func(void *p_arg);
extern void Test2_Task_Func(void *p_arg);
extern void Test3_Task_Func(void *p_arg);

void UCOS_III_Init(void)
{
    OS_ERR os_err;

    OSInit(&os_err);
    if (os_err != OS_ERR_NONE) {
        return;
    }

#if (OS_CFG_SCHED_ROUND_ROBIN_EN > 0u)
    /* 开启时间片功能, 时间片设置1个tick */
    OSSchedRoundRobinCfg(OS_TRUE,1,&os_err);
#endif

    App_OS_SetAllHooks();

    OSTaskCreate((OS_TCB     *)&s_Start_Task_TCB,
                 (CPU_CHAR   *)"Start",
                 (OS_TASK_PTR )Start_Task_Func,
                 (void       *)0,
                 (OS_PRIO     )START_TASK_PRIO,
                 (CPU_STK    *)&s_Start_Task_Stk[0],
                 (CPU_STK_SIZE)sizeof(s_Start_Task_Stk)/sizeof(s_Start_Task_Stk[0]) / 10,
                 (CPU_STK_SIZE)sizeof(s_Start_Task_Stk)/sizeof(s_Start_Task_Stk[0]),
                 (OS_MSG_QTY  )0,                                                 
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&os_err);
    
    if (os_err != OS_ERR_NONE) {
        return;
    }

    OSStart(&os_err);
    
    while(1);   /* 系统正常启动后，永远运行不到这里 */
}

static void Start_Task_Func(void *p_arg)
{
    (void)p_arg;
    OS_ERR os_err;

    CPU_Init();
    OS_CPU_SysTickInitFreq((CPU_INT32U)SystemCoreClock);

#if OS_CFG_STAT_TASK_EN == DEF_ENABLED
    OSStatTaskCPUUsageInit(&os_err); /* Compute CPU capacity with no task running */
    OSStatReset(&os_err);
#endif

    OS_TRACE_INIT();

    /* 创建其他用户任务 */
    OSTaskCreate((OS_TCB     *)&s_Test1_Task_TCB,
                (CPU_CHAR   *)"Test1",
                (OS_TASK_PTR )Test1_Task_Func,
                (void       *)0,
                (OS_PRIO     )TEST1_TASK_PRIO,
                (CPU_STK    *)&s_Test1_Task_Stk[0],
                (CPU_STK_SIZE)sizeof(s_Test1_Task_Stk)/sizeof(s_Test1_Task_Stk[0]) / 10,
                (CPU_STK_SIZE)sizeof(s_Test1_Task_Stk)/sizeof(s_Test1_Task_Stk[0]),
                (OS_MSG_QTY  )0,                                                 
                (OS_TICK     )0,
                (void       *)0,
                (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR     *)&os_err);

    OSTaskCreate((OS_TCB     *)&s_Test2_Task_TCB,
                (CPU_CHAR   *)"Test2",
                (OS_TASK_PTR )Test2_Task_Func,
                (void       *)0,
                (OS_PRIO     )TEST2_TASK_PRIO,
                (CPU_STK    *)&s_Test2_Task_Stk[0],
                (CPU_STK_SIZE)sizeof(s_Test2_Task_Stk)/sizeof(s_Test2_Task_Stk[0]) / 10,
                (CPU_STK_SIZE)sizeof(s_Test2_Task_Stk)/sizeof(s_Test2_Task_Stk[0]),
                (OS_MSG_QTY  )0,                                                 
                (OS_TICK     )0,
                (void       *)0,
                (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR     *)&os_err);
    
    OSTaskCreate((OS_TCB     *)&s_Test3_Task_TCB,
                (CPU_CHAR   *)"Test3",
                (OS_TASK_PTR )Test3_Task_Func,
                (void       *)0,
                (OS_PRIO     )TEST3_TASK_PRIO,
                (CPU_STK    *)&s_Test3_Task_Stk[0],
                (CPU_STK_SIZE)sizeof(s_Test3_Task_Stk)/sizeof(s_Test3_Task_Stk[0]) / 10,
                (CPU_STK_SIZE)sizeof(s_Test3_Task_Stk)/sizeof(s_Test3_Task_Stk[0]),
                (OS_MSG_QTY  )0,                                                 
                (OS_TICK     )0,
                (void       *)0,
                (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR     *)&os_err);

}

