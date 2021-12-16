cmake_minimum_required(VERSION 3.21)
project(CodeSnippet)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CHAPTERS
    test
)

set(test
    test_mem_detect
    test_template_if
    test_template_while
    test_gcd_lcm
    test_loop_unrolling
    test_template_expression
)
foreach(CHAPTER ${CHAPTERS})
    foreach(DEMO ${${CHAPTER}})
        file(GLOB SOURCE
            "${CHAPTER}/${DEMO}.h"
            "${CHAPTER}/${DEMO}.cpp")
        add_executable(${DEMO} ${SOURCE})

    endforeach(DEMO)
endforeach(CHAPTER)