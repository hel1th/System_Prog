add_subdirectory(tests)

add_library(
        sys_prog_allctr_allctr_glbl_hp
        src/allocator_global_heap.cpp)

target_include_directories(
        sys_prog_allctr_allctr_glbl_hp
        PUBLIC
        ./include)

target_link_libraries(
        sys_prog_allctr_allctr_glbl_hp
        PUBLIC
        sys_prog_cmmn
        sys_prog_allctr_allctr)

add_executable(
        allocator_global_heap_demo
        src/main.cpp)

target_link_libraries(
        allocator_global_heap_demo
        PRIVATE
        sys_prog_allctr_allctr_glbl_hp)