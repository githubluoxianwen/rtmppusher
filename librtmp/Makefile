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
LIB_PATH	= ../lib

ifeq ($(version), release)
	VERSION = release
else
	VERSION = debug
endif

#���� CCHIP ѡ�񻷾�
-include ../../../../../../Include/build_env

#libuv
cc += -std=gnu89

######################################################
#��Ŀ¼������Ҫ�����.c�ļ�
DIRS = .

#librtmp ��linux ����Ҫ������ļ�
#LINUX_APPEND_FILES = $(foreach dir, ., $(FIND_FILES_CPP))

#�û��Զ������ѡ��. ʾ��: -D__RJ_LINUX32__
#CXXFLAGS := -nostdlib -nostartfiles -fomit-frame-pointer -O2 -fvisibility=hidden -pedantic -Wall -Wextra -Wno-unused-parameter -D_GNU_SOURCE
CXXFLAGS := -D_GNU_SOURCE


#����ͷ�ļ�. ʾ��: -I ../include
INCLUDES = -I ../include/librtmp -I ../../../../../../Include

#���õľ�̬��,�Լ���̬��Ŀ¼. ʾ��: -L ../lib_linux -luv
LIB_STATIC = -L $(libpath) -Bstatic -lcrypto -lssl

#���õĶ�̬��,�Լ���̬��Ŀ¼. ʾ��: -L ../lib_linux -lrt -lpthread
LIB_DYNAMIC = -L $(libpath) -lrt -lpthread 

#����Ŀ������
TARGET_NAME = rtmp
TARGET		= ../lib/lib$(TARGET_NAME).a

#########################################################
## ������Ϣ���޸�
#########################################################

#������
CC		 = $(cc)
CXX		 = $(cxx)
CAR		 = $(car)
CRANLIB  = $(cranlib)
CVERSION = $(version)
MAKE	 = make
RM		 = -rm -f

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