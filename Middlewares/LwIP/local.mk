# 组件内部编译



# 头文件路径
C_INCLUDES 	+= Middlewares/RTOS/StdIf/Include	# 依赖的外部接口
C_INCLUDES 	+= Middlewares/LwIP/Config
C_INCLUDES 	+= Middlewares/LwIP/Ports/include
C_INCLUDES 	+= Middlewares/LwIP/lwip-2.1.3/src/include

# 源文件路径
C_SOURCES_DIRS 	+= Middlewares/LwIP/lwip-2.1.3/src/core
C_SOURCES_DIRS 	+= Middlewares/LwIP/lwip-2.1.3/src/core/ipv4

# 源文件列表
C_SOURCES 	+= Middlewares/LwIP/Ports/src/sys_arch.c

# 汇编文件列表
ASM_SOURCES +=
