#ifndef MAIN_CPP
#define MAIN_CPP

#include "ThreadPool.cpp"
#include "benchmark.cpp"

void GenerateRandomVector(std::vector<int> &generated, int length) {
  generated.clear();
  for (int i = 0; i < length; ++i) {
    generated.push_back(std::rand());
  }

  // std::cout << "Generating vector of length: " << length << '\n';
}

int Partition(std::vector<int> &values, int low, int high) {
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

void Quicksort(std::vector<int> &values, int low, int high) {

  if (low < high) {
    int index = Partition(values, low, high);

    Quicksort(values, low, index - 1);
    Quicksort(values, index + 1, high);
  }
}

double Average(std::vector<double> values) {
  double sum{0};

  std::for_each(values.begin(), values.end(), [&sum](double i) { sum += i; }

  );

  return sum / values.size();
}

void ParallelSort(std::vector<int> &values, ThreadPool &pool,
                  std::vector<double> &threadsDurations) {
  Timer timer(threadsDurations);
  int num_cores = std::thread::hardware_concurrency();
  int chunk_size = values.size() / num_cores;
  std::vector<std::future<void>> futures;

  for (int i = 0; i < num_cores; ++i) {
    int left = i * chunk_size;
    int right =
        (i == num_cores - 1) ? values.size() - 1 : (left + chunk_size - 1);

    futures.push_back(pool.QueueJob(
        [&values, left, right] { Quicksort(values, left, right); }));
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
  unsigned int numTests = 20000;
  unsigned int maxthreads{std::thread::hardware_concurrency()};
  unsigned int exponent{4};
  std::vector<int> master;
  std::vector<double> threadsDurations;
  std::vector<double> serialDurations;

  std::cout << "Starting " << numTests
            << " tests. Vectors will be of size: " << pow(maxthreads, exponent)
            << '\n';

  GenerateRandomVector(master, pow(maxthreads, exponent));

  std::vector<int> values{master};

  ThreadPool pool;

  pool.Start();

  for (unsigned int i = 0; i < numTests; ++i) {
    values = master;
    ParallelSort(values, pool, threadsDurations);
  }

  pool.Stop();

  for (unsigned i = 0; i < numTests; ++i) {
    {
      values = master;
      Timer timer(serialDurations);
      Quicksort(values, 0, values.size() - 1);
    }
  }

  double averageThreads = Average(threadsDurations);
  double averageStandard = Average(serialDurations);

  std::cout << "Thread pool took an average of: " << averageThreads << " ms\n";
  std::cout << "Standard took an average of: " << averageStandard << " ms\n";

  std::cout << "Number of tests: " << numTests << std::endl;

  if (averageThreads < averageStandard) {
    std::cout << "Thread Pool won!" << std::endl;
    std::cout << "It was " << (averageStandard / averageThreads * 100) - 100
              << "% faster!" << std::endl;
  }

  if (averageThreads > averageStandard) {
    std::cout << "Standard won!" << std::endl;
    std::cout << "It was " << (averageThreads / averageStandard * 100) - 100
              << "% faster!" << std::endl;
  }
}

#endif
