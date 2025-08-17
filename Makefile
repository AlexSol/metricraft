BUILD_DIR := build
CMAKE_ARGS := -DCMAKE_BUILD_TYPE=Release

all: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)

$(BUILD_DIR)/Makefile:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_ARGS)

run:
	$(BUILD_DIR)/metricraft-agent

build-release:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_ARGS)
	cmake --build $(BUILD_DIR) -j $(shell nproc)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

# --
# Збірка
# mkdir build && cd build
# cmake -DAGENT_ENABLE_PYTHON=ON -DAGENT_ENABLE_LUA=ON ..
# cmake --build . -j

# # Запуск (за замовчуванням)
# ./agent   # PLUGINS_DIR=./plugins, TEXTFILE_DIR=./out, STATE_DIR=./state

# # Перевірка
# ls ../out/*.prom