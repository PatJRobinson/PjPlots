#include <array>
#include <span>
#include <string_view>

class Mat{
public:
    [[nodiscard]] constexpr static auto to_string() -> std::string_view{
        return "I am a matrix of bytes";
    }
};

class Chart{
public: 
    class Params {
    public:
        Params(size_t, size_t) {}
    private:
        size_t m_series_length;
        size_t m_num_series;
    };

    template <typename UnderlyingType>
    [[nodiscard]] constexpr static auto get_plot(std::span<UnderlyingType> data, Params params) -> Mat {
        return Mat{};
    }
    struct TypeMapper {
        using type = Params;
    };
};

template <class PlotType>
class plot_params_t {
public:
    using type = typename PlotType::TypeMapper::type;
};

class PjPlot {
public:
    template <class PlotType, typename UnderlyingType>
    [[nodiscard]] constexpr static auto get_plot(std::span<const UnderlyingType> data, plot_params_t<PlotType>::type params) -> Mat {
        return PlotType::get_plot(data, params);
    }
};

