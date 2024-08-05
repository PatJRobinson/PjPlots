#include "PjPlots.h"
#include <iostream>
#include <cmath>

int main() {
    constexpr size_t k_num_series = 5;
    constexpr size_t k_series_length = 1024;
    constexpr size_t k_data_size = k_num_series*k_series_length;
    
    // we can fill a constexpr array using an immediately returning lambda
    static constexpr std::array<double, k_data_size> arr = []()->std::array<double, k_data_size>{
        std::array<double, k_data_size> arr;
        size_t count = 0;
        auto it = arr.begin();
        for (auto & val : arr) {
            const auto sample_idx = count % k_series_length;
            const auto series_idx = count / k_series_length;
            val = static_cast<double>(series_idx + 1 + static_cast<double>(sample_idx/static_cast<double>(k_series_length)));
            ++count;
        }
        return arr;
    }();

    // std::cout << "Array values are: \n";
    // for (const auto & val : arr) {
    //     std::cout << val << '\n';
    // }
    
    PjPlot::Factory builder;
    builder.get_appearance_options().set_background_colour(PjPlot::Colour::BLACK);
    builder.get_appearance_options().set_text_colour(PjPlot::Colour::WHITE);
    const auto img = builder.get_plot<PjPlot::Chart, double>(arr, PjPlot::Chart::Params(k_num_series, k_series_length));
    std::cout << "I am a " << img.to_string() << ", my underlying type is: " << img.type_s() << '\n';
    const auto img2 = img;

    std::cout << "Background colour is: " << PjPlot::to_string(builder.get_appearance_options().get_background_colour()) << "\n";
    std::cout << "Text colour is: " << PjPlot::to_string(builder.get_appearance_options().get_text_colour()) << "\n";

    try {
        std::cout << "Test exception handling: " << PjPlot::to_string(static_cast<PjPlot::Colour>(3));
    } catch (std::runtime_error& e) {
        std::cout << e.what() << '\n';
    }
    // allows compile time checking!
    static_assert(img.type_s() == img2.type_s());

    std::cout << "Program completed successfully\n";
}
