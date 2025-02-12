#include "utils.hpp"

namespace PjPlot {


    template <Colour Val>
        requires (Val < Colour::COUNT)
    [[nodiscard]] constexpr static auto to_string() -> std::string_view {
        if constexpr (Val == Colour::WHITE) {
            return "white";
        } else if constexpr (Val == Colour::BLACK) {
            return "black";
        } else {
            static_assert(always_false<void>::value, "Error: unhandled type");
        }
    }

#ifdef PJPLOT_ENABLE_TESTS
    // little compile-time test to ensure all colour types are handled
    template <size_t I = 0>
    consteval static auto test_colour_to_string() -> bool{
        if constexpr (I < static_cast<size_t>(Colour::COUNT)) {
            constexpr auto s = to_string<static_cast<Colour>(I)>();
            return test_colour_to_string<I+1>();
        }
        return true;
    }
    constexpr static bool to_colour_to_string_test = test_colour_to_string();
#endif

    // runtime version 
    [[nodiscard]] static auto to_string(Colour val) -> std::string_view {
        switch (val) {
            case Colour::WHITE:
                return to_string<Colour::WHITE>();
            case Colour::BLACK:
                return to_string<Colour::BLACK>();
            default:
                // important for catching UB
                throw std::invalid_argument("Error: unsupported colour type");
        }
    }

    // a class to store the image elements for the grid, including lines, labels and ticks
    // each element has a pair of x and y coordinates (representing the top-left corner), and a Mat2 of RGBA values
    // the purpose is to quickly draw the grid on the image without having to iterate over the entire image
    // can support either dynamic memory or static memory for each of the grid elements depending on user requirements
    template <Size2 TitleSize, Size2 LabelSize, Size2 TickSize>
    class GridData {
    public:

    private:
        template <typename T>
        struct GridElement {
            T m_element;
            Vec2<size_t> m_offset;
        };
        static constexpr size_t k_max_num_labels = 2;
        static constexpr size_t k_max_num_ticks = 20;
        GridElement<Mat2<RGBA, TitleSize>> m_title;
        ArrayNd<GridElement<Mat2<RGBA, LabelSize>>, StaticSize1<k_max_num_labels>> m_labels;
        ArrayNd<GridElement<Mat2<RGBA, TickSize>>, StaticSize1<k_max_num_ticks>> m_ticks;
    };

    // a class to store the options for the grid, including whether to show x, y, x labels and y labels
    class GridOptions {
    public:
        constexpr GridOptions() = default;


        constexpr GridOptions(size_t border_pixels, bool show_x, bool show_y, bool show_x_labels, bool show_y_labels, bool show_minor_gridlines, bool show_major_gridlines) 
        : m_border_pixels(border_pixels), m_show_x(show_x), m_show_y(show_y), m_show_x_labels(show_x_labels), m_show_y_labels(show_y_labels), m_show_minor_gridlines(show_minor_gridlines), m_show_major_gridlines(show_major_gridlines) {

        }

        constexpr void set_border_pixels(size_t border_pixels) {
            m_border_pixels = border_pixels;
        }

        constexpr void set_show_x(bool show_x) {
            m_show_x = show_x;
        }

        constexpr void set_show_y(bool show_y) {
            m_show_y = show_y;
        }

        constexpr void set_show_x_labels(bool show_x_labels) {
            m_show_x_labels = show_x_labels;
        }

        constexpr void set_show_y_labels(bool show_y_labels) {
            m_show_y_labels = show_y_labels;
        }

        constexpr void set_show_minor_gridlines(bool show_minor_gridlines) {
            m_show_minor_gridlines = show_minor_gridlines;
        }

        constexpr void set_show_major_gridlines(bool show_major_gridlines) {
            m_show_major_gridlines = show_major_gridlines;
        }

        [[nodiscard]] constexpr auto get_show_x() const noexcept -> bool {
            return m_show_x;
        }

        [[nodiscard]] constexpr auto get_show_y() const noexcept -> bool {
            return m_show_y;
        }

        [[nodiscard]] constexpr auto get_show_x_labels() const noexcept -> bool {
            return m_show_x_labels;
        }

        [[nodiscard]] constexpr auto get_show_y_labels() const noexcept -> bool {
            return m_show_y_labels;
        }

        [[nodiscard]] constexpr auto get_show_minor_gridlines() const noexcept -> bool {
            return m_show_minor_gridlines;
        }

        [[nodiscard]] constexpr auto get_show_major_gridlines() const noexcept -> bool {
            return m_show_major_gridlines;
        }   

    private:

