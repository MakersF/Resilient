PROJECT_PATH:=$(abspath .)

UBUNTU16:=resilient-ubuntu16
UBUNTU16-GCC6:=$(UBUNTU16)-gcc6
UBUNTU16-GCC7:=$(UBUNTU16)-gcc7

DOCKER?=docker

default: test

# Build the base image
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
#
# A test target runs into an image, mounting the source directory into /src
# and a unique build directory, based on the name of the test, into /build,
# the bash command provided.
define test-target
test-$1-$2: buildimg/.docker-build-image-$1
	@$(DOCKER) run \
		--rm \
		-t \
		-v $(PROJECT_PATH):/src:ro \
		-v $(PROJECT_PATH)/build/$1-$2:/build \
		-w /build \
		$1 \
		/bin/bash -c "$3"

TEST_TARGETS_$1 += test-$1-$2
TEST_TARGETS += test-$1-$2
endef


# Define the test targets
$(eval $(call test-target,$(UBUNTU16-GCC7),cpp17,cmake -D CPP17=ON /src && make && make test))
$(eval $(call test-target,$(UBUNTU16-GCC7),cpp14,cmake /src && make && make test))

$(eval $(call test-target,$(UBUNTU16-GCC6),cpp17,cmake -D CPP17=ON /src && make && make test))
$(eval $(call test-target,$(UBUNTU16-GCC6),cpp14,cmake /src && make && make test))

$(eval $(call test-target,$(UBUNTU16),cpp17,cmake -D CPP17=ON /src && make && make test))
$(eval $(call test-target,$(UBUNTU16),cpp14,cmake /src && make && make test))

# Run the tests on all the images
test: $(TEST_TARGETS)

.PHONY: test