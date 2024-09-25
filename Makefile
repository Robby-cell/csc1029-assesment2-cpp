BUILD_DIR = out

TUI = ${BUILD_DIR}/QUBImages
GUI = ${BUILD_DIR}/QUBMediaImages

export VCPKG_ROOT

DEBUG_FLAGS = -DCMAKE_BUILD_TYPE=Debug
RELEASE_FLAGS = -DCMAKE_BUILD_TYPE=Release

tui: debug
	./${TUI}
gui: debug
	./${GUI}

BIN: debug

${BIN}: ${BUILD_DIR}/ debug

${BIN}.exe: ${BUILD_DIR}/ debug

debug: ${BUILD_DIR}/
	cd ${BUILD_DIR} && \
	cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ${DEBUG_FLAGS} && \
	cmake --build .

release: ${BUILD_DIR}/
	cd ${BUILD_DIR} && \
	cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ${RELEASE_FLAGS} && \
	cmake --build .

${BUILD_DIR}/:
	mkdir ${BUILD_DIR}