override MYDIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
override TOPDIR := $(abspath $(MYDIR)/..)

ABI ?= armeabi-v7a-hard x86 arm64-v8a x86_64

#========================================================================================

__comma := ,
__empty :=
__space := $(__empty) $(__empty)

define commas-to-spaces
$(subst $(__comma),$(__space),$(1))
endef

define spaces-to-commas
$(subst $(__space),$(__comma),$(1))
endef

# $1: ABI
define outdir
$(or $(strip $(OUTDIR)),$(MYDIR)/out)/$(1)
endef

define abis
$(call commas-to-spaces,$(ABI))
endef

#========================================================================================

.PHONY: all
all: build install

.PHONY: rebuild
rebuild:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory build

.PHONY: configure
configure: $(addprefix configure-,$(call abis))

.PHONY: build
build: $(addprefix build-,$(call abis))

.PHONY: test
test: $(addprefix test-,$(call abis))

.PHONY: install
install: $(addprefix install-,$(call abis))

.PHONY: clean
clean: $(addprefix clean-,$(call abis))

define add-build-rule
.PHONY: configure-$(1)
configure-$(1): | $$(call outdir,$(1))/build
	$$(if $$(strip $$(wildcard $$(NDK))),\
		$$(eval override NDK := $$(wildcard $$(NDK))),\
		$$(error NDK is not specified)\
	)
	$$(strip cd $$(call outdir,$(1))/build && \
		cmake \
			-DBUILD_ALL=ON \
			-DCMAKE_INSTALL_PREFIX=$$(call outdir,$(1))/install \
			-DCMAKE_TOOLCHAIN_FILE=$$(NDK)/cmake/toolchain.cmake \
			-DANDROID_ABI=$(1) \
			$$(TOPDIR)/dev \
	)

.PHONY: build-$(1)
build-$(1): configure-$(1)
	cd $$(call outdir,$(1))/build && $$(MAKE) -j16

.PHONY: test-$(1)
test-$(1):
	cd $$(call outdir,$(1))/build && $$(MAKE) test ARGS="-V"

.PHONY: install-$(1)
install-$(1): build-$(1)
	cd $$(call outdir,$(1))/build && $$(MAKE) install

.PHONY: clean-$(1)
clean-$(1):
	$$(if $$(wildcard $$(call outdir,$(1))),-rm -Rf $$(call outdir,$(1)))
	$$(if $$(wildcard $$(dir $$(call outdir,$(1)))),@rmdir $$(dir $$(call outdir,$(1))) >/dev/null 2>&1 || true)

$$(call outdir,$(1))/build:
	mkdir -p $$@
endef

$(foreach __abi,$(call abis),\
    $(eval $(call add-build-rule,$(__abi)))\
)
