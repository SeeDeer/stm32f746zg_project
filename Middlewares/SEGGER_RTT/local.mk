
# 组件内部编译
C_SOURCES 	+= Middlewares/SEGGER_RTT/RTT/SEGGER_RTT.c
C_SOURCES 	+= Middlewares/SEGGER_RTT/RTT/SEGGER_RTT_printf.c
C_SOURCES 	+= Middlewares/SEGGER_RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c
C_INCLUDES 	+= Middlewares/SEGGER_RTT/Config
C_INCLUDES 	+= Middlewares/SEGGER_RTT/RTT
ASM_SOURCES += Middlewares/SEGGER_RTT/RTT/SEGGER_RTT_ASM_ARMv7M.s
