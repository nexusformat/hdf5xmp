#include "base64.h"
#include <bitset>
#include <catch.hpp>
#include <fstream>
#include <iostream>

#include "thumbnailerCore.h"
#define testImage                                                              \
  "iVBORw0KGgoAAAANSUhEUgAAACQAAAAWCAQAAAACu/a1AAAAbUlEQVQ4y+WUsRKAMAhDc+f/"   \
  "f7BLcVQkQWqZtN0CfUcKLaxp4dMg0F07fGZ2gphsfSBWndekB8+"                        \
  "PSZgHxeRrfNvvirTm74qrZRAyK2MJ5CMvrOm2lytiamxFOkfP7YectmT4IBSbflS//"         \
  "9h61gEtfkIBMQGMAwAAAABJRU5ErkJggg=="

TEST_CASE("Remove XMP Headers", "[regex, headers, removeXMPHeaders]") {
  // Test if the removeXMPHeaders function reliably removes the headers.

  // Test with mixed quotes and apostrophes, U+FEFF and end=r
  std::string xmp = "<?xpacket begin=\"\uFEFF' id=\"W5M0MpCehiHzreSzNTczkc9d'?>"
                    "FirstTestString"
                    "<?xpacket end=\"r'?>";

  REQUIRE(removeXMPHeaders(xmp.c_str()) == "FirstTestString");

  // Test with only quotes, no U+FEFF and end=w
  xmp = "<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>"
        "SecondTestString"
        "<?xpacket end=\"w\"?>";

  REQUIRE(removeXMPHeaders(xmp.c_str()) == "SecondTestString");
}

SCENARIO("Test header recognition",
         "[ifstream, check_header, header_exists, read_size]") {

  std::ifstream stream("testFile", std::ifstream::binary);
  REQUIRE(stream.good());

  GIVEN("the file is open") {
    WHEN("at the header of our data block") {
      THEN("it should be recognized") {
        REQUIRE(check_header(stream, 0, XMP_OUR_MAGIC));
        REQUIRE(header_exists(stream, 0));
        AND_THEN("it should read the size") {
          REQUIRE(read_size(stream, 0) == 53160);
        }
      }
    }
    WHEN("at the header of another data block") {
      THEN("it should recognize it as a header but not as ours") {
        REQUIRE(!check_header(stream, 16, XMP_OUR_MAGIC));
        REQUIRE(header_exists(stream, 16));
        AND_THEN("it should read the size") {
          REQUIRE(read_size(stream, 16) == 116);
        }
      }
    }
    WHEN("somewhere with no header") {
      THEN("it should recognize it as not being a header") {
        REQUIRE(!header_exists(stream, 32));
        AND_THEN("it should be able to read a 0 size") {
          REQUIRE(read_size(stream, 32) == 0);
        }
      }
    }
  }
  stream.close();
}

SCENARIO("Reading xmp sidecar",
         "[sidecar, readImageFromXMPBySize, base64_decode, findImageData]") {
  GIVEN("the xmp file exists") {
    WHEN("reading the xmp file") {
      THEN("it should be able to read the image data") {
        std::string data = readXmpFromSidecar("test.hdf5");
        // Put the result of the comparison into a variable to not put png data
        // into the test output
        bool comparison = data == base64_decode(testImage);
        REQUIRE(comparison);
      }
    }
  }
}

SCENARIO("Reading hdf5 file", "[hdf5, thumbnail in middle, readFromHdfFile]") {
  GIVEN("the hdf5 files exist") {
    WHEN("the xmp block is at the beginning") {
      THEN("it should read the data from the xmp block") {
        std::string data = readFromHdfFile("test1.hdf5");
        bool comparison = data == base64_decode(testImage);
        REQUIRE(comparison);
      }
    }
    WHEN("the xmp block is in the middle") {
      THEN("it should skip the first block and read the data from the xmp "
           "block") {
        std::string data = readFromHdfFile("test2.hdf5");
        bool comparison = data == base64_decode(testImage);
        REQUIRE(comparison);
      }
    }
    WHEN("the xmp block is at the end") {
      THEN("it should skip over the other data blocks and read the data from "
           "the xmp block") {
        std::string data = readFromHdfFile("test3.hdf5");
        bool comparison = data == base64_decode(testImage);
        REQUIRE(comparison);
      }
    }
  }
}