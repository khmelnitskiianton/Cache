set(LRU_TARGET cache_lru)
set(LFU_TARGET cache_lfu)
set(IDEAL_TARGET cache_ideal)

set(LRU_SOURCES 
    ${PROJECT_SOURCE_DIR}/source/lru.cpp
   )
set(LFU_SOURCES 
   ${PROJECT_SOURCE_DIR}/source/lfu.cpp
  )
set(IDEAL_SOURCES
    ${PROJECT_SOURCE_DIR}/source/ideal.cpp
   )

set(CACHE_TARGETS ${LRU_TARGET} ${IDEAL_TARGET} ${LFU_TARGET})

add_executable(${LRU_TARGET} ${LRU_SOURCES})
add_executable(${IDEAL_TARGET} ${IDEAL_SOURCES})
add_executable(${LFU_TARGET} ${LFU_SOURCES})

foreach(t IN LISTS CACHE_TARGETS)
    target_include_directories(${t} PUBLIC "${PROJECT_SOURCE_DIR}/include")
    set_target_properties(${t} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    target_compile_options(${t} PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:
            # Debug codegen
            -D _DEBUG 
            -g 
            -ggdb3
            -O0
            -fno-omit-frame-pointer
            -fno-optimize-sibling-calls
            
            # Warnings
            -Wall -Wextra -Wpedantic
            -Wconversion -Wsign-conversion -Wshadow
            -Wnon-virtual-dtor -Woverloaded-virtual
            -Wcast-align -Wcast-qual
            -Wmissing-declarations -Wundef
            -Wformat -Wformat-security
            -Wswitch-default -Wswitch-enum
            -Wunreachable-code
            -Wno-missing-field-initializers
            -Wno-old-style-cast
            -Wvariadic-macros

            # Sanitizers 
            -fsanitize=address,undefined,leak,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr
        >
    )
    target_link_options(${t} PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:CXX>>:
            # Sanitizers
            -fsanitize=address,undefined,leak,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr
        >
    )
    target_compile_options(${t} PRIVATE
        $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:CXX>>:
            # Optimizations & disabling checks
            -O3 -DNDEBUG
        >
    )

endforeach()
