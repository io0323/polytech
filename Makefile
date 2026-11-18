CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic
SRC_DIR := 01_fundamentals/c-language（C言語）
BUILD   := build

SRCS := $(wildcard $(SRC_DIR)/*/*.c)

# 01_fundamentals/c-language（C言語）/01_control_functions/04_for_and_while.c
#   -> build/01_control_functions_04_for_and_while
bin_name = $(BUILD)/$(subst /,_,$(patsubst $(SRC_DIR)/%.c,%,$(1)))

BINS := $(foreach s,$(SRCS),$(call bin_name,$(s)))

.PHONY: all clean
all: $(BINS)

define BUILD_RULE
$(call bin_name,$(1)): $(1) | $(BUILD)
	$$(CC) $$(CFLAGS) $$< -o $$@
endef
$(foreach s,$(SRCS),$(eval $(call BUILD_RULE,$(s))))

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
