# -*- Makefile -*-
#########################################################################
# @file   platform_arm_linux.GNU
#
# $Id: platform_arm_linux.GNU $
#
# @author Chong Wuk Pak <chong.pak@lmco.com>
#
# This file contains the contents of the platform_arm_linux.GNU in order 
# to build ACE+TAO for ARM processor using the cross platform toolchain 
# that is built from buildroot uclibC.
#########################################################################

debug ?= 1
optimize ?= 1
threads ?= 1
versioned_so ?= 1

cross_compile ?= 1
debug ?= 0
optimize ?= 1
inline ?= 0
ssl ?= 0
zlib ?= 1
boost ?= 0
no_hidden_visibility ?= 1

FLAGS_C_CC += -Wno-psabi

CROSS_COMPILE=$(ARM_ROOT)/bin/arm-linux-
include $(ACE_ROOT)/include/makeinclude/platform_linux.GNU
