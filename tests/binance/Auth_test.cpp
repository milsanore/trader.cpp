#include "binance/Auth.h"

#include <gtest/gtest.h>

#include <string>

TEST(Auth, signPayload) {
  std::string payload =
      "8=FIX.4.4|9=0000113|35=A|49=SPOT|56=BMDWATCH|34=1|52=20250915-03:27:02.028992|98="
      "0|108=30|25037=5a8455c3-bafd-45b3-8c76-fbc17d118531|10=209";
  std::replace(payload.begin(), payload.end(), '|', '\x01');
  const std::vector<unsigned char> seed(32, 1);
  const std::string signature = binance::Auth::signPayload(payload, seed);

  ASSERT_EQ(signature,
            "khyxombd20pcdg2YroiHRztZtM3LqsywFnTjEQdN1W6TK0dN2SQM/"
            "U3fUylIyvVTWbfUzySb7rcS1AyiL8O8Dg");
}
