#include <gtest/gtest.h>

#include "common/encoding_solidity.hpp"
#include "logger/logger.hpp"
#include "test_util/gtest.hpp"

namespace daily::core_tests {

struct ABITest : BaseTest {};

// based on https://docs.soliditylang.org/en/v0.8.10/abi-spec.html#examples
TEST_F(ABITest, abi_encoding) {
  EXPECT_EQ(dev::fromHex("0xcdcd77c0"), util::EncodingSolidity::getFunction("baz(uint32,bool)"));
  EXPECT_EQ(util::EncodingSolidity::pack(69),
            dev::fromHex("0x0000000000000000000000000000000000000000000000000000000000000045"));
  EXPECT_EQ(util::EncodingSolidity::pack(true),
            dev::fromHex("0x0000000000000000000000000000000000000000000000000000000000000001"));
  EXPECT_EQ(util::EncodingSolidity::pack(addr_t("0x9da168dd6f1e0d4fe15736a65af68d9b9c772a1b")),
            dev::fromHex("0x0000000000000000000000009da168dd6f1e0d4fe15736a65af68d9b9c772a1b"));
  EXPECT_EQ(
      dev::fromHex(
          "0xcdcd77c000000000000000000000000000000000000000000000000000000000000000450000000000000000000000000000000000"
          "0000000000000000000000000000010000000000000000000000009da168dd6f1e0d4fe15736a65af68d9b9c772a1b"),
      util::EncodingSolidity::packFunctionCall("baz(uint32,bool)", 69, true,
                                               addr_t("0x9da168dd6f1e0d4fe15736a65af68d9b9c772a1b")));
}

TEST_F(ABITest, abi_dynamic_encoding) {
  EXPECT_EQ(
      util::EncodingSolidity::pack(dev::asBytes("Hello, world!")),
      dev::fromHex(
          "0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000"
          "000000000000000000000d48656c6c6f2c20776f726c642100000000000000000000000000000000000000"));
  EXPECT_EQ(util::EncodingSolidity::pack(dev::asBytes("")),
            dev::fromHex("0x0000000000000000000000000000000000000000000000000000000000000000"));

  EXPECT_EQ(
      util::EncodingSolidity::pack(dev::fromHex("0xf9aad20feab5c2c3f0d9655fe22e65288d04b8faa925db55dc2d6b0390e8d1192ff"
                                                "5b95dcc5dad1ea0e0e3e96af4c569a76aad5b083dc91e53f4874ee5170d861c")),
      dev::fromHex("0x0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000"
                   "0000000000000000000000000000000000041f9aad20feab5c2c3f0d9655fe22e65288d04b8faa925db55dc2d6b03"
                   "90e8d1192ff5b95dcc5dad1ea0e0e3e96af4c569a76aad5b083dc91e53f4874ee5170d861c0000000000000000000"
                   "0000000000000000000000000000000000000000000"));
  EXPECT_EQ(
      util::EncodingSolidity::packFunctionCall(
          "registerValidator(address,bytes,uint16,string,string)", addr_t("0xb9F0C5b79F0bcd9d4794372a7574923dB7F6E2a1"),
          dev::fromHex("0xf9aad20feab5c2c3f0d9655fe22e65288d04b8faa925db55dc2d6b0390e8d1192f"
                       "f5b95dcc5dad1ea0e0e3e96af4c569a76aad5b083dc91e53f4874ee5170d861c"),
          10, dev::asBytes("test"), dev::asBytes("test")),
      dev::fromHex(
          "0x939fdda8000000000000000000000000b9f0c5b79f0bcd9d4794372a7574923db7f6e2a10000000000000000000000000000000000"
          "0000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000000a00000000000000"
          "000000000000000000000000000000000000000000000001200000000000000000000000000000000000000000000000000000000000"
          "0001600000000000000000000000000000000000000000000000000000000000000041f9aad20feab5c2c3f0d9655fe22e65288d04b8"
          "faa925db55dc2d6b0390e8d1192ff5b95dcc5dad1ea0e0e3e96af4c569a76aad5b083dc91e53f4874ee5170d861c0000000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
          "047465737400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
          "00000000000000000000047465737400000000000000000000000000000000000000000000000000000000"));
}

}  // namespace daily::core_tests

using namespace daily;
int main(int argc, char** argv) {
  daily::static_init();

  auto logging = logger::createDefaultLoggingConfig();
  logging.verbosity = logger::Verbosity::Error;
  addr_t node_addr;
  logging.InitLogging(node_addr);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}