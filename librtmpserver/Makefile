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
LIB_PATH	= ../../../../../..
PROTAL_LIB_PATH = ../lib

ifeq ($(version), release)
	VERSION = release
else
	VERSION = debug
endif

#根据 CCHIP 选择环境
-include ../../../../../../Include/build_env

######################################################
#从目录检索需要编译的.cpp文件
DIRS = .

#编译选项. 示例: -D__RJ_LINUX32__
CXXFLAGS := -D__WATCHDOG__

#引用头文件. 示例: -I $(PROJ_DIR)/include
INCLUDES =  -I ../../../../../../Include -I ../../../../../../NVR/main -I ../../../../../../Include/rj_base  -I ../include/libaac -I ../include -I ../include/librtmp


#引用的静态库,以及静态库目录. 示例: -L ../lib_linux -luv
LIB_STATIC = -L $(libpath) -Bstatic -lrtmp -laac

#引用的动态库,以及动态库目录. 示例: -L ../lib_linux -lrt -lpthread
LIB_DYNAMIC = -L $(libpath) -Bdynamic -ldl -lrt -lpthread


#编译目标名称
TARGET_NAME = rtmpserver
TARGET		= ../lib/lib$(TARGET_NAME).a


#########################################################
## 以下信息勿修改
#########################################################

#编译器参数
CC		= $(cc)
CXX		= $(cxx)
CAR		= $(car)
CRANLIB = $(cranlib)
CVERSION= $(version)
MAKE	 = make
RM		 = -rm -f
D2U    = dos2unix
CSH    = sh

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
FIND_FILES_CPP = $(wildcard $(dir)/*.cpp)
SOURCES = $(foreach dir, $(DIRS), $(FIND_FILES_CPP))

LIB_L_OBJS = $(addsuffix .ol, $(SOURCES))

#整合参数
L_PARAMS = $(CXXFLAGS) $(LIB_STATIC) $(SDK_LIB) $(LIB_DYNAMIC) $(INCLUDES)

.PHONY:all clean

all: $(TARGET) depends $(LIB_L_OBJS)

%.cpp.ol: %.cpp
	$(CXX) $(L_PARAMS) -c -o $@ $<

%.cpp.os: %.cpp
	$(CXX) $(SO_PARAMS) -c -o $@ $<
depends:
	if [ -d ../libaac ]; then $(MAKE) -C ../libaac; fi
	if [ -d ../librtmp ]; then $(MAKE) -C ../librtmp; fi

$(TARGET): depends $(LIB_L_OBJS)
	$(tip_all)
	$(CAR) rs $(TARGET) $(LIB_L_OBJS)
	rm -rf *.ol
	
clean:
	if [ -d ../libaac ]; then $(MAKE) -C ../libaac clean; fi
	if [ -d ../librtmp ]; then $(MAKE) -C ../librtmp clean; fi

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
