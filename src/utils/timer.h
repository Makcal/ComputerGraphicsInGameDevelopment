#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <utility>

namespace cg::utils {

class timer {
  public:
    explicit timer(std::string message) : event(std::move(message)) {
        start = std::chrono::high_resolution_clock::now();
    }

    timer(const timer&) = delete;
    timer(timer&&) = delete;
    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = delete;

    ~timer() {
        auto stop = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> duration = stop - start;
        std::cout << event << "- took " << duration.count() << "ms\n";
    }

  private:
    std::string event;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

} // namespace cg::utils
