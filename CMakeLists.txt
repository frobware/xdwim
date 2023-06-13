cmake_minimum_required(VERSION 3.21)

project("xdwim")
enable_testing()
enable_language(C)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-padded -Wno-unused-parameter")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -ggdb3 -fno-inline -fno-omit-frame-pointer")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -ggdb3 -fno-inline -fno-omit-frame-pointer")
endif()

set(HDRS xdwim.h)

set(xdwim_SRC xdwim.c)
add_executable(xdwim ${xdwim_SRC} ${HDRS})
target_link_libraries(xdwim xdo)

set(xdwimctl_SRC xdwimctl.c)
add_executable(xdwimctl ${xdwimctl_SRC} ${HDRS})

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/xdwim DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/xdwimctl DESTINATION bin)

add_executable(tests tests.c ${HDRS})
target_link_libraries(tests cunit)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  target_compile_definitions(tests PRIVATE -DCU_VERBOSE=1 -DC_SILENT=0)
  # Enable code coverage for debug builds
  set(ENABLE_CODE_COVERAGE ON)
endif()

# To execute any CUnit tests, run `make test` (which runs ctest(1),
# which in turn runs our tests executable; our tests use CUnit to run
# the test_readln test).
#
# For inline and verbose output run `make test ARGS=-V`.
find_program(VALGRIND "valgrind")
if(VALGRIND)
  add_test(NAME all COMMAND ${VALGRIND} --leak-check=full --show-leak-kinds=all --verbose --log-file=valgrind_tests.log $<TARGET_FILE:tests>)
else()
  add_test(NAME all COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests --no-tests=error --help)
endif()

#set_tests_properties(all PROPERTIES ENVIRONMENT "CU_SILENT=0")

# Makefile target for running tests and generating code coverage
add_custom_target(coverage
  COMMAND lcov --directory . --capture --output-file coverage.info
  COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage.info
  COMMAND lcov --list coverage.info
  COMMAND genhtml coverage.info --output-directory out
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating code coverage report"
)

option(ENABLE_CODE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_CODE_COVERAGE)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
  endif()
  target_link_libraries(tests gcov)
endif()

# Generate file compile_commands.json.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  # The purpose of augmenting CMAKE_C_STANDARD_INCLUDE_DIRECTORIES is
  # to ensure that the paths to header files (libc, et al) are
  # correctly encoded in compile_commands.json. This is necessary in
  # Nix-based environments where the location of the headers can be
  # different from typical locations (e.g., /usr/include,
  # /usr/local/include). By including the implicit include directories
  # in compile_commands.json, Nix-based environments that also use
  # language servers (e.g., Emacs & eglot & clangd) can properly
  # locate header files and subsequently jump to symbol
  # definitions/declarations.
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# foo
