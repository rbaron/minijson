# Config from:
# https://github.com/google/googletest/blob/master/googletest/README.md#incorporating-into-an-existing-cmake-project

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include"
                      "${gmock_SOURCE_DIR}/include")
endif()

add_executable(tokenizer_tests tokenizer_tests.cc)
target_link_libraries(tokenizer_tests gtest_main gmock_main)
add_test(NAME tokenizer_tests COMMAND tokenizer_tests)

add_executable(parser_tests parser_tests.cc)
target_link_libraries(parser_tests gtest_main gmock_main)
add_test(NAME parser_tests COMMAND parser_tests)

add_executable(serializer_tests serializer_tests.cc)
target_link_libraries(serializer_tests gtest_main gmock_main)
add_test(NAME serializer_tests COMMAND serializer_tests)

add_executable(unicode_tests unicode_tests.cc)
target_link_libraries(unicode_tests gtest_main gmock_main)
add_test(NAME unicode_tests COMMAND unicode_tests)

add_executable(jsonnode_tests jsonnode_tests.cc)
target_link_libraries(jsonnode_tests gtest_main gmock_main)
add_test(NAME jsonnode_tests COMMAND jsonnode_tests)

add_executable(bounditerator_tests bounditerator_tests.cc)
target_link_libraries(bounditerator_tests gtest_main gmock_main)
add_test(NAME bounditerator_tests COMMAND bounditerator_tests)

add_executable(iterator_tests iterator_tests.cc)
target_link_libraries(iterator_tests gtest_main gmock_main)
add_test(NAME iterator_tests COMMAND iterator_tests)