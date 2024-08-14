cmake_minimum_required(VERSION 3.2)
project(main)
enable_testing()

# ------------------------- DEFINITIONS ------------------------- #

set(CMAKE_CXX_STANDARD 14)

# Defining the relevant versions of OpenDLV Standard Message Set.
# The OpenDLV Standard Message Set contains a set of messages usually used in automotive research project.
set(OPENDLV_STANDARD_MESSAGE_SET opendlv-standard-message-set-v0.9.6.odvd)
# Defining the relevant version of libcluon.
set(CLUON_COMPLETE cluon-complete-v0.0.127.hpp)

# Set the search path for .cmake files.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build a static binary.
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Add further warning levels.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -D_XOPEN_SOURCE=700 \
    -D_FORTIFY_SOURCE=2 \
    -O2 \
    -fstack-protector \
    -fomit-frame-pointer \
    -pipe \
    -Weffc++ \
    -Wall -Wextra -Wshadow -Wdeprecated \
    -Wdiv-by-zero -Wfloat-equal -Wfloat-conversion -Wsign-compare -Wpointer-arith \
    -Wuninitialized -Wunreachable-code \
    -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-but-set-parameter -Wunused-but-set-variable \
    -Wunused-value -Wunused-variable -Wunused-result \
    -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn \
    -w")

# Threads are necessary for linking the resulting binaries as UDPReceiver is running in parallel.
set(THREADS_PREFER_PTHREAD_FLAG ON)

# --------------------- I HAVE NO CLUE --------------------- #

find_package(Threads REQUIRED)

# Create cluon-complete.hpp.
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cluon-msc
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/src/${CLUON_COMPLETE} ${CMAKE_BINARY_DIR}/cluon-complete.hpp
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/cluon-complete.hpp ${CMAKE_BINARY_DIR}/cluon-complete.cpp
    COMMAND ${CMAKE_CXX_COMPILER} -o ${CMAKE_BINARY_DIR}/cluon-msc ${CMAKE_BINARY_DIR}/cluon-complete.cpp -std=c++14 -pthread -D HAVE_CLUON_MSC
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${CLUON_COMPLETE})

# Generate opendlv-standard-message-set.hpp from ${OPENDLV_STANDARD_MESSAGE_SET} file.
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/opendlv-standard-message-set.hpp
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_BINARY_DIR}/cluon-msc --cpp --out=${CMAKE_BINARY_DIR}/opendlv-standard-message-set.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/${OPENDLV_STANDARD_MESSAGE_SET}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${OPENDLV_STANDARD_MESSAGE_SET} ${CMAKE_BINARY_DIR}/cluon-msc)

# Add current build directory as include directory as it contains generated files.
include_directories(SYSTEM ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Gather all object code first to avoid double compilation.
set(LIBRARIES Threads::Threads)

if(UNIX)
    if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        find_package(LibRT REQUIRED)
        set(LIBRARIES ${LIBRARIES} ${LIBRT_LIBRARIES})
        include_directories(SYSTEM ${LIBRT_INCLUDE_DIR})
    endif()
endif()

find_package(OpenCV REQUIRED core highgui imgproc imgcodecs)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})
set(LIBRARIES ${LIBRARIES} ${OpenCV_LIBS})

# ------------------------- EXECUTABLES ------------------------- #

add_executable(${PROJECT_NAME}-Runner ${CMAKE_BINARY_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/TestMain.cpp)
add_test(NAME ${PROJECT_NAME}-Runner COMMAND ${PROJECT_NAME}-Runner)
target_link_libraries(${PROJECT_NAME}-Runner PRIVATE ${LIBRARIES} stdc++fs --coverage)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp )
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# Add dependency to OpenDLV Standard Message Set.
add_custom_target(generate_opendlv_standard_message_set_hpp DEPENDS ${CMAKE_BINARY_DIR}/opendlv-standard-message-set.hpp)
add_dependencies(${PROJECT_NAME} generate_opendlv_standard_message_set_hpp)

install(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT ${PROJECT_NAME})

# ---------------- VALGRIND COMPILE OPTIONS ---------------- #

find_program(VALGRIND "valgrind")

if(VALGRIND)
    target_compile_options(${PROJECT_NAME} PRIVATE -g) # Bad for performance
    target_compile_options(${PROJECT_NAME} PRIVATE -O0) # Bad for performance
endif()

# --------------- COVERAGE REPORT GENERATION --------------- #

# Adds coverage flags to the compiler and linker
target_compile_options(${PROJECT_NAME}-Runner PRIVATE --coverage)

# Defines coverage target
add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E env "COVERAGE_DIR=." lcov --directory . --capture --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info # Filters out system libraries
    COMMAND genhtml coverage.info --output-directory coverageReport # Generates HTML report
)

