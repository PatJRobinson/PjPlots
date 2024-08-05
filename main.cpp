#include "PjPlots.h"
#include <iostream>

int main() {
    constexpr size_t k_num_series = 1;
    constexpr size_t k_series_length = 2;
    
    static constexpr std::array<double, 5> arr = {1.0, 2.0, 3.0, 4.0, 5.0};

    const auto img = PjPlot::get_plot<Chart, double>(arr, Chart::Params(k_num_series, k_series_length));
    std::cout << img.to_string() << '\n';
}
