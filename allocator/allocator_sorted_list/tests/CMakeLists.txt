add_executable(
        sys_prog_allctr_allctr_srtd_lst_tests
        allocator_sorted_list_tests.cpp)

target_link_libraries(
        sys_prog_allctr_allctr_srtd_lst_tests
        PRIVATE
        GTest::GTest
        GTest::Main)
target_link_libraries(
        sys_prog_allctr_allctr_srtd_lst_tests
        PRIVATE
        sys_prog_allctr_allctr_srtd_lst)