##############################################################################
# Product: Makefile for Project
# Date of the Last Update:  November 1, 2012
#
#                  QP_LWIP_STM32F2xx_eth_DPP_Example 
#                    ---------------------------
#
#
##############################################################################
# examples of invoking this Makefile:
# building configurations: Debug (default), Release, and Spy
# make
# make CONF=rel
# make CONF=spy
#
# cleaning configurations: Debug (default), Release, and Spy
# make clean
# make CONF=rel clean
# make CONF=spy clean

# Output file basename (should be the same as the directory name)
PROJECT_NAME			= QP_LWIP_STM32F2xx_eth_DPP_Example

#------------------------------------------------------------------------------
#  TOOLCHAIN SETUP
#------------------------------------------------------------------------------
CROSS    				= arm-none-eabi-
CC       				= $(CROSS)gcc
CPP      				= $(CROSS)g++
AS       				= $(CROSS)as
LINK     				= $(CROSS)gcc
OBJCPY   				= ${CROSS}objcopy
RM       				= rm -rf
ECHO     				= echo
MKDIR    				= mkdir

#-----------------------------------------------------------------------------
# Directories
#
#-----------------------------------------------------------------------------
SRC_DIR     			= ./Source

# BSP Directories
BSP_DIR					= $(SRC_DIR)/bsp
CMSIS_DIR				= $(BSP_DIR)/CMSIS/Device/ST/STM32F2xx/Include
CMSIS_INC				= $(BSP_DIR)/CMSIS/Include

# STM32 Drivers
COMMON_DIR				= $(BSP_DIR)/Common
STM3220_EVAL_DIR		= $(BSP_DIR)/stm322xg_eval
STM32F2XX_STD_PERIPH	= $(BSP_DIR)/STM32F2xx_StdPeriph_Driver

# Ethernet Driver
QP_LWIP_PORT_DIR		= $(BSP_DIR)/lwip_port
ETH_DRV_DIR				= $(BSP_DIR)/stm3220_eth_driver

# Directories that need to be passed down to LWIP
LWIP_PORT_FOR_LWIP		= ../bsp/lwip_port
BSP_DIR_FOR_LWIP		= ../

# QPC directories
QPC 					= $(SRC_DIR)/qpc
QP_PORT_DIR 			= $(QPC)/ports/arm-cortex/qk/gnu

# LWIP directories
LWIP_DIR    			= $(SRC_DIR)/lwip

# Webserver
WEBSERVER_DIR			= $(SRC_DIR)/webserver
FS_DIR					= $(WEBSERVER_DIR)/fs


# Source virtual directories
VPATH 					= $(SRC_DIR) \
						  $(BSP_DIR) \
						  \
						  $(COMMON_DIR) \
						  $(ETH_DRV_DIR)/src \
						  $(STM3220_EVAL_DIR) \
						  \
						  $(QP_LWIP_PORT_DIR)/netif \
						  \
						  $(SRC_DIR)/runtime \
						  \
						  $(STM32F2XX_STD_PERIPH)/src \
						  \
						  $(WEBSERVER_DIR) \
						  $(FS_DIR) 

# include directories
INCLUDES  				= -I$(SRC_DIR) \
						  \
						  -I$(LWIP_DIR)/src/include \
						  -I$(LWIP_DIR)/src/include/netif \
						  -I$(LWIP_DIR)/src/include/lwip \
						  -I$(LWIP_DIR)/src/include/ipv4 \
						  \
						  -I$(QPC)/include \
						  -I$(QP_PORT_DIR) \
						  \
						  -I$(BSP_DIR) \
						  \
						  -I$(ETH_DRV_DIR)/inc \
						  \
						  -I$(CMSIS_DIR) \
						  -I$(CMSIS_INC) \
						  \
						  -I$(STM3220_EVAL_DIR) \
						  -I$(COMMON_DIR) \
						  \
						  -I$(QP_LWIP_PORT_DIR)/ \
						  -I$(QP_LWIP_PORT_DIR)/arch \
						  -I$(QP_LWIP_PORT_DIR)/netif \
						  \
						  -I$(STM32F2XX_STD_PERIPH)/inc \
						   \
						  -I$(WEBSERVER_DIR) \
						  -I$(FS_DIR) 

# defines
DEFINES   				= -DUSE_STM322xG_EVAL \
						  -DUSE_STDPERIPH_DRIVER

#-----------------------------------------------------------------------------
# files
#

# assembler source files
ASM_SRCS 				:= \
						startup_stm32f2xx.S \
						memcpy.S \
						stm32_hardfault_handler.S

