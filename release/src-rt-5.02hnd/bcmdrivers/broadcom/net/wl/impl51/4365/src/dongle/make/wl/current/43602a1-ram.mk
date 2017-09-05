# Makefile for hndrte based 43602a1 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2016,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$

# chip specification
CHIP		:= 43602
REV		:= a1
REVID		:= 1

# default targets
TARGETS		:= \
	pcie-ag-assert-err-splitrx \
	pcie-ag-mfgtest-seqcmds-splitrx

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= pcie
THUMB		:= 1
HBUS_PROTO	:= msgbuf
NODIS		:= 0

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43602a1_bu
WLTUNEFILE	:= wltunable_rte_43602a1.h

TOOLSVER	:= 2011.09

# 0x0018_0000 is start of RAM
TEXT_START	:= 0x00180000

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (pci)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))

# 43602 has 512KB ATCM (instructions+data) + 448KB BxTCM (data) = 960KB = 983040 bytes
MEMBASE		:= 0x00180000
MEMSIZE		:= 983040
MFGTEST		:= 0
WLTINYDUMP	:= 0
# Enabling DBG_ASSERT would have an adverse effect on throughput.
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 0
WLRXOV		:= 0

HNDLBUFCOMPACT	:= 1
BCMPKTIDMAP	:= 1
BCMFRAGPOOL	:= 1
BCMLFRAG	:= 1
BCMSPLITRX	:= 1

POOL_LEN_MAX    := 200
POOL_LEN        := 6
WL_POST_FIFO1   := 2
MFGTESTPOOL_LEN := 10
FRAG_POOL_LEN	:= 200
RXFRAG_POOL_LEN	:= 160

# To make NcSim possible
#INTERNAL	:= 1

PCIE_NTXD	:= 256
PCIE_NRXD	:= 256

ifeq ($(findstring mfgtest,$(TARGET)),mfgtest)
	BCMNVRAMR	:= 0
	BCMNVRAMW	:= 1
endif

ifeq ($(findstring fdap,$(TARGET)),fdap)
	#router image, enable router specific features
	CLM_TYPE			:= 43602a1_access

	AP				:= 1
	SUFFIX_ENAB			:= 1
	INITDATA_EMBED			:= 1
	WLC_DISABLE_DFS_RADAR_SUPPORT	:= 0
	# Max MBSS virtual slave devices
	MBSS_MAXSLAVES			:= 4
	# Max Tx Flowrings
	PCIE_TXFLOWS			:= 132

	# Reduce packet pool lens for internal assert
	# builds to fix out of memory issues
	ifeq ($(findstring assert,$(TARGET)),assert)
		FRAG_POOL_LEN	:= 160
		RXFRAG_POOL_LEN	:= 160
	endif
	WLBSSLOAD			:= 1
	WLOSEN				:= 1
	WLPROBRESP_MAC_FILTER		:= 1

	EXTRA_DFLAGS	+= -DPKTC_FDAP

	# Reduce above POOLs size to make RAM dongle firmware runable.
	POOL_LEN_MAX    := 128
	FRAG_POOL_LEN	:= 128
	RXFRAG_POOL_LEN	:= 128
	H2D_DMAQ_LEN	:= 128
	D2H_DMAQ_LEN	:= 128
	PCIE_NTXD	:= 128
	PCIE_NRXD	:= 128

	#Turn on 16bit indices by default
	BCMDMAINDEX16 := 1
else
	# CLM info
	CLM_TYPE			:= 43602a1

	#Turn on 32bit indices by default
	BCMDMAINDEX32 := 1
endif

BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0

# Reduce stack size to increase free heap
HND_STACK_SIZE	:= 4608
EXTRA_DFLAGS	+= -DHND_STACK_SIZE=$(HND_STACK_SIZE)
EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Add flops support
FLOPS_SUPPORT	:= 1

EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)
# Only support EPA
EXTRA_DFLAGS	+= -DWLPHY_EPA_ONLY -DEPA_SUPPORT=1
# Don't support PAPD
EXTRA_DFLAGS	+= -DEPAPD_SUPPORT=0 -DWLC_DISABLE_PAPD_SUPPORT -DPAPD_SUPPORT=0

EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=32
EXTRA_DFLAGS	+= -DPKTC_DONGLE
EXTRA_DFLAGS	+= -DPCIEDEV_USE_EXT_BUF_FOR_IOCTL
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

# RxOffsets for the PCIE mem2mem DMA
EXTRA_DFLAGS    += -DH2D_PD_RX_OFFSET=0
EXTRA_DFLAGS    += -DD2H_PD_RX_OFFSET=0

#Support for SROM format
EXTRA_DFLAGS	+= -DBCMPCIEDEV_SROM_FORMAT

#Turn on 32bit indices by default
BCMDMAINDEX32 := 1
