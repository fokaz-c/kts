#include "gameboy/cpu/registers.hpp"

#include <cstdio>
#include <cstdlib>

namespace kts::cpu {
namespace {

[[noreturn]] void invalid_register(const char *kind, int value) {
	std::fprintf(stderr, "Invalid %s value: %d\n", kind, value);
	std::abort();
}

} // namespace

u8 RegisterFile::read(Register8 reg) const {
	switch (reg) {
	case Register8::A:
		return regs_.A;
	case Register8::F:
		return regs_.F;
	case Register8::B:
		return regs_.B;
	case Register8::C:
		return regs_.C;
	case Register8::D:
		return regs_.D;
	case Register8::E:
		return regs_.E;
	case Register8::H:
		return regs_.H;
	case Register8::L:
		return regs_.L;
	}
	invalid_register("Register8", static_cast<int>(reg));
}

u16 RegisterFile::read(Register16 reg) const {
	switch (reg) {
	case Register16::AF:
		return static_cast<u16>((static_cast<u16>(regs_.A) << 8) | regs_.F);
	case Register16::BC:
		return static_cast<u16>((static_cast<u16>(regs_.B) << 8) | regs_.C);
	case Register16::DE:
		return static_cast<u16>((static_cast<u16>(regs_.D) << 8) | regs_.E);
	case Register16::HL:
		return static_cast<u16>((static_cast<u16>(regs_.H) << 8) | regs_.L);
	case Register16::SP:
		return regs_.SP;
	case Register16::PC:
		return regs_.PC;
	}

	invalid_register("Register16", static_cast<int>(reg));
}

void RegisterFile::write(Register8 reg, u8 value) {
	switch (reg) {
	case Register8::A:
		regs_.A = value;
		return;
	case Register8::F:
		regs_.F = static_cast<u8>(value & 0xF0);
		return;
	case Register8::B:
		regs_.B = value;
		return;
	case Register8::C:
		regs_.C = value;
		return;
	case Register8::D:
		regs_.D = value;
		return;
	case Register8::E:
		regs_.E = value;
		return;
	case Register8::H:
		regs_.H = value;
		return;
	case Register8::L:
		regs_.L = value;
		return;
	}
	invalid_register("Register8", static_cast<int>(reg));
}

void RegisterFile::write(Register16 reg, u16 value) {
	switch (reg) {
	case Register16::AF:
		regs_.A = static_cast<u8>(value >> 8);
		regs_.F = static_cast<u8>(value & 0xF0);
		return;
	case Register16::BC:
		regs_.B = static_cast<u8>(value >> 8);
		regs_.C = static_cast<u8>(value & 0xFF);
		return;
	case Register16::DE:
		regs_.D = static_cast<u8>(value >> 8);
		regs_.E = static_cast<u8>(value & 0xFF);
		return;
	case Register16::HL:
		regs_.H = static_cast<u8>(value >> 8);
		regs_.L = static_cast<u8>(value & 0xFF);
		return;
	case Register16::SP:
		regs_.SP = value;
		return;
	case Register16::PC:
		regs_.PC = value;
		return;
	}
	invalid_register("Register16", static_cast<int>(reg));
}

bool RegisterFile::flag(Flag flag) const {
	switch (flag) {
	case Flag::Zero:
	case Flag::Subtract:
	case Flag::HalfCarry:
	case Flag::Carry:
		return (regs_.F & static_cast<u8>(flag)) != 0;
	}
	invalid_register("Flag", static_cast<int>(flag));
}

void RegisterFile::set_flag(Flag flag, bool value) {
	switch (flag) {
	case Flag::Zero:
	case Flag::Subtract:
	case Flag::HalfCarry:
	case Flag::Carry:
		break;
	default:
		invalid_register("Flag", static_cast<int>(flag));
	}
	const auto mask = static_cast<u8>(flag);
	if (value) {
		regs_.F = static_cast<u8>(regs_.F | mask);
	} else {
		regs_.F = static_cast<u8>(regs_.F & static_cast<u8>(~mask));
	}
}

} // namespace kts::cpu
