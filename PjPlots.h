#include <array>
#include <vector>
#include <span>
#include <string_view>
#include <variant>
#include <exception>
#include <stdexcept>
#include <stdint.h>

namespace DataType {
    
    using v_AllowedTypes = std::variant<int, uint8_t, uint32_t, float, double>;

    // Concept to constrain T to one of the allowed types in v_AllowedTypes
    // Helper trait to check if a type is in a variant
    template <typename T, typename Variant>
    struct is_in_variant;

    // Specialization to recursively check each type in the variant
    template <typename T, typename... Types>
    struct is_in_variant<T, std::variant<Types...>> 
        : std::disjunction<std::is_same<T, Types>...> {};

    // Convenience variable template
    template <typename T, typename Variant>
    inline constexpr bool is_in_variant_v = is_in_variant<T, Variant>::value;

    template <typename T>
    concept AllowedType = is_in_variant_v<T, v_AllowedTypes>;
    
    // helper function to check if variant contains template parameter
    template <AllowedType T, typename V, size_t I = 0>
    constexpr static auto check_contains() -> bool {
        if constexpr (I < std::variant_size_v<V>) {
            constexpr bool ret = check_contains<T, V, I+1>();
            return std::is_same_v<std::variant_alternative_t<I, V>, T> || ret;
        }
        return false;
    }

    // won't compile unless all invoked types are handled
    template <AllowedType T, size_t I = 0>
    [[nodiscard]] constexpr static auto to_string() -> std::string_view {
        static_assert(check_contains<T, v_AllowedTypes>());
        if constexpr (std::is_same_v<T, int>) {
            return "int";
        } else if constexpr (std::is_same_v<T, uint8_t>) {
            return "uint8_t";
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return "uint32_t";
        } else if constexpr (std::is_same_v<T, float>) {
            return "float";
        } else if constexpr (std::is_same_v<T, double>) {
            return "double";
        } else {
            return to_string<T, I+1>();   
        }
    }

    // little compile-time test to ensure all variant types are handled
    template <size_t I = 0>
    constexpr static auto test_to_string() -> bool{
        if constexpr (I < std::variant_size_v<v_AllowedTypes>) {
            constexpr auto s = to_string<std::variant_alternative_t<I, v_AllowedTypes>>();
            return test_to_string<I+1>();
        }
        return true;
    }
    constexpr static bool is_working = test_to_string();
}

// convenience alias for use outside 'DataType'
template <typename T>
concept UnderlyingType = DataType::AllowedType<T>;

namespace PjPlot {

    class FailureType {
    public:
        constexpr FailureType(){}
        constexpr FailureType(std::string_view message) : m_message(message) {}

        [[nodiscard]] constexpr auto get_message() const noexcept -> std::string_view {
            return m_message;
        }
    private:
        std::string m_message;
    };

    template <typename T>
    class ResultWithValue {
    public:
        constexpr ResultWithValue(T value) : m_value(value) {}
        constexpr ResultWithValue(FailureType&& value) : m_value(std::move(value)) {}
        [[nodiscard]] constexpr static auto Failure(std::string&& msg) noexcept -> ResultWithValue<T>{
            return ResultWithValue<T>(FailureType(std::move(msg)));
        }

        [[nodiscard]] constexpr auto get_value() const noexcept -> T {
            return m_value;
        }

    private:
        std::variant<T, FailureType> m_value;
    };

    template <UnderlyingType T>
    class Mat2{
    public:
        constexpr Mat2(){}
        constexpr Mat2(size_t cols, size_t rows) : m_cols(cols), m_rows(rows), m_data(cols*rows) {

        }

        [[nodiscard]] constexpr auto to_string() const noexcept -> std::string_view{
            return "2d matrix";
        }

        [[nodiscard]] constexpr auto cols() const noexcept -> size_t {
            return m_cols;
        }

        [[nodiscard]] constexpr auto rows() const noexcept -> size_t {
            return m_rows;
        }

        [[nodiscard]] constexpr auto type_s() const noexcept -> std::string_view {
            return DataType::to_string<T>();
        }

    private:
        size_t m_cols;
        size_t m_rows;
        std::vector<T> m_data;
    };

    using Img2 = Mat2<uint8_t>;

    enum class Colour {
        WHITE, BLACK, COUNT
    };

    // won't compile unless all invoked values are handled
    template <Colour Val, size_t I = 0>
    [[nodiscard]] constexpr static auto to_string() noexcept -> std::string_view {
        static_assert(I < static_cast<size_t>(Colour::COUNT));
        if constexpr (Val == Colour::WHITE) {
            return "white";
        } else if constexpr (Val == Colour::BLACK) {
            return "black";
        } else {
            return to_string<Val, I+1>();
        }
    };

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

        template <typename UnderlyingType>
        [[nodiscard]] constexpr static auto get_plot(std::span<UnderlyingType> data, Params params, const AppearanceOptions&) -> Img2 {
            return Img2{};
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

    class Factory {
    public:
        constexpr Factory() {

        }
        template <class PlotType, typename UnderlyingType>
        [[nodiscard]] constexpr auto get_plot(std::span<const UnderlyingType> data, typename plot_params_t<PlotType>::type params) const -> Img2 {
            return PlotType::get_plot(data, params, m_appearance_options);
        }
        
        [[nodiscard]] constexpr auto get_appearance_options() noexcept -> AppearanceOptions& {
            return m_appearance_options;
        }

    private:
        AppearanceOptions m_appearance_options;
    };

}
