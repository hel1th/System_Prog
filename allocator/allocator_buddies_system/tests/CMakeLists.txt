add_executable(
        sys_prog_allctr_allctr_bdds_sstm_tests
        allocator_buddies_system_tests.cpp)

target_link_libraries(
        sys_prog_allctr_allctr_bdds_sstm_tests
        PRIVATE
        GTest::gtest_main
        sys_prog_allctr_allctr_bdds_sstm)
