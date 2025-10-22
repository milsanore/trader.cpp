#include "binance/auth.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

std::filesystem::path path_ = std::filesystem::absolute(__FILE__);

TEST(Auth, sign_payload) {
  std::string payload =
      "8=FIX.4.4|9=0000113|35=A|49=SPOT|56=BMDWATCH|34=1|52=20250915-03:27:02.028992|98="
      "0|108=30|25037=5a8455c3-bafd-45b3-8c76-fbc17d118531|10=209";
  std::replace(payload.begin(), payload.end(), '|', '\x01');
  const std::vector<unsigned char> seed(32, 1);
  const std::string signature = binance::Auth::sign_payload(payload, seed);

  ASSERT_EQ(signature,
            "khyxombd20pcdg2YroiHRztZtM3LqsywFnTjEQdN1W6TK0dN2SQM/"
            "U3fUylIyvVTWbfUzySb7rcS1AyiL8O8Dg");
}

TEST(AuthTest, GetSeedFromPem_ValidKey_ReturnsCorrectSeed) {
  std::string api_key = "dummy";
  std::filesystem::path pem_path =
      path_.parent_path() / "test_resources" / "valid_key.pem";
  std::string pem_path_str = pem_path.string();

  binance::Auth auth(api_key, pem_path_str);
  auto seed = auth.get_seed_from_pem();

  // Expected seed bytes hardcoded here
  std::vector<unsigned char> expected_seed = {
      0x82, 0x44, 0x61, 0x6b, 0x46, 0x06, 0xb8, 0x40, 0x0a, 0x66, 0xfd,
      0x0e, 0xfc, 0xbe, 0xa9, 0xaf, 0x16, 0x11, 0xfd, 0x25, 0x40, 0xe9,
      0x75, 0xb3, 0x80, 0x8b, 0x20, 0x00, 0x7d, 0x9b, 0xcf, 0x6e};

  EXPECT_EQ(seed.size(), expected_seed.size());
  EXPECT_TRUE(std::equal(seed.begin(), seed.end(), expected_seed.begin()));
}

TEST(AuthTest, GetSeedFromPem_InvalidFile_Throws) {
  std::string api_key = "dummy";
  std::filesystem::path pem_path =
      path_.parent_path() / "test_resources" / "nonexistent.pem";
  std::string pem_path_str = pem_path.string();

  binance::Auth auth(api_key, pem_path_str);

  EXPECT_THROW(auth.get_seed_from_pem(), std::runtime_error);
}

TEST(AuthTest, GetSeedFromPem_InvalidKey_Throws) {
  std::string api_key = "dummy";
  std::filesystem::path pem_path =
      path_.parent_path() / "test_resources" / "invalid_key.pem";
  std::string pem_path_str = pem_path.string();

  binance::Auth auth(api_key, pem_path_str);

  EXPECT_THROW(auth.get_seed_from_pem(), std::runtime_error);
}
