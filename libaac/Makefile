# 编译命令 : make
# 编译命令行 : make chip=xxx version=release
# 清除命令行 : make clean chip=xxx version=release

SHELL = /bin/bash

######################################################
#make 参数设置
#debug release
version		= debug
plat		= hisi
chip		= hs_3520d
user		= 

######################################################
#编译器环境
CCHIP		= $(chip)
LIB_PATH	= ../lib

ifeq ($(version), release)
	VERSION = release
else
	VERSION = debug
endif

#根据 CCHIP 选择环境
-include ../../../../../../Include/build_env

#libuv
cc += -std=gnu89

######################################################
#从目录检索需要编译的.c文件
DIRS = .

#libuv 在linux 下需要编译的文件
LINUX_APPEND_FILES = $(foreach dir, ./kiss_fft, $(FIND_FILES_CPP))

#用户自定义编译选项. 示例: -D__RJ_LINUX32__
#CXXFLAGS := -nostdlib -nostartfiles -fomit-frame-pointer -O2 -fvisibility=hidden -pedantic -Wall -Wextra -Wno-unused-parameter -D_GNU_SOURCE
CXXFLAGS := -D_GNU_SOURCE


#引用头文件. 示例: -I ../include
INCLUDES = -I ../include/librtmp -I  ./kiss_ffr

#引用的静态库,以及静态库目录. 示例: -L ../lib_linux -luv
LIB_STATIC = -L $(libpath) -Bstatic

#引用的动态库,以及动态库目录. 示例: -L ../lib_linux -lrt -lpthread
LIB_DYNAMIC = -L $(libpath) -lrt -lpthread

#编译目标名称
TARGET_NAME = aac
#TARGET		= $(LIB_PATH)/lib$(TARGET_NAME).a
TARGET		= ../lib/lib$(TARGET_NAME).a
#########################################################
## 以下信息勿修改
#########################################################

#编译器
CC		 = $(cc)
CXX		 = $(cxx)
CAR		 = $(car)
CRANLIB  = $(cranlib)
CVERSION = $(version)
MAKE	 = make
RM		 = -rm -f

#添加平台宏
CXXFLAGS += $(platform)

#添加额外定义
CXXFLAGS += $(user) $(cxx_flags)

#处理版本问题
ifeq ($(CVERSION), release)
	CXXFLAGS += -DNDEBUG
else
	CXXFLAGS += -g
endif

#编译文件
FIND_FILES_CPP = $(wildcard ./*.c)
SOURCES = $(FIND_FILES_CPP)
#SOURCES = $(foreach dir, $(DIRS), $(FIND_FILES_CPP))
#SOURCES += $(LINUX_APPEND_FILES)

LIB_L_OBJS = $(addsuffix .ol, $(SOURCES))
L_PARAMS   = $(CXXFLAGS) $(LIB_STATIC) $(LIB_DYNAMIC) $(INCLUDES)

.PHONY:all clean

all: $(TARGET)

%.c.ol: %.c
	$(CC) $(L_PARAMS) -c -o $@ $<

%.c.os: %.c
	$(CC) $(SO_PARAMS) -c -o $@ $<

$(TARGET): $(LIB_L_OBJS)
	$(tip_all)
	$(CAR) rs $(TARGET) $(LIB_L_OBJS)
	$(CRANLIB) $(TARGET)
	rm -rf *.ol

clean:
	@echo "++++++++++++++++ make clean ++++++++++++++"
	@echo "++ DIRS = $(DIRS)"
	@echo "++ RM = $(RM)"
	$(RM) $(LIB_L_OBJS)
	$(RM) $(TARGET)
	@echo "++++++++++++++++++++++++++++++++++++++++++"

define tip_all
	@echo "+++++++++++++ make $(TARGET) ++++++++++++++"
	@echo "++ CC = $(CC)"
	@echo "++ CXX = $(CXX)"
	@echo "++ plat = $(plat)"
	@echo "++ CVERSION = $(CVERSION)"
	@echo "++ DIRS = $(DIRS)"
	@echo "++ L_PARAMS = $(L_PARAMS)"
	@echo "++ SOURCES = $(notdir $(SOURCES))"
	@echo "++ TARGET = $(TARGET)"
	@echo "++++++++++++++++++++++++++++++++++++++++++"
endef
