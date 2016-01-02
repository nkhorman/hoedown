cmake_minimum_required (VERSION 2.6)
project (Hoedown)

option (BUILD_SHARED_LIBS "Wether to build Hoedown as a shared library" true)
option (BUILD_CLI "Wether to build the hoedown executable" true)

set (FLAGS "-std=c99 -Wall -Wextra -Wno-unused-parameter")


# Pregenerated files
add_custom_target(_html_blocks.c
  gperf -L ANSI-C -N hoedown_find_block_tag -7 -c -C -E -S 1 --ignore-case -m100 data/html_blocks.gperf > src/_html_blocks.c
)
add_custom_target(_autolink_schemes.c
  gperf -L ANSI-C -N hoedown_find_autolink_scheme -l -c -C -E -S 1 --ignore-case -m100 data/autolink_schemes.gperf > src/_autolink_schemes.c
)
add_custom_target(_case_folding.h
  gperf -L ANSI-C -N find_case_mapping -l -c -C -E -S 1 -t -m100 data/case_folding.gperf > src/_case_folding.h
)


# Library
add_library(libhoedown
  src/autolink.c
  src/buffer.c
  src/document.c
  src/escape.c
  src/html.c
  src/list.c
  src/pool.c
  src/version.c
  src/manpage.c

  src/_html_blocks.c
  src/_autolink_schemes.c
)

set_target_properties(libhoedown PROPERTIES
  COMPILE_FLAGS ${FLAGS}
  OUTPUT_NAME hoedown
  SOVERSION 4
  VERSION 4.0.0
)

install(TARGETS libhoedown DESTINATION lib)
install(DIRECTORY src/ DESTINATION include/hoedown
  FILES_MATCHING PATTERN "*.h"
  PATTERN "_*" EXCLUDE
)


# Executables
if (BUILD_CLI)
  add_executable(hoedown bin/hoedown.c bin/noop.c)
  target_link_libraries(hoedown libhoedown)

  set_target_properties(hoedown PROPERTIES COMPILE_FLAGS ${FLAGS})
  include_directories(${PROJECT_SOURCE_DIR}/src)

  install(TARGETS hoedown DESTINATION bin)

  get_target_property(HOEDOWN_LOCATION hoedown LOCATION)
  add_custom_target(test
    python3 CommonMark/test/spec_tests.py --spec spec.txt --program "${HOEDOWN_LOCATION}"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    DEPENDS hoedown
  )
endif (BUILD_CLI)
