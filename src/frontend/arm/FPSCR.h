/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include "common/bit_util.h"
#include "common/common_types.h"

namespace Dynarmic {
namespace Arm {

/**
 * Representation of the Floating-Point Status and Control Register.
 */
class FPSCR final
{
public:
    enum class RoundingMode {
        ToNearest,
        TowardsPlusInfinity,
        TowardsMinusInfinity,
        TowardsZero
    };

    FPSCR() = default;
    FPSCR(const FPSCR&) = default;
    FPSCR(FPSCR&&) = default;
    explicit FPSCR(u32 data) : value{data} {}

    FPSCR& operator=(const FPSCR&) = default;
    FPSCR& operator=(FPSCR&&) = default;
    FPSCR& operator=(u32 data) {
        value = data;
        return *this;
    }

    /// Negative condition flag.
    bool N() const {
        return Common::Bit<31>(value);
    }

    /// Zero condition flag.
    bool Z() const {
        return Common::Bit<30>(value);
    }

    /// Carry condition flag.
    bool C() const {
        return Common::Bit<29>(value);
    }

    /// Overflow condition flag.
    bool V() const {
        return Common::Bit<28>(value);
    }

    /// Cumulative saturation flag.
    bool QC() const {
        return Common::Bit<27>(value);
    }

    /// Alternate half-precision control flag.
    bool AHP() const {
        return Common::Bit<26>(value);
    }

    /// Default NaN mode control bit.
    bool DN() const {
        return Common::Bit<25>(value);
    }

    /// Flush-to-zero mode control bit.
    bool FTZ() const {
        return Common::Bit<24>(value);
    }

    /// Rounding mode control field.
    RoundingMode RMode() const {
        return static_cast<RoundingMode>(Common::Bits<22, 23>(value));
    }

    /// Indicates the stride of a vector.
    u32 Stride() const {
        return Common::Bits<20, 21>(value) + 1;
    }

    /// Indicates the length of a vector.
    u32 Len() const {
        return Common::Bits<16, 18>(value) + 1;
    }

    /// Input denormal exception trap enable flag.
    bool IDE() const {
        return Common::Bit<15>(value);
    }

    /// Inexact exception trap enable flag.
    bool IXE() const {
        return Common::Bit<12>(value);
    }

    /// Underflow exception trap enable flag.
    bool UFE() const {
        return Common::Bit<11>(value);
    }

    /// Overflow exception trap enable flag.
    bool OFE() const {
        return Common::Bit<10>(value);
    }

    /// Division by zero exception trap enable flag.
    bool DZE() const {
        return Common::Bit<9>(value);
    }

    /// Invalid operation exception trap enable flag.
    bool IOE() const {
        return Common::Bit<8>(value);
    }

    /// Input denormal cumulative exception bit.
    bool IDC() const {
        return Common::Bit<7>(value);
    }

    /// Inexact cumulative exception bit.
    bool IXC() const {
        return Common::Bit<4>(value);
    }

    /// Underflow cumulative exception bit.
    bool UFC() const {
        return Common::Bit<3>(value);
    }

    /// Overflow cumulative exception bit.
    bool OFC() const {
        return Common::Bit<2>(value);
    }

    /// Division by zero cumulative exception bit.
    bool DZC() const {
        return Common::Bit<1>(value);
    }

    /// Invalid operation cumulative exception bit.
    bool IOC() const {
        return Common::Bit<0>(value);
    }

    /**
     * Whether or not the FPSCR indicates RunFast mode.
     *
     * RunFast mode is enabled when:
     *   - Flush-to-zero is enabled
     *   - Default NaNs are enabled.
     *   - All exception enable bits are cleared.
     */
    bool InRunFastMode() const {
        constexpr u32 mask     = 0x03001F00;
        constexpr u32 expected = 0x03000000;

        return (value & mask) == expected;
    }

    /// Gets the underlying raw value within the FPSCR.
    u32 Value() const {
        return value;
    }

private:
    u32 value = 0;
};

inline bool operator==(FPSCR lhs, FPSCR rhs) {
    return lhs.Value() == rhs.Value();
}

inline bool operator!=(FPSCR lhs, FPSCR rhs) {
    return !operator==(lhs, rhs);
}

} // namespace Arm
} // namespace Dynarmic
