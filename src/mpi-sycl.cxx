#include <mpi.h>
#include <iostream>

#include <CL/sycl.hpp>

class hello_world;
class hello_world2;

int main(int argc, char* argv[]) {
  // Initialise MPI
  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    std::cout << "Failed to initialize MPI\n";
    exit(-1);
  }
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // SYCL Hello World
  auto defaultQueue = cl::sycl::queue{};

  {
    auto t1 = defaultQueue
      .submit([&](cl::sycl::handler& cgh) {
        auto os = cl::sycl::stream{128, 128, cgh};
        
        cgh.single_task<hello_world>([=]() { os << "Hello World from " << rank <<"!\n"; });
      });
    auto t2 = defaultQueue
      .submit([&](cl::sycl::handler& cgh) {
        auto os = cl::sycl::stream{128, 128, cgh};
        
        cgh.single_task<hello_world2>([=]() { os << "Hello World2 from " << rank << "!\n"; });
      });
  }
  
  MPI_Finalize();
  return 0;
}
