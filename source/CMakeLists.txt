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
    target_include_directories(${t} PRIVATE "${PROJECT_SOURCE_DIR}/include")

    set_target_properties(${t} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    install(TARGETS ${t}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
    )

    if (MSVC)
        target_compile_options(${t} PRIVATE
            # Warnings
            /W4
            /WX
        )
    else()
        target_compile_options(${t} PRIVATE
            # Warnings
            -Wall 
            -Wextra 
            -Werror
            # Sanitizers 
            $<$<BOOL:${SANITIZE}>: -fsanitize=address,undefined,leak,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr>
        )
        target_link_options(${t} PRIVATE
            # Sanitizers
            $<$<BOOL:${SANITIZE}>: -fsanitize=address,undefined,leak,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,unreachable,vla-bound,vptr>
        )
    endif()
endforeach()
