add_executable(
        sys_prog_allctr_allctr_bndr_tgs_tests
        allocator_boundary_tags_tests.cpp)

target_link_libraries(
        sys_prog_allctr_allctr_bndr_tgs_tests
        PRIVATE
        GTest::gtest_main
        sys_prog_allctr_allctr_bndr_tgs)
