# Makefile for hndrte based 43xx standalone programs,
#	to generate romtable.S for 43xx ROM builds
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 43xx.mk 254324 2011-04-20 06:12:25Z $:

# chip specification
CHIP		:= 4334
REV		:= a0
REVID		:= 0

# Note on rebuilding romtable.S
#	make 43xx
#	cp builds/43xx/sdio/romtable_full.S ../roml/43xx/romtable.S
TARGETS		:= sdio

# common target attributes
TARGET_HBUS	:= sdio usb
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
THUMB		:= 1
HBUS_PROTO 	:= cdc
BAND		:= ag

# wlconfig & wltunable
WLCONFFILE	:= wlconfig_rte_43xx_dev
WLTUNEFILE	:= wltunable_rte_4334a0.h

# ROMCTL needed for location of romctl.txt
ROMCTL		:= $(TOPDIR)/../roml/4334sim-43xx/romctl.txt

# features (sync with 43xx.mk, 4334a0-romlsim-43xx.mk)
SMALL		:= 0
MEMSIZE		:= 524288	# Hardcoding it saves ~112 bytes from startarm.S
FLASH		:= 0
MFGTEST		:= 1
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ERROR	:= 1
BCMPKTPOOL	:= 1
DMATXRC		:= 1
WLRXOV		:= 1
PROP_TXSTATUS	:= 1

POOL_LEN_MAX	:= 60

# p2p dongle code support
VDEV		:= 1

# extra flags
EXTRA_DFLAGS	+= -DSHARE_RIJNDAEL_SBOX	# Save 1400 bytes; wep & tkhash slightly slower
EXTRA_DFLAGS	+= -DWLAMSDU_TX -DAMPDU_RX_BA_DEF_WSIZE=16
ifneq ($(ROMLIB),1)
ROMBUILD	:= 1
EXTRA_DFLAGS	+= -DBCMROMSYMGEN_BUILD
endif
