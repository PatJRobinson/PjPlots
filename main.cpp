#include "PjPlots.h"
#include <iostream>

int main() {
    constexpr size_t k_num_series = 1;
    constexpr size_t k_series_length = 2;
    
    // we can fill a constexpr array using an immediately returning lambda
    static constexpr std::array<double, 5> arr = []()->std::array<double, 5>{
        std::array<double, 5> arr;
        size_t count = 0;
        for (auto & val : arr) {
            val = static_cast<double>(count++);
        }
        return arr;
    }();

    std::cout << "Array values are: \n";
    for (const auto & val : arr) {
        std::cout << val << '\n';
    }
    
    PjPlot::Factory builder;
    builder.get_appearance_options().set_background_colour(PjPlot::Colour::BLACK);
    builder.get_appearance_options().set_text_colour(PjPlot::Colour::WHITE);
    const auto img = builder.get_plot<PjPlot::Chart, double>(arr, PjPlot::Chart::Params(k_num_series, k_series_length));
    std::cout << "I am a " << img.to_string() << ", my underlying type is: " << img.type_s() << '\n';
    const auto img2 = img;

    std::cout << "Background colour is: " << PjPlot::to_string(builder.get_appearance_options().get_background_colour()) << "\n";
    std::cout << "Text colour is: " << PjPlot::to_string(builder.get_appearance_options().get_text_colour()) << "\n";

    // allows compile time checking!
    static_assert(img.type_s() == img2.type_s());
}