        size_t m_border_pixels = 0;
        bool m_show_x = true;
        bool m_show_y = true;
        bool m_show_x_labels = true;
        bool m_show_y_labels = true;
        bool m_show_minor_gridlines = true;
        bool m_show_major_gridlines = true;
    };  

    class AppearanceOptions {
    public:
        constexpr AppearanceOptions(){}

        [[nodiscard]] constexpr static auto create(Colour background, Colour text) noexcept-> ResultWithValue<AppearanceOptions> {
            if (background < Colour::COUNT && text < Colour::COUNT) {
                return ResultWithValue<AppearanceOptions>(AppearanceOptions(background, text));
            } else {
                return ResultWithValue<AppearanceOptions>::Failure("Error: invalid colour type");
            }
        }
        constexpr void set_background_colour(Colour background) {
            m_background_colour = background;
        }

        constexpr void set_text_colour(Colour text) {
            m_text_colour = text;
        }

        [[nodiscard]] constexpr auto get_background_colour() const noexcept {
            return m_background_colour;
        }

        [[nodiscard]] constexpr auto get_text_colour() const noexcept {
            return m_text_colour;
        }

    private:

        constexpr AppearanceOptions(Colour background, Colour text) 
        : m_background_colour(background), m_text_colour(text) {

        }

        Colour m_background_colour = Colour::WHITE;
        Colour m_text_colour = Colour::BLACK;
    };


    enum class ChartType {
        LINE, BAR, SCATTER, COUNT
    };

    template <ChartType Type>
        requires (Type < ChartType::COUNT) // valid chart type
    class Chart{
    public: 
        class Params {
        public:
            constexpr Params(size_t series_length, size_t num_series) 
            : m_series_length(series_length), m_num_series(num_series) {

            }
        private:
            size_t m_series_length;
            size_t m_num_series;
        };

        template <UnderlyingType ElementType, Size2 OutSize>
        constexpr static auto get_plot(const Mat2<ElementType, OutSize>& plot_data, Params params, const AppearanceOptions&, Img2<OutSize>& img_data) -> void {
            if constexpr (Type == ChartType::LINE) {
                // clear the image
                detail::clear(img_data, m_appearance_options.get_background_colour());

                // get the plot size
                // const auto max_x ...
                // const auto max_y ...

                // draw the grid
                // calculate limits, number of ticks, etc.

                // draw the lines
                for (const auto& row : plot_data) {
                    for (size_t i = 0; i < row.length() - 1; ++i) {
                        const auto curr = row[i];
                        const auto next = row[i+1];
                        // get the x and y coordinates given the normalisation to the plot size

                        // detail::bresenham_line(img_data, pixel, 0, params.series_length, 0, params.series_length);
                    }
                };
                
            } if constexpr (Type == ChartType::BAR) {
                return;
            } else if constexpr (Type == ChartType::SCATTER) {
                return;
            } else {
                static_assert(always_false<void>::value, "Error: unhandled type");
            }
        }

        template <UnderlyingType ElementType, Size2 OutSize>
        [[nodiscard]] constexpr static auto get_plot(const Mat2<ElementType, OutSize>& plot_data, Params params, const AppearanceOptions&, OutSize out_size) -> Img2<OutSize> {
            Img2<OutSize> img_out(out_size);
            get_plot(plot_data, params, m_appearance_options, img_out);
            return img_out;
        }

        struct TypeMapper {
            using type = Params;
        };
    };

    using LineChart = Chart<ChartType::LINE>;
    using ScatterChart = Chart<ChartType::SCATTER>;
    using BarChart = Chart<ChartType::BAR>;

    template <class PlotType>
    class plot_params_t {
    public:
        using type = typename PlotType::TypeMapper::type;
    };

    class Factory {
    public:
        constexpr Factory() {

        }
        template <class PlotType, UnderlyingType ElementType, Size2 OutSize = DynamicSize2>
        [[nodiscard]] constexpr auto get_plot(std::span<const ElementType> plot_data, typename plot_params_t<PlotType>::type params, OutSize output_size) const -> Img2<OutSize> {
            return PlotType::template get_plot<ElementType, OutSize>(plot_data, params, m_appearance_options, output_size);
        }
        
        template <class PlotType, UnderlyingType ElementType, Size2 OutSize = DynamicSize2>
        constexpr auto get_plot(std::span<const ElementType> plot_data, typename plot_params_t<PlotType>::type params, Img2<OutSize>& img_out) const -> void {
            return PlotType::template get_plot<ElementType, OutSize>(plot_data, params, m_appearance_options, img_out);
        }

        [[nodiscard]] constexpr auto get_appearance_options() noexcept -> AppearanceOptions& {
            return m_appearance_options;
        }

    private:
        AppearanceOptions m_appearance_options;
    };

}
