#include "../include/allocator_global_heap.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <pp_allocator.h>

struct Point3D {
  double x;
  double y;
  double z;
};

int main() {
  try {
    std::unique_ptr<smart_mem_resource> alloc(new allocator_global_heap());

    int *p_int = reinterpret_cast<int *>(alloc->allocate(sizeof(int)));
    *p_int = 42;
    std::cout << "int: " << *p_int << "\n";

    double *p_double =
        reinterpret_cast<double *>(alloc->allocate(sizeof(double)));
    *p_double = 3.14159;
    std::cout << "double: " << *p_double << "\n";

    char *p_str = reinterpret_cast<char *>(alloc->allocate(sizeof(char) * 20));
    strcpy(p_str, "Hello, world!");
    std::cout << "string: " << p_str << "\n";

    Point3D *p_point =
        reinterpret_cast<Point3D *>(alloc->allocate(sizeof(Point3D)));
    p_point->x = 1.0;
    p_point->y = 2.5;
    p_point->z = -3.7;
    std::cout << "point: x=" << p_point->x << ", y=" << p_point->y
              << ", z=" << p_point->z << "\n";

    alloc->deallocate(p_int, sizeof(int));
    alloc->deallocate(p_double, sizeof(double));
    alloc->deallocate(p_str, sizeof(char) * 20);
    alloc->deallocate(p_point, sizeof(Point3D));

    int *p_int2 = reinterpret_cast<int *>(alloc->allocate(sizeof(int)));
    *p_int2 = 100;
    std::cout << "int after deallocation: " << *p_int2 << "\n";
    alloc->deallocate(p_int2, sizeof(int));

    allocator_global_heap alloc2;
    std::cout << "two global_heap equal: "
              << (alloc->is_equal(alloc2) ? "yes" : "no") << "\n";

    test_mem_resource other;
    std::cout << "global_heap equal to test_mem_resource: "
              << (alloc->is_equal(other) ? "yes" : "no") << "\n";
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << "\n";
  }

  return 0;
}