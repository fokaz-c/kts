#pragma once

#include <cstdint>

namespace gameboy {

class CycleCount {
public:
    explicit constexpr CycleCount(std::uint32_t m_cycles) noexcept
        : m_cycles_(m_cycles) {}

    [[nodiscard]] constexpr std::uint32_t m_cycles() const noexcept {
        return m_cycles_;
    }

private:
    std::uint32_t m_cycles_;
};

}  // namespace gameboy
