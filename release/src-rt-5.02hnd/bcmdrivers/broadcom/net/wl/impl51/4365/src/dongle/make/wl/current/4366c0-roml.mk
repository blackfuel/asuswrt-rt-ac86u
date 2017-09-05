#
# Makefile for hndrte based 4366c0 ROM Offload image building
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4366c0-roml.mk$
#

####################################################################################################
# This makefile is used when building a ROM offload image, so an image that runs in RAM and calls
# routines that reside in ROM. It is not used when building a ROM. Default settings are defined in
# the 'wlconfig', settings in there may be redefined in this file when a 'default' ROM offload image
# should support more or less features than the ROM.
####################################################################################################

# Reuse the makefiles of 4365C0
include $(TOPDIR)/current/4365c0-roml.mk
CHIP := 4366
ROMLDIR := $(TOPDIR)/../../../../components/chips/images/roml/4365c0
ROMLVRTSDIR := $(TOPDIR)/../../../dongle/roml/4365c0
ifeq ($(call opt,fdap),1)
	#router image, enable router specific features
	CLM_TYPE	:= 4366c0_access
else
	# CLM info
	CLM_TYPE	:= 4366c0
endif
