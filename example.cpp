#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>

#include "mono_wedge.h"

struct Sample {
  float value;
  int time;

  bool operator<(const Sample& o) const { return value < o.value; }
  bool operator>(const Sample& o) const { return value > o.value; }
};
template <class charT, charT sep>
class punct_facet : public std::numpunct<charT> {
 protected:
  charT do_decimal_point() const { return sep; }
};

std::ostream& operator<<(std::ostream& os, const Sample& sample) {
  os << sample.time << '/' << sample.value;
  return os;
}

class TimeGauge {
 private:
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point end_time;

 public:
  TimeGauge() { start_time = std::chrono::steady_clock::now(); }
  auto Stop() {
    end_time = std::chrono::steady_clock::now();
    return end_time - start_time;
  }
};

std::chrono::nanoseconds example(const std::vector<float> &values, bool write_file, bool write_console) {
  std::vector<Sample> samples;
  int time = 0;
  std::transform(values.begin(), values.end(), std::back_inserter(samples), [&time](float n) {
    return Sample{20 + n / 10.0f, time++};
  });

  std::ofstream out_file;
  if (write_file) {
    out_file.open("output.csv");
    out_file.imbue(std::locale(std::cout.getloc(), new punct_facet<char, ','>));
  }

  const int rangeSize = 20;
  mono_wedge::mono_wedge<int, Sample> wedge;
  std::vector<std::chrono::nanoseconds> durations;

  for (auto& sample : samples) {
    TimeGauge timer;
    // Add the new sample to our wedge
    wedge.max_update(sample.time, sample);

    int removed{};
    while (wedge.begin()->first <= sample.time - rangeSize) {
      wedge.pop_front();
      removed++;
    }

    // The maximum value is at the front of the (never empty) wedge.
    auto maximumInRange = wedge.begin();
    durations.push_back(timer.Stop());

    auto maximum_in_range = *maximumInRange;
    if (write_console) {
      std::cout << sample << "\n   Wedge: ";

      for (auto& i : wedge) {
        std::cout << i.second << ' ';
      }
      std::cout << "\n";
      if (removed) {
        std::cout << "   REMOVED " << removed << '\n';
      }
    }

    if (write_file) {
      out_file << sample.time << ';' << sample.value << ';' << maximumInRange->second.value << '\n';
    }
  }

  const auto total_time = std::accumulate(durations.begin(), durations.end(), std::chrono::nanoseconds::zero());
  return total_time;
}

int main(void) {
  std::vector<float> values{
      72, 63, 72, 84, 29, 30, 16, 49, 83, 78, 35,  8,  5,  42, 31, 82, 72, 74, 97, 86, 5,  76, 77, 6,  6,  56, 25, 5,
      93, 71, 6,  43, 18, 79, 79, 3,  45, 81, 57,  86, 95, 25, 75, 17, 51, 27, 8,  7,  28, 70, 34, 6,  19, 3,  84, 84,
      77, 6,  7,  98, 45, 57, 72, 93, 90, 49, 50,  87, 35, 98, 79, 4,  55, 10, 32, 61, 29, 93, 41, 31, 88, 93, 0,  37,
      67, 8,  30, 55, 36, 4,  79, 59, 71, 22, 97,  30, 59, 28, 95, 97, 6,  39, 2,  55, 16, 70, 10, 73, 22, 26, 61, 96,
      60, 98, 89, 24, 85, 11, 60, 88, 27, 61, 58,  33, 8,  7,  81, 19, 93, 78, 54, 83, 53, 6,  82, 22, 63, 18, 50, 19,
      83, 88, 26, 50, 1,  48, 1,  51, 69, 84, 64,  74, 46, 17, 47, 66, 60, 85, 12, 46, 68, 31, 16, 75, 91, 22, 61, 16,
      15, 85, 99, 49, 11, 55, 17, 89, 65, 40, 91,  78, 87, 6,  72, 81, 64, 68, 66, 70, 53, 13, 39, 9,  48, 50, 81, 23,
      32, 13, 7,  53, 10, 1,  31, 62, 57, 32, 100, 51, 26, 68, 79, 43, 73, 14, 47, 30, 41, 59, 81, 8,  47, 32, 47, 88,
      67, 16, 72, 92, 59, 59, 88, 49, 55, 83, 50,  93, 5,  11, 63, 83, 32, 9,  51, 63, 9,  87, 54, 25, 99, 1,  47, 55,
      2,  84, 13, 45, 46, 70, 41, 59, 47, 95, 86,  65, 30, 54, 16, 73, 97, 1,  46, 98, 97, 48, 91, 60, 16, 39, 70, 36,
      9,  78, 48, 1,  77, 26, 91, 30, 42, 73, 21,  90, 50, 71, 57, 80, 54, 90, 42, 63,72, 63, 72, 84, 29, 30, 16, 49, 83, 78, 35,  8,  5,  42, 31, 82, 72, 74, 97, 86, 5,  76, 77, 6,  6,  56, 25, 5,
      93, 71, 6,  43, 18, 79, 79, 3,  45, 81, 57,  86, 95, 25, 75, 17, 51, 27, 8,  7,  28, 70, 34, 6,  19, 3,  84, 84,
      77, 6,  7,  98, 45, 57, 72, 93, 90, 49, 50,  87, 35, 98, 79, 4,  55, 10, 32, 61, 29, 93, 41, 31, 88, 93, 0,  37,
      67, 8,  30, 55, 36, 4,  79, 59, 71, 22, 97,  30, 59, 28, 95, 97, 6,  39, 2,  55, 16, 70, 10, 73, 22, 26, 61, 96,
      60, 98, 89, 24, 85, 11, 60, 88, 27, 61, 58,  33, 8,  7,  81, 19, 93, 78, 54, 83, 53, 6,  82, 22, 63, 18, 50, 19,
      83, 88, 26, 50, 1,  48, 1,  51, 69, 84, 64,  74, 46, 17, 47, 66, 60, 85, 12, 46, 68, 31, 16, 75, 91, 22, 61, 16,
      15, 85, 99, 49, 11, 55, 17, 89, 65, 40, 91,  78, 87, 6,  72, 81, 64, 68, 66, 70, 53, 13, 39, 9,  48, 50, 81, 23,
      32, 13, 7,  53, 10, 1,  31, 62, 57, 32, 100, 51, 26, 68, 79, 43, 73, 14, 47, 30, 41, 59, 81, 8,  47, 32, 47, 88,
      67, 16, 72, 92, 59, 59, 88, 49, 55, 83, 50,  93, 5,  11, 63, 83, 32, 9,  51, 63, 9,  87, 54, 25, 99, 1,  47, 55,
      2,  84, 13, 45, 46, 70, 41, 59, 47, 95, 86,  65, 30, 54, 16, 73, 97, 1,  46, 98, 97, 48, 91, 60, 16, 39, 70, 36,
      9,  78, 48, 1,  77, 26, 91, 30, 42, 73, 21,  90, 50, 71, 57, 80, 54, 90, 42, 63,
  };

  example(values, true, true);

  const int number_of_runs = 50000;
  std::vector<std::chrono::nanoseconds> times;
  times.reserve(number_of_runs);
  for (int i = 0; i < number_of_runs; i++) {
    times.push_back(example(values, false, false));
  }

  const auto total = std::accumulate(times.begin(), times.end(), std::chrono::nanoseconds::zero());
  const auto avg = total / times.size();
  std::cout << "Average of " << number_of_runs << " runs with " << values.size() << " each = " << avg.count() / 1000.0 << "us\n";
  return 0;
}