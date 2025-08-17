PROJECT_NAME := metricraft-agent

BUILD_DIR := build
RELEASE_CMAKE_ARGS := -DCMAKE_BUILD_TYPE=Release

JOBS_BUILD := $(shell nproc)

all: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)

$(BUILD_DIR)/Makefile:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_ARGS)

run:
	$(BUILD_DIR)/$(PROJECT_NAME) ./plugins

build-debug:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DSTATIC_CXX_RUNTIME=OFF
	cmake --build $(BUILD_DIR) -j $(JOBS_BUILD)

build-release:
	cmake -S . -B $(BUILD_DIR) $(RELEASE_CMAKE_ARGS) -DSTATIC_CXX_RUNTIME=OFF
	cmake --build $(BUILD_DIR) -j $(JOBS_BUILD)

build-release-static-cxx-runtime:
	cmake -S . -B $(BUILD_DIR) $(RELEASE_CMAKE_ARGS) -DSTATIC_CXX_RUNTIME=ON
	cmake --build $(BUILD_DIR) -j $(JOBS_BUILD)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all
