# Front-end subdir level makefile for hndrte based standalone programs.
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#
# Invoked from src/dongle/make/wl/Makefile.
#

ifeq ($(TOPDIR),)
$(error TOPDIR is not defined)
endif

SUBDIR := $(shell /bin/pwd)
SUBDIR := $(SUBDIR:$(TOPDIR)/%=%)

SUBBLDDIR ?= $(SUBDIR)

ROMLSYMBLDDIR := $(TOPBLDDIR)/romlsym

#####################################################################################

ifeq ($(SBRAND),)

BRANDS := $(patsubst %.mk,%,$(wildcard *.mk))

# default make target
.PHONY: default
default: all

# show all brands
showbrands: FORCE
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets
showtgts: FORCE
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets in a brand
$(BRANDS:=/showtgts): FORCE
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets
showmaxsize_tgts: FORCE
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# show all targets in a brand
$(BRANDS:=/showmaxsize_tgts): FORCE
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# clean all brands
.PHONY: clean
clean: $(BRANDS:=/clean)

# clean all targets in a brand
$(BRANDS:=/clean): FORCE
	$(MAKE) SBRAND=$(@:/clean=) $@

# clean a specific target in a brand
$(BRANDS:=/%/clean): _BRAND = $(firstword $(subst /, ,$@))
$(BRANDS:=/%/clean): FORCE
	$(MAKE) SBRAND=$(_BRAND) $@

# build all brands
.PHONY: all
all: $(BRANDS)

# build all targets in a brand
$(BRANDS): FORCE
	$(MAKE) SBRAND=$@

# build a specific target in a brand
$(BRANDS:=/%): _BRAND = $(firstword $(subst /, ,$@))
$(BRANDS:=/%): FORCE
	$(MAKE) SBRAND=$(_BRAND) $@

endif	# !SBRAND

#####################################################################################

ifneq ($(SBRAND),)

# prevent multiple targets from invoking the same romlsim make
.NOTPARALLEL:

include $(SBRAND).mk

ifndef ROMLDIR
$(error ROMLDIR is not defined)
endif

ifeq ($(wildcard $(ROMLDIR)/Makefile),)
$(error $(ROMLDIR)/Makefile does not exist)
endif

# build both ROM and RAM targets
.PHONY: all
all: $(ROMLDIR) $(SBRAND)

# clean romtable subdir
romlsym/clean: FORCE
	$(MAKE) -C $(ROMLDIR) ROMLSYMBLDDIR=$(ROMLSYMBLDDIR) clean

# clean ROM subdir
roml/clean: FORCE
	$(MAKE) -C $(ROMLDIR) ROMLSYMBLDDIR=$(ROMLSYMBLDDIR) clean

# clean all targets in a brand
.PHONY: $(SBRAND)/clean
$(SBRAND)/clean: romlsym/clean roml/clean
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# clean a specific target in a brand
$(SBRAND)/%/clean: FORCE
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# build all targets
.PHONY: $(SBRAND)
$(SBRAND): romlsym roml
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# build a specific target
.PHONY: $(SBRAND)/%
$(SBRAND)/%: romlsym roml
	$(MAKE) -C $(TOPDIR) -f $(TOPMK) SUBDIR=$(SUBDIR) SUBBLDDIR=$(SUBBLDDIR) $@

# build romtable_full.S
romlsym: FORCE
	$(MAKE) -C $(ROMLDIR) ROMLSYMBLDDIR=$(ROMLSYMBLDDIR) romtable_full

# build rom
roml: FORCE
	$(MAKE) -C $(ROMLDIR) ROMLSYMBLDDIR=$(ROMLSYMBLDDIR) all

endif	# SBRAND

.PHONY: FORCE
FORCE:
