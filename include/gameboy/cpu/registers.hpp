#pragma once

#include "gameboy/types.hpp"

namespace kts::cpu {

class RegisterFile {
      public:
	enum class Register8 : u8 { A, F, B, C, D, E, H, L };

	enum class Register16 : u8 { AF, BC, DE, HL, SP, PC };

	enum class Flag : u8 { Zero = 0x80, Subtract = 0x40, HalfCarry = 0x20, Carry = 0x10 };

	RegisterFile() = default;

	u8 read(Register8 reg) const;
	void write(Register8 reg, u8 value);

	u16 read(Register16 reg) const;
	void write(Register16 reg, u16 value);

	bool flag(Flag flag) const;
	void set_flag(Flag flag, bool value);

      private:
	struct Registers {
		u8 A = 0;
		u8 F = 0;
		u8 B = 0;
		u8 C = 0;
		u8 D = 0;
		u8 E = 0;
		u8 H = 0;
		u8 L = 0;

		u16 SP = 0;
		u16 PC = 0;
	};

	Registers regs_{};
};

} // namespace kts::cpu