# C source files
C_SRCS 					:= \
			      		main.c \
			      		stm32f2xx_it.c \
			      		system_stm32f2xx.c \
			      		no_heap.c \
			      		lwipmgr.c \
					    lwip.c \
					    philo.c \
					    table.c \
					    \
					    lcd_log.c \
					    stm322xg_eval.c \
					    stm322xg_eval_lcd.c \
					    fonts.c \
					    \
					    eth_driver.c \
					    stm32f2x7_eth.c \
				  		stm32f2x7_eth_bsp.c \
			      		\
			      		bsp.c \
						\
			      		syscalls.c \
						\
			      		misc.c  \
			      		stm32f2xx_adc.c  \
			      		stm32f2xx_can.c \
			      		stm32f2xx_crc.c \
			      		stm32f2xx_cryp.c \
			      		stm32f2xx_cryp_aes.c \
			      		stm32f2xx_cryp_des.c \
			      		stm32f2xx_cryp_tdes.c \
			      		stm32f2xx_dac.c \
			      		stm32f2xx_dma.c \
			      		stm32f2xx_exti.c \
			      		stm32f2xx_dbgmcu.c \
			      		stm32f2xx_flash.c \
			      		stm32f2xx_fsmc.c \
			      		stm32f2xx_gpio.c \
			      		stm32f2xx_hash.c \
			      		stm32f2xx_hash_md5.c \
			      		stm32f2xx_hash_sha1.c \
			      		stm32f2xx_i2c.c \
			      		stm32f2xx_iwdg.c \
			      		stm32f2xx_pwr.c \
			      		stm32f2xx_rng.c \
			      		stm32f2xx_rtc.c \
			      		stm32f2xx_sdio.c \
			      		stm32f2xx_spi.c \
			      		stm32f2xx_syscfg.c \
			      		stm32f2xx_tim.c \
			      		stm32f2xx_usart.c \
			      		stm32f2xx_rcc.c \
			      		stm32f2xx_wwdg.c \
			      		\
			      		fs.c \
			      		httpd.c

# C++ source files
CPP_SRCS 				:=	$(wildcard *.cpp)


LD_SCRIPT 				:= $(SRC_DIR)/linkerscript/stm32f207flash.ld
LDFLAGS  				:= -T$(LDSCRIPT)

#-----------------------------------------------------------------------------
# build options for various configurations
#

ARM_CORE 	= cortex-m3
LIBS    	:= -lqp_$(ARM_CORE)_cs -llwip_$(ARM_CORE)_cs

ifeq (rel, $(CONF))       # Release configuration ............................

BIN_DIR 	:= rel
DEFINES		+= -DNDEBUG

ASFLAGS 	= -mthumb -mcpu=$(ARM_CORE)
CFLAGS 		= -mcpu=$(ARM_CORE) -mthumb -Wall -ffunction-sections -fdata-sections\
			  -Os $(INCLUDES) $(DEFINES)

CPPFLAGS 	= -mcpu=$(ARM_CORE) -mthumb \
			  -Wall -fno-rtti -fno-exceptions \
			  -Os $(INCLUDES) $(DEFINES)

LINKFLAGS 	= -nodefaultlibs -Xlinker --gc-sections -Wl,-Map,$(BIN_DIR)/$(PROJECT_NAME).map -mcpu=$(ARM_CORE) -mthumb

else ifeq (spy, $(CONF))  # Spy configuration ................................

BIN_DIR 	:= spy
DEFINES		+= -DQ_SPY
ASFLAGS 	= -g -mthumb -mcpu=$(ARM_CORE)
CFLAGS 		= -mcpu=$(ARM_CORE) -mthumb -Wall -Os \
			  -g -O $(INCLUDES) $(DEFINES)

CPPFLAGS 	= -mcpu=$(ARM_CORE) -mthumb \
			  -Wall -fno-rtti -fno-exceptions \
			  -g -O $(INCLUDES) $(DEFINES)

LINKFLAGS 	= -nodefaultlibs -Xlinker --gc-sections -Wl,-Map,$(BIN_DIR)/$(PROJECT_NAME).map -mcpu=$(ARM_CORE) -mthumb -g3 -gdwarf-2

else                     # default Debug configuration .......................

BIN_DIR 	:= dbg
ASFLAGS 	= -g -mthumb -mcpu=$(ARM_CORE)
CFLAGS 		= -mcpu=$(ARM_CORE) -mthumb -Wall -Os \
			  -g -O $(INCLUDES) $(DEFINES)

CPPFLAGS 	= -mcpu=$(ARM_CORE) -mthumb \
			  -Wall -fno-rtti -fno-exceptions \
			  -g -O $(INCLUDES) $(DEFINES)
	
LINKFLAGS 	= -nodefaultlibs -Xlinker --gc-sections -Wl,-Map,$(BIN_DIR)/$(PROJECT_NAME).map -mcpu=$(ARM_CORE) -mthumb -g3 -gdwarf-2

endif


ASM_OBJS     = $(patsubst %.S, $(BIN_DIR)/%.o, $(ASM_SRCS))
C_OBJS       = $(patsubst %.c, $(BIN_DIR)/%.o, $(C_SRCS))
#CPP_OBJS     = $(patsubst %.cpp, $(BIN_DIR)/%.o, $(CPP_SRCS))

TARGET_BIN   = $(BIN_DIR)/$(PROJECT_NAME).hex
TARGET_ELF   = $(BIN_DIR)/$(PROJECT_NAME).elf
C_DEPS       = $(patsubst %.o, %.d, $(C_OBJS))
#CPP_DEPS     = $(patsubst %.o, %.d, $(CPP_OBJS))


