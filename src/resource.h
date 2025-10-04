#pragma once

#include <linalg.h>

#include <vector>

namespace cg {
using namespace linalg::aliases;

template <typename T>
class resource {
  public:
    static constexpr std::size_t kItemSize = sizeof(T);

    explicit resource(std::size_t size);
    resource(std::size_t x_size, std::size_t y_size);

    T* get_data();
    const T* get_data() const;

    T& item(std::size_t item);
    const T& item(std::size_t item) const;

    T& item(std::size_t x, std::size_t y);
    const T& item(std::size_t x, std::size_t y) const;

    [[nodiscard]] std::size_t count() const;
    [[nodiscard]] std::size_t get_stride() const;

  private:
    std::vector<T> data;
    std::size_t stride;
};

struct color {
    float r;
    float g;
    float b;

    static color from_float3(const float3& in);
    [[nodiscard]] float3 to_float3() const;
};

struct unsigned_color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;

    static unsigned_color from_color(const color& color);
    static unsigned_color from_float3(const float3& color);
    [[nodiscard]] float3 to_float3() const;
};

struct vertex {
    float3 v;
    float3 n;
    float2 tex;
    float3 ambient;
    float3 diffuse;
    float3 emissive;
};

} // namespace cg

namespace cg {
template <typename T>
inline resource<T>::resource(std::size_t size) : data(size), stride{0} {}

template <typename T>
inline resource<T>::resource(std::size_t x_size, std::size_t y_size) : data(x_size * y_size), stride{x_size} {}

template <typename T>
inline T* resource<T>::get_data() {
    return data.data();
}

template <typename T>
inline const T* resource<T>::get_data() const {
    return data.data();
}

template <typename T>
inline T& resource<T>::item(std::size_t item) {
    return data[item];
}

template <typename T>
inline const T& resource<T>::item(std::size_t item) const {
    return data[item];
}

template <typename T>
inline T& resource<T>::item(std::size_t x, std::size_t y) {
    return data[x + (stride * y)];
}

template <typename T>
inline const T& resource<T>::item(std::size_t x, std::size_t y) const {
    return data[x + (stride * y)];
}

template <typename T>
inline std::size_t resource<T>::count() const {
    return data.size();
}

template <typename T>
inline std::size_t resource<T>::get_stride() const {
    return stride;
}

inline color color::from_float3(const float3& in) {
    return {in.x, in.y, in.z};
};

inline float3 color::to_float3() const {
    return {r, g, b};
}

inline unsigned_color unsigned_color::from_color(const color& color) {
    static constexpr std::uint8_t kMin = 0;
    static constexpr std::uint8_t kMax = 255;
    float3 clamped = linalg::clamp(color.to_float3() * kMax, kMin, kMax);
    return {
        static_cast<std::uint8_t>(clamped.x),
        static_cast<std::uint8_t>(clamped.y),
        static_cast<std::uint8_t>(clamped.z),
    };
};

inline unsigned_color unsigned_color::from_float3(const float3& color) {
    static constexpr std::uint8_t kMin = 0;
    static constexpr std::uint8_t kMax = 255;
    float3 clamped = linalg::clamp(color * kMax, kMin, kMax);
    return {
        static_cast<std::uint8_t>(clamped.x),
        static_cast<std::uint8_t>(clamped.y),
        static_cast<std::uint8_t>(clamped.z),
    };
};

inline float3 unsigned_color::to_float3() const {
    static constexpr std::uint8_t kMax = 255;
    return float3{
        static_cast<float>(r) / kMax,
        static_cast<float>(b) / kMax,
        static_cast<float>(g) / kMax,
    };
};
} // namespace cg
