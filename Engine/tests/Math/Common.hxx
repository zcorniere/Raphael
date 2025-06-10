template <typename T>
struct TEpsilon;

template <>
struct TEpsilon<float>
{
    static constexpr float Value = 1e-5f;
};

template <>
struct TEpsilon<double>
{
    static constexpr double Value = 1e-12;
};