#-----------------------------------------------------------------------------
# rules
#

all: $(BIN_DIR) build_libs $(TARGET_BIN)

$(BIN_DIR):
	@echo ---------------------------
	@echo --- Creating directory ---
	@echo ---------------------------
	mkdir -p $@

$(TARGET_BIN): $(TARGET_ELF)
	@echo ---------------------------
	@echo --- Creating the binary ---
	@echo ---------------------------
	$(OBJCPY) -O ihex $< $@

$(TARGET_ELF) : $(ASM_OBJS) $(C_OBJS) #$(CPP_OBJS)
	@echo ---------------------------
	@echo --- Linking libraries   ---
	@echo ---------------------------
	$(LINK) -T$(LD_SCRIPT) -$(LINKFLAGS) -L$(QP_PORT_DIR)/$(BIN_DIR) -L$(LWIP_DIR)/$(BIN_DIR) -o $@ $^ $(LIBS)
	
build_libs: build_qpc build_lwip

build_qpc:
	@echo ---------------------------
	@echo --- Building QPC libraries ---
	@echo ---------------------------
	cd $(QP_PORT_DIR); make CONF=$(BIN_DIR)

build_lwip:
	@echo ---------------------------
	@echo --- Building LWIP libraries ---
	@echo ----Passing in current proj directory
	#TODO: The passed in directories must be relative to the LWIP library, not this project
	cd $(LWIP_DIR); make LWIP_PORT_DIR=$(LWIP_PORT_FOR_LWIP) PROJ_BSP=$(BSP_DIR_FOR_LWIP)

$(BIN_DIR)/%.d : %.c
	$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $< > $@

$(BIN_DIR)/%.d : %.cpp
	$(CPP) -MM -MT $(@:.d=.o) $(CPPFLAGS) $< > $@

$(BIN_DIR)/%.o : %.S
	@echo ---------------------------
	@echo --- Compiling ASM object ---
	@echo ---------------------------
	$(AS) $(ASFLAGS) $< -o $@

$(BIN_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

-include $(C_DEPS) $(CPP_DEPS)


.PHONY : clean clean_with_libs
cleanall:
	@echo ---------------------------
	@echo --- Cleaning EVERYTHING
	@echo ---------------------------
	cd $(QP_PORT_DIR); make CONF=dbg clean
	cd $(QP_PORT_DIR); make CONF=spy clean
	cd $(QP_PORT_DIR); make CONF=rel clean
	cd $(LWIP_DIR); make LWIP_PORT_DIR=$(LWIP_PORT_FOR_LWIP) PROJ_BSP=$(BSP_DIR_FOR_LWIP) CONF=dbg clean
	cd $(LWIP_DIR); make LWIP_PORT_DIR=$(LWIP_PORT_FOR_LWIP) PROJ_BSP=$(BSP_DIR_FOR_LWIP) CONF=spy clean
	cd $(LWIP_DIR); make LWIP_PORT_DIR=$(LWIP_PORT_FOR_LWIP) PROJ_BSP=$(BSP_DIR_FOR_LWIP) CONF=rel clean
	-$(RM) dbg/*.o dbg/*.d dbg/*.hex dbg/*.elf dbg/*.map dbg
	-$(RM) spy/*.o spy/*.d spy/*.hex spy/*.elf spy/*.map rel
	-$(RM) rel/*.o rel/*.d rel/*.hex rel/*.elf rel/*.map spy
	
clean:
	-$(RM) $(BIN_DIR)/*.o \
	$(BIN_DIR)/*.d \
	$(BIN_DIR)/*.hex \
	$(BIN_DIR)/*.elf \
	$(BIN_DIR)/*.map
	
clean_with_libs: clean clean_qpc_libs clean_lwip_libs
	@echo ---------------------------
	@echo --- Cleaned All libs and bins
	@echo ---------------------------
	
clean_qpc_libs:
	@echo ---------------------------
	@echo --- Cleaning QPC libraries
	@echo ---------------------------
	cd $(QP_PORT_DIR); make CONF=$(BIN_DIR) clean

clean_lwip_libs:
	@echo ---------------------------
	@echo --- Cleaning LWIP libraries
	@echo ---------------------------
	cd $(LWIP_DIR); make LWIP_PORT_DIR=$(LWIP_PORT_FOR_LWIP) PROJ_BSP=$(BSP_DIR_FOR_LWIP) CONF=$(BIN_DIR) clean

show:
	@echo CONF = $(CONF)
	@echo ASM_SRCS = $(ASM_SRCS)
	@echo C_SRCS = $(C_SRCS)
	@echo CPP_SRCS = $(CPP_SRCS)
	@echo ASM_OBJS = $(ASM_OBJS)
	@echo C_OBJS = $(C_OBJS)
	@echo C_DEPS = $(C_DEPS)
	@echo CPP_DEPS = $(CPP_DEPS)
	@echo TARGET_ELF = $(TARGET_ELF)
	@echo LIBS = $(LIBS)
	@echo INCLUDES = $(INCLUDES)

