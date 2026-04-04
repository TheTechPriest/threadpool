#ifndef MAIN_CPP
#define MAIN_CPP

#include "ThreadPool.cpp"
#include "benchmark.cpp"

void GenerateRandomVector(std::vector<int> &generated, int length) {
  for (int i = 0; i <= length; ++i) {
    generated.push_back(std::rand());
  }
}

int partition(std::vector<int> &values, int low, int high) {
  int pivot = values[high];

  int i = low - 1;

  for (int j = low; j <= high - 1; ++j) {
    if (values[j] <= pivot) {
      ++i;
      std::swap(values[i], values[j]);
    }
  }

  std::swap(values[i + 1], values[high]);

  return (i + 1);
}

void quicksort(std::vector<int> &values, int low, int high) {

  if (low < high) {
    int index = partition(values, low, high);

    quicksort(values, low, index - 1);
    quicksort(values, index + 1, high);
  }
}

double average(std::vector<double> values) {
  double sum{0};

  std::for_each(values.begin(), values.end(), [&sum](double i) { sum += i; }

  );

  return sum / values.size();
}

void ParallelSort(std::vector<int> &values, ThreadPool &pool) {
  int num_cores = std::thread::hardware_concurrency();
  int chunk_size = values.size() / num_cores;
  std::vector<std::future<void>> futures;

  for (int i = 0; i < num_cores; ++i) {
    int left = i * chunk_size;
    int right =
        (i == num_cores - 1) ? values.size() - 1 : (left + chunk_size - 1);

    // Capture by reference ONLY if you are sure values lives long enough!
    futures.push_back(pool.QueueJob(
        [&values, left, right] { quicksort(values, left, right); }));
  }

  for (auto &fut : futures) {
    fut.wait();
  }
  for (int i = 1; i < num_cores; ++i) {
    int mid = i * chunk_size;
    int end = (i == num_cores - 1) ? values.size() : ((i + 1) * chunk_size);
    std::inplace_merge(values.begin(), values.begin() + mid,
                       values.begin() + end);
  }
}

int main() {

  //   std::cout << "You have " << std::thread::hardware_concurrency()
  //             << " threads available for concurrency.\n";
  //   return 0;

  // unsigned int numTests = 2000;
  unsigned int maxthreads{std::thread::hardware_concurrency()};
  unsigned int exponent{2};
  std::vector<int> values;

  std::vector<double> threadsDurations;
  std::vector<double> serialDurations;

  GenerateRandomVector(values, pow(maxthreads, exponent));
  std::cout << "Generating vector of length: " << pow(maxthreads, exponent)
            << '\n';

  // std::vector<double> average_test = {1, 5};

  // std::cout << "Average value of vector is: " << average(average_test) <<
  // '\n';

  for (auto i : values) {
    std::cout << i << " ";
  }

  std::cout << std::endl;

  ThreadPool pool;

  pool.Start();

  ParallelSort(values, pool);

  // for (unsigned int i = 0; i < numTests; ++i) {
  //   {

  //     Timer timer(threadsDurations);
  //     pool.QueueJob([&values] { quicksort(values, 0, values.size() - 1); });

  //     std::cout << "Time taken by thread pool implementation: ";
  //   }
  // }

  pool.Stop();

  for (auto i : values) {
    std::cout << i << " ";
  }

  // std::cout << "SORTED:\n";
  // for (auto i : values) {
  //   std::cout << i << " ";
  // }

  // std::cout << "UNSORTED:\n";
  // for (auto i : values) {
  //   std::cout << i << " ";
  // }

  // std::cout << std::endl;

  // for (unsigned i = 0; i < numTests; ++i) {
  //   {
  //     Timer timer(serialDurations);
  //     quicksort(values, 0, values.size() - 1);

  //     // std::cout << "SORTED:\n";
  //     // for (auto i : values) {
  //     //   std::cout << i << " ";
  //     // }

  //     std::cout << "Time taken by serial implementation: ";
  //   }
  // }

  // std::cout << "Durations: \n";

  // std::cout << "Thread pool took an average of: " <<
  // average(threadsDurations)
  //           << " ms\n";
  // std::cout << "Standard took an average of: " << average(serialDurations)
  //           << " ms\n";

  // std::cout << "Number of tests: " << numTests << std::endl;
}

#endif
