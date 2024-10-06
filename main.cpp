#include "PjPlots.h"
#include <iostream>
#include <cmath>

constexpr size_t k_num_series = 5;
constexpr size_t k_series_length = 1024;
constexpr size_t k_data_size = k_num_series*k_series_length;

static consteval auto get_test_data()->std::array<double, k_data_size>{
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
}

int main() {
    
    // we can fill a constexpr array using an immediately returning lambda
    static constexpr std::array<double, k_data_size> arr = get_test_data();

    // std::cout << "Array values are: \n";
    // for (const auto & val : arr) {
    //     std::cout << val << '\n';
    // }

    PjPlot::Mat2<double, PjPlot::DynamicSize2> mat(PjPlot::DynamicSize2(600, 600));

    // test iterators
    for (auto & val : mat) {
        val = 0.0;
    }

    double test = 0;
    for (const auto & val : mat) {
        test = val;
    }

    static_assert(std::is_same_v<PjPlot::StorageType<uint8_t, PjPlot::StaticSize2<600, 600>>, std::array<uint8_t, 360000>>);
    static_assert(std::is_same_v<PjPlot::StorageType<uint8_t, PjPlot::DynamicSize2>, std::vector<uint8_t>>);

    if constexpr (std::is_same_v<PjPlot::StorageType<uint8_t, PjPlot::StaticSize2<600, 600>>, std::array<uint8_t, 360000>>) {
        std::cout << "Static sized array is working\n";
    } else {
        std::cout << "Static sized array is not working\n";
    }

    // test FailureType
    try {
        auto test_opt = PjPlot::AppearanceOptions::create(PjPlot::Colour::COUNT, PjPlot::Colour::WHITE);
        auto& test = test_opt.get_value();
    } catch (const PjPlot::UnhandledFailureException& e) {
        std::cout << "caught unhandled failure exception: " << e.what() << '\n';
    }

    constexpr auto test_appearance_opt = PjPlot::AppearanceOptions::create(PjPlot::Colour::BLACK, PjPlot::Colour::WHITE);
    
    PjPlot::Factory builder;
    builder.get_appearance_options().set_background_colour(PjPlot::Colour::BLACK);
    builder.get_appearance_options().set_text_colour(PjPlot::Colour::WHITE);
    const auto img = builder.get_plot<PjPlot::LineChart, double>(arr, PjPlot::LineChart::Params(k_num_series, k_series_length), PjPlot::StaticSize2<600, 600>{});
    const auto img_dynamic = builder.get_plot<PjPlot::LineChart, double>(arr, PjPlot::LineChart::Params(k_num_series, k_series_length), PjPlot::DynamicSize2(600, 600));
    // const auto img = builder.get_plot<PjPlot::Chart, double>(arr, PjPlot::Chart::Params(k_num_series, k_series_length));
    std::cout << "I am a " << img.to_string() << ", my underlying type is: " << img.type_s() << '\n';
    const auto img2 = img;

    std::cout << "Background colour is: " << PjPlot::to_string(builder.get_appearance_options().get_background_colour()) << "\n";
    std::cout << "Text colour is: " << PjPlot::to_string(builder.get_appearance_options().get_text_colour()) << "\n";

    try {
        std::cout << "Test exception handling: " << PjPlot::to_string(static_cast<PjPlot::Colour>(3));
    } catch (std::invalid_argument& e) {
        std::cout << e.what() << '\n';
    }
    // allows compile time checking!
    static_assert(img.type_s() == img2.type_s());

    std::cout << "Program completed successfully\n";
}
