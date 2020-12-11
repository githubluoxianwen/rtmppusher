# �������� : make
# ���������� : make chip=xxx version=release
# ��������� : make clean chip=xxx version=release

SHELL = /bin/bash

######################################################
#make ��������
#debug release
version		= debug
plat		= hisi
chip		= hs_3520d
user		= 

######################################################
#����������
CCHIP		= $(chip)
LIB_PATH	= ../../../../../..
PROTAL_LIB_PATH = ../lib

ifeq ($(version), release)
	VERSION = release
else
	VERSION = debug
endif

#���� CCHIP ѡ�񻷾�
-include ../../../../../../Include/build_env

######################################################
#��Ŀ¼������Ҫ�����.cpp�ļ�
DIRS = .

#����ѡ��. ʾ��: -D__RJ_LINUX32__
CXXFLAGS := -D__WATCHDOG__

#����ͷ�ļ�. ʾ��: -I $(PROJ_DIR)/include
INCLUDES =  -I ../../../../../../Include -I ../../../../../../NVR/main -I ../../../../../../Include/rj_base  -I ../include/libaac -I ../include -I ../include/librtmp


#���õľ�̬��,�Լ���̬��Ŀ¼. ʾ��: -L ../lib_linux -luv
LIB_STATIC = -L $(libpath) -Bstatic -lrtmp -laac

#���õĶ�̬��,�Լ���̬��Ŀ¼. ʾ��: -L ../lib_linux -lrt -lpthread
LIB_DYNAMIC = -L $(libpath) -Bdynamic -ldl -lrt -lpthread


#����Ŀ������
TARGET_NAME = rtmpserver
TARGET		= ../lib/lib$(TARGET_NAME).a


#########################################################
## ������Ϣ���޸�
#########################################################

#����������
CC		= $(cc)
CXX		= $(cxx)
CAR		= $(car)
CRANLIB = $(cranlib)
CVERSION= $(version)
MAKE	 = make
RM		 = -rm -f
D2U    = dos2unix
CSH    = sh

#����ƽ̨��
CXXFLAGS += $(platform)

#���Ӷ��ⶨ��
CXXFLAGS += $(user) $(cxx_flags)

#�����汾����
ifeq ($(CVERSION), release)
	CXXFLAGS += -DNDEBUG
else
	CXXFLAGS += -g
endif

#�����ļ�
FIND_FILES_CPP = $(wildcard $(dir)/*.cpp)
SOURCES = $(foreach dir, $(DIRS), $(FIND_FILES_CPP))

LIB_L_OBJS = $(addsuffix .ol, $(SOURCES))

#���ϲ���
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