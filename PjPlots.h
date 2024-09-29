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

    template <size_t Rows, size_t Cols>
    struct StaticSize2 {
        using is_static_size = std::true_type;
        using is_size_type = std::true_type;
        static constexpr size_t rows() {return Rows;}
        static constexpr size_t cols() {return Cols;}
        static constexpr size_t dims = 2;
    };

    struct DynamicSize2 {
        static constexpr bool is_dynamic = true;
        using is_static_size = std::false_type;
        using is_size_type = std::true_type;

        constexpr DynamicSize2(size_t rows, size_t cols) : m_rows(rows), m_cols(cols) {}

        size_t rows() {return m_rows;}
        size_t cols() {return m_cols;}
        size_t m_rows = 0;
        size_t m_cols = 0;
        static constexpr size_t dims = 2;
    };
    
    // Define a concept that checks if a type is a size type
    template <typename T>
    concept Size2 = requires {
        typename T::is_size_type;  // Ensures the type has the is_size_type trait
        { std::declval<T>().cols() } -> std::same_as<size_t>;
        { std::declval<T>().rows() } -> std::same_as<size_t>;
        T::dims == 2;
    };

    using DynamcSize = StaticSize2<0, 0>;

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

    template <typename>
    struct always_false : std::false_type {};

    template <Size2 Size>
    constexpr auto get_static_nele() -> size_t {
        if constexpr (Size::is_static_size::value) {
            return Size::rows()*Size::cols();
        } else {
            return 0;
        }
    }

    // Type trait to select either std::array or std::vector based on static size
    template <typename T, Size2 Size>
    using StorageType = std::conditional_t<
        (Size::is_static_size::value),
        std::array<T, get_static_nele<Size>()>, // Static size
        std::vector<T>              // Dynamic size
    >;

    template <typename T, Size2 Size>
    class Mat2Impl {
        // Helper trait to check if Rows and Cols are greater than zero
        using is_static_size = Size::is_static_size;

    public:
        using is_matrix_type = std::true_type;

        // SFINAE constructor for when a static size type is provided
        constexpr Mat2Impl(Size size) noexcept 
        requires is_static_size::value : m_size(size), m_data() {}

        // SFINAE constructor for when a dynamic size type is provided
        constexpr Mat2Impl(Size size) noexcept 
        requires (!is_static_size::value) : m_size(size), m_data(std::vector<T>(size.cols() * size.rows())) {}

        [[nodiscard]] constexpr auto to_string() const noexcept -> std::string_view {
            return "2d matrix";
        }

        // Getter for the number of columns
        [[nodiscard]] constexpr auto cols() const noexcept -> size_t {
            return m_size.cols();
        }

        // Getter for the number of rows
        [[nodiscard]] constexpr auto rows() const noexcept -> size_t {
            return m_size.rows();
        }

        // Getter for the type as a string (using DataType::to_string)
        [[nodiscard]] constexpr auto type_s() const noexcept -> std::string_view {
            return DataType::to_string<T>();
        }

        // Const getter for matrix data as std::span
        [[nodiscard]] auto data() const noexcept -> std::span<const T> {
            return std::span<const T>(m_data.data(), m_data.size());
        }

        // Non-const getter for matrix data as std::span
        [[nodiscard]] auto data() noexcept -> std::span<T> {
            return std::span<T>(m_data.data(), m_data.size());
        }

        // Element-wise access operator (non-const)
        [[nodiscard]] auto operator()(size_t col, size_t row) noexcept -> T& {
            size_t idx = row * m_size.cols() + col;
            return data()[idx];
        }

        // Element-wise access operator (const)
        [[nodiscard]] auto operator()(size_t col, size_t row) const noexcept -> const T& {
            size_t idx = row * m_size.cols() + col;
            return data()[idx];
        }

    private:
        Size m_size;
        StorageType<T, Size> m_data;
    };

    // Define a concept that checks if a type is a matrix type
    template <typename T>
    concept Mat2 = requires {
        typename T::is_matrix_type;  // Ensures the type has the is_matrix_type trait
        { std::declval<T>().rows() } -> std::same_as<size_t>;
        { std::declval<T>().cols() } -> std::same_as<size_t>;
        { std::declval<T>().data() } -> std::same_as<std::span<const typename T::value_type>>;
    };

    // Alias for an image type (e.g., uint8_t matrix)
    template <Size2 Size>
    using Img2 = Mat2Impl<uint8_t, Size>;

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

        template <typename UnderlyingType, Size2 OutSize>
        [[nodiscard]] constexpr static auto get_plot(std::span<const UnderlyingType> data, Params params, const AppearanceOptions&, OutSize out_size) -> Img2<OutSize> {
            return Img2<OutSize>(out_size);
        }

        template <typename UnderlyingType, Size2 OutSize>
        constexpr static auto get_plot(std::span<const UnderlyingType> data, Params params, const AppearanceOptions&, Img2<OutSize>&) -> void {
            return;
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
        template <class PlotType, typename UnderlyingType, Size2 OutSize = DynamcSize>
        [[nodiscard]] constexpr auto get_plot(std::span<const UnderlyingType> data, typename plot_params_t<PlotType>::type params, OutSize output_size) const -> Img2<OutSize> {
            return PlotType::template get_plot<UnderlyingType, OutSize>(data, params, m_appearance_options, output_size);
        }
        
        template <class PlotType, typename UnderlyingType, Size2 OutSize = DynamcSize>
        constexpr auto get_plot(std::span<const UnderlyingType> data, typename plot_params_t<PlotType>::type params, Img2<OutSize>&, OutSize output_size) const -> void {
            return;
        }

        [[nodiscard]] constexpr auto get_appearance_options() noexcept -> AppearanceOptions& {
            return m_appearance_options;
        }

    private:
        AppearanceOptions m_appearance_options;
    };

}
