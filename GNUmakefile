# If this Makefile changes, then everthing should be rebuilt.
.EXTRA_PREREQS := $(abspath $(lastword $(MAKEFILE_LIST)))

CMAKE_GENERATOR		= "Unix Makefiles"
CMAKE_BUILDER           = make
CMAKE_BUILDER_ARGS      = -j --no-print-directory
BUILD_FILE              = Makefile
BUILD_DIR               = build

CMAKE_BUILD_TYPE	?= Release
CMAKE_C_COMPILER        ?= gcc

# Argument 1 is the build directory.
# Argument 2 are specific arguments to pass to cmake.
# Argument 3 is the directory containing the CMakeLists.txt.
define regenerate_cmake_rules
	@echo "Regenerating CMake build in $(1)"
	@cd $(1) ;					\
	cmake -G$(CMAKE_GENERATOR)			\
		-DCMAKE_C_COMPILER=$(CMAKE_C_COMPILER)	\
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)	\
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON	\
		$(2) $(3)
endef

.PHONY: rclean $(BUILD_DIR)

all: $(BUILD_DIR)/$(BUILD_FILE)
	@$(CMAKE_BUILDER) $(CMAKE_BUILDER_ARGS) -C $(BUILD_DIR) $@

test: all
	@$(CMAKE_BUILDER) $(CMAKE_BUILDER_ARGS) -C $(BUILD_DIR) test

rclean:
	$(RM) -r $(BUILD_DIR)

$(BUILD_DIR)/$(BUILD_FILE): | $(BUILD_DIR)
	$(call regenerate_cmake_rules,$(BUILD_DIR),$(CURDIR))

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/$(BUILD_FILE): GNUmakefile		\
	CMakeLists.txt				\
	| $(BUILD_DIR)

.DEFAULT:
	@$(CMAKE_BUILDER) $(CMAKE_BUILDER_ARGS) -C $(BUILD_DIR) $@

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all     - Build the entire project"
	@echo "  rclean  - Remove the build directory"
	@echo "  test    - Run the cunit tests"
