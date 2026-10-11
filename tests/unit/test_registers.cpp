#include <array>
#include <utility>

#include <gtest/gtest.h>

#include "gameboy/cpu/registers.hpp"

namespace {

using kts::cpu::RegisterFile;

TEST(RegisterFileTest, RegistersDefaultToZero) {
	RegisterFile registers;

	constexpr std::array byte_registers{
	    RegisterFile::Register8::A, RegisterFile::Register8::F, RegisterFile::Register8::B,
	    RegisterFile::Register8::C, RegisterFile::Register8::D, RegisterFile::Register8::E,
	    RegisterFile::Register8::H, RegisterFile::Register8::L,
	};
	for (const auto reg : byte_registers) {
		EXPECT_EQ(registers.read(reg), 0);
	}

	constexpr std::array word_registers{
	    RegisterFile::Register16::AF, RegisterFile::Register16::BC,
	    RegisterFile::Register16::DE, RegisterFile::Register16::HL,
	    RegisterFile::Register16::SP, RegisterFile::Register16::PC,
	};
	for (const auto reg : word_registers) {
		EXPECT_EQ(registers.read(reg), 0);
	}
}

TEST(RegisterFileTest, StoresAndReadsEachEightBitRegister) {
	RegisterFile registers;
	constexpr std::array values{
	    std::pair{RegisterFile::Register8::A, u8{0x12}},
	    std::pair{RegisterFile::Register8::F, u8{0xF0}},
	    std::pair{RegisterFile::Register8::B, u8{0x34}},
	    std::pair{RegisterFile::Register8::C, u8{0x56}},
	    std::pair{RegisterFile::Register8::D, u8{0x78}},
	    std::pair{RegisterFile::Register8::E, u8{0x9A}},
	    std::pair{RegisterFile::Register8::H, u8{0xBC}},
	    std::pair{RegisterFile::Register8::L, u8{0xDE}},
	};

	for (const auto &[reg, value] : values) {
		registers.write(reg, value);
		EXPECT_EQ(registers.read(reg), value);
	}
}

TEST(RegisterFileTest, WritesAndReadsRegisterPairs) {
	RegisterFile registers;
	constexpr std::array pairs{
	    std::pair{RegisterFile::Register16::AF, u16{0x12F0}},
	    std::pair{RegisterFile::Register16::BC, u16{0x3456}},
	    std::pair{RegisterFile::Register16::DE, u16{0x789A}},
	    std::pair{RegisterFile::Register16::HL, u16{0xBCDE}},
	};

	for (const auto &[reg, value] : pairs) {
		registers.write(reg, value);
		EXPECT_EQ(registers.read(reg), value);
	}
}

TEST(RegisterFileTest, RegisterPairsComposeAndDecomposeBytes) {
	RegisterFile registers;
	registers.write(RegisterFile::Register8::A, 0x12);
	registers.write(RegisterFile::Register8::F, 0xA0);
	registers.write(RegisterFile::Register8::B, 0x34);
	registers.write(RegisterFile::Register8::C, 0x56);
	registers.write(RegisterFile::Register8::D, 0x78);
	registers.write(RegisterFile::Register8::E, 0x9A);
	registers.write(RegisterFile::Register8::H, 0xBC);
	registers.write(RegisterFile::Register8::L, 0xDE);

	EXPECT_EQ(registers.read(RegisterFile::Register16::AF), 0x12A0);
	EXPECT_EQ(registers.read(RegisterFile::Register16::BC), 0x3456);
	EXPECT_EQ(registers.read(RegisterFile::Register16::DE), 0x789A);
	EXPECT_EQ(registers.read(RegisterFile::Register16::HL), 0xBCDE);

	registers.write(RegisterFile::Register16::BC, 0xA1B2);
	EXPECT_EQ(registers.read(RegisterFile::Register8::B), 0xA1);
	EXPECT_EQ(registers.read(RegisterFile::Register8::C), 0xB2);
}

TEST(RegisterFileTest, StackPointerAndProgramCounterStoreBoundaryValues) {
	RegisterFile registers;

	registers.write(RegisterFile::Register16::SP, 0xFFFF);
	registers.write(RegisterFile::Register16::PC, 0x8000);
	EXPECT_EQ(registers.read(RegisterFile::Register16::SP), 0xFFFF);
	EXPECT_EQ(registers.read(RegisterFile::Register16::PC), 0x8000);

	registers.write(RegisterFile::Register16::SP, 0);
	registers.write(RegisterFile::Register16::PC, 0);
	EXPECT_EQ(registers.read(RegisterFile::Register16::SP), 0);
	EXPECT_EQ(registers.read(RegisterFile::Register16::PC), 0);
}

TEST(RegisterFileTest, MasksLowerNibbleOfFlagRegisterWrites) {
	RegisterFile registers;

	registers.write(RegisterFile::Register8::F, 0xFF);
	EXPECT_EQ(registers.read(RegisterFile::Register8::F), 0xF0);
	EXPECT_EQ(registers.read(RegisterFile::Register16::AF), 0x00F0);

	registers.write(RegisterFile::Register16::AF, 0x12FF);
	EXPECT_EQ(registers.read(RegisterFile::Register8::A), 0x12);
	EXPECT_EQ(registers.read(RegisterFile::Register8::F), 0xF0);
}

TEST(RegisterFileTest, FlagAccessSetsAndClearsIndividualBits) {
	RegisterFile registers;
	constexpr std::array flags{
	    RegisterFile::Flag::Zero,
	    RegisterFile::Flag::Subtract,
	    RegisterFile::Flag::HalfCarry,
	    RegisterFile::Flag::Carry,
	};

	for (const auto flag : flags) {
		EXPECT_FALSE(registers.flag(flag));
		registers.set_flag(flag, true);
		EXPECT_TRUE(registers.flag(flag));
		registers.set_flag(flag, false);
		EXPECT_FALSE(registers.flag(flag));
	}
}

TEST(RegisterFileTest, FlagUpdatesPreserveOtherFlagsAndKeepUnusedBitsClear) {
	RegisterFile registers;

	registers.set_flag(RegisterFile::Flag::Zero, true);
	registers.set_flag(RegisterFile::Flag::Carry, true);
	registers.set_flag(RegisterFile::Flag::Subtract, true);
	registers.set_flag(RegisterFile::Flag::Carry, false);

	EXPECT_TRUE(registers.flag(RegisterFile::Flag::Zero));
	EXPECT_TRUE(registers.flag(RegisterFile::Flag::Subtract));
	EXPECT_FALSE(registers.flag(RegisterFile::Flag::HalfCarry));
	EXPECT_FALSE(registers.flag(RegisterFile::Flag::Carry));
	EXPECT_EQ(registers.read(RegisterFile::Register8::F) & 0x0F, 0);
}

TEST(RegisterFileTest, InvalidRegister8OperationsAbortWithDiagnostic) {
	RegisterFile registers;
	constexpr auto invalid = static_cast<RegisterFile::Register8>(0xFF);

	EXPECT_DEATH(registers.read(invalid), "Invalid Register8 value: 255");
	EXPECT_DEATH(registers.write(invalid, 0), "Invalid Register8 value: 255");
}

TEST(RegisterFileTest, InvalidRegister16OperationsAbortWithDiagnostic) {
	RegisterFile registers;
	constexpr auto invalid = static_cast<RegisterFile::Register16>(0xFF);

	EXPECT_DEATH(registers.read(invalid), "Invalid Register16 value: 255");
	EXPECT_DEATH(registers.write(invalid, 0), "Invalid Register16 value: 255");
}

TEST(RegisterFileTest, InvalidFlagOperationsAbortWithDiagnostic) {
	RegisterFile registers;
	constexpr auto invalid = static_cast<RegisterFile::Flag>(0xFF);

	EXPECT_DEATH(registers.flag(invalid), "Invalid Flag value: 255");
	EXPECT_DEATH(registers.set_flag(invalid, true), "Invalid Flag value: 255");
}

} // namespace
