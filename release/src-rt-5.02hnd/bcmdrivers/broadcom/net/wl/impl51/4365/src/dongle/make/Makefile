#
# Makefile for standalone programs based on rte
#
# Broadcom Proprietary and Confidential. Copyright (C) 2016,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: Makefile 470559 2014-04-15 18:54:14Z $
#

# For brand/release builds wl is built explicitly, so skip it here
SKIP       := $(if $(BRAND),wl)

# Skip test, jtagd, sim and wlap modules on branches
SKIP       += $(if $(TAG),test jtagd sim wlap)

SUBMKFILES := $(wildcard */Makefile)
SUBDIRS	   := $(sort $(filter-out $(SKIP),$(SUBMKFILES:/Makefile=)))

all: $(SUBDIRS)

$(SUBDIRS): FORCE
	@echo "START [`date`]: $(MAKE) $(if $(BRAND),-k) -w -C $@"
	@$(MAKE) $(if $(BRAND),-k) -w -C $@
	@echo "END   [`date`]: $(MAKE) $(if $(BRAND),-k) -w -C $@"

ifeq ($(BRAND),)
wl: roml romlsim
endif

mrproper: clean

show showsubdirs:
	@echo "SUBDIRS = $(SUBDIRS)"

clean: $(SUBDIRS:=_clean)

$(SUBDIRS:=_clean): FORCE
	@$(MAKE) -C $(@:_clean=) clean

.PHONY: FORCE
FORCE:
