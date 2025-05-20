#ifndef _Timer_hpp_
#define _Timer_hpp_

#include <iostream>
#include <chrono>
#include <string>

using namespace std;

class Timer {
  using high_clock = chrono::high_resolution_clock;

 public:
  Timer() : start_time(high_clock::now()) {}

  void reset() { start_time = high_clock::now(); }

  // 返回以毫秒为单位的计时结果
  double elapsedMilliseconds() const {
    return chrono::duration<double, std::milli>(high_clock::now() - start_time).count();
  }

  // 返回以秒为单位的计时结果
  double elapsedSeconds() const {
    return chrono::duration<double>(high_clock::now() - start_time).count();
  }
  
  double elapsedMicroseconds() const {
    return chrono::duration<double, std::micro>(high_clock::now() - start_time).count();
  }

 private:
  chrono::time_point<high_clock> start_time;
};

class ModuleTimer {
  using high_clock = chrono::high_resolution_clock;

 public:
  ModuleTimer()
      : m_total_time(0.0),
        m_min_time(std::numeric_limits<double>::max()),
        m_max_time(0.0),
        m_count(0) {}

  void start() { m_start_time_point = high_clock::now(); }

  void stop() {
    auto end_time_point = high_clock::now();
    auto start_time =
        chrono::time_point_cast<chrono::microseconds>(m_start_time_point).time_since_epoch();
    auto end = chrono::time_point_cast<chrono::microseconds>(end_time_point).time_since_epoch();

    double elapsed_time = static_cast<double>((end - start_time).count());  // 微秒

    // 更新总时间
    m_total_time += elapsed_time;

    // 更新最大、最小时间
    if (elapsed_time < m_min_time) m_min_time = elapsed_time;
    if (elapsed_time > m_max_time) m_max_time = elapsed_time;

    // 增加执行次数
    m_count++;
  }

 void info_str() const {
    cout << " - total time: " << m_total_time << " "<< endl;
    cout << " - min time: " << m_min_time << " "<< endl;
    cout << " - max time: " << m_max_time << " "<< endl;
    cout << " - count: " << m_count << " "<< endl;

    // return (std::format(
    //             "-- tot time : %.6f ms\n-- min time : %.6f ms\n-- max time : %.6f ms\n-- run count "
    //             ": %llu\n") %
    //         m_total_time % m_min_time % m_max_time % m_count).str();
  }

  double total_time() const { return m_total_time; }
  double min_time() const { return m_min_time; }
  double max_time() const { return m_max_time; }
  uint64_t count() const { return m_count; }

 private:
  std::chrono::time_point<high_clock> m_start_time_point;
  double m_total_time;
  double m_min_time;
  double m_max_time;
  uint64_t m_count;
};

#endif