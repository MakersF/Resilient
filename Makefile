PROJECT_PATH:=$(abspath .)

UBUNTU16:=resilient-ubuntu16
UBUNTU16-GCC6:=$(UBUNTU16)-gcc6
UBUNTU16-GCC7:=$(UBUNTU16)-gcc7

# For some make weirdness we need to escape 4 times the $
BUILD_COMMAND_CPP17:=cmake -D CPP17=ON $$$$SRC && make && make test
BUILD_COMMAND_CPP14:=cmake $$$$SRC && make && make test
BUILD_ENV_VARS:=GTEST_COLOR=1 CTEST_OUTPUT_ON_FAILURE=1

DOCKER?=docker

default: test

# Build the base image.
# $< is the first dependency of the target
buildimg/.docker-build-image-resilient-ubuntu16: buildimg/$(UBUNTU16).Dockerfile
	$(DOCKER) build -t $(UBUNTU16) -f $< .
	@touch $@

# Build any image which depends on the base image
buildimg/.docker-build-image-$(UBUNTU16)-%: buildimg/$(UBUNTU16)-%.Dockerfile buildimg/.docker-build-image-$(UBUNTU16)
	$(DOCKER) build -t $(UBUNTU16)-$* -f $< .
	@touch $@


# Allow the rules for building the images to be run again
docker-images-clean:
	@$(RM) -f buildimg/.docker-build-image-*


# Define a new test target.
# Args:
#  - name of the image to use
#  - name of the test
#  - command to execute in the image
#  - environment variables to be set in the image (space separated list of ENV=VALUE)
#
# A test target runs into an image, mounting the source directory into folder which path
# is stored in $SRC and a unique build directory, based on the name of the test, into /build.
# It executed the provided command in the /build directory.
#
# ------- Implementation Notice -------
#
# -t:
# We allocate a pseudo terminal in order to have colors.
#
# --cap-add:
# We give SYS_PTRACE capability to the container in order to allow asan
# to detect problems.
#
# -v:
# We mount the current project directory in the same path on the image.
# To allow the command to access it we define an env variable SRC
# which specifies the path.
#
# --env:
# We expand the list of user defined environment variables to pass to the
# container.
#
# -w:
# We change the default working directory to the build one.
#
# $1:
# We use the name of the container provided
#
# We then run the command provided in bash
define test-target
test-$1-$2: buildimg/.docker-build-image-$1
	@echo Running $2 on $1
	@$(DOCKER) run \
		--rm \
		-t \
		--cap-add SYS_PTRACE \
		-v $(PROJECT_PATH):$(PROJECT_PATH):ro \
		-v $(PROJECT_PATH)/build/$1-$2:/build \
		--env SRC="$(PROJECT_PATH)" \
		$(foreach env,$4,--env $(env)) \
		-w /build \
		$1 \
		/bin/bash -c '$3'

TEST_TARGETS_$1 += test-$1-$2
TEST_TARGETS += test-$1-$2
endef


# Define the test targets
$(eval $(call test-target,$(UBUNTU16-GCC7),cpp17,$(BUILD_COMMAND_CPP17),$(BUILD_ENV_VARS)))
$(eval $(call test-target,$(UBUNTU16-GCC7),cpp14,$(BUILD_COMMAND_CPP14),$(BUILD_ENV_VARS)))

$(eval $(call test-target,$(UBUNTU16-GCC6),cpp17,$(BUILD_COMMAND_CPP17),$(BUILD_ENV_VARS)))
$(eval $(call test-target,$(UBUNTU16-GCC6),cpp14,$(BUILD_COMMAND_CPP14),$(BUILD_ENV_VARS)))

$(eval $(call test-target,$(UBUNTU16),cpp17,$(BUILD_COMMAND_CPP17),$(BUILD_ENV_VARS)))
$(eval $(call test-target,$(UBUNTU16),cpp14,$(BUILD_COMMAND_CPP14),$(BUILD_ENV_VARS)))

# Run the tests on all the images
test: $(TEST_TARGETS)

local:
	cd build/local && cmake ../../ && make && ${BUILD_ENV_VARS} make test

.PHONY: test