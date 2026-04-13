add_executable(
        sys_prog_allctr_allctr_rb_tr_tests
        allocator_red_black_tree_tests.cpp)

target_link_libraries(sys_prog_allctr_allctr_rb_tr_tests
        PRIVATE
        GTest::gtest_main
        sys_prog_allctr_allctr_rb_tr
)