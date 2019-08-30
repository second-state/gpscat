#include "catch.hpp"

#include <gpscat/Utils.h>
#include <vector>
#include <string>

using gpscat::split;
using strings = std::vector<std::string>;

TEST_CASE("Utils: split", "[utils]") {
    REQUIRE(split("12 ab .]") == strings{"12", "ab", ".]"});
    REQUIRE(split("\te1\t3\tef\n") == strings{"e1", "3", "ef"});
    REQUIRE(split("\t\t3\n \nt\t\n3") == strings{"3", "t", "3"});
    REQUIRE(split(" ef ef ") == strings{"ef", "ef"});
    REQUIRE(split("f123_efeij") == strings{"f123_efeij"});
    REQUIRE(split(" \t\n ") == strings{});
    REQUIRE(split("") == strings{});
    REQUIRE(split("  3  ") == strings{"3"});
    REQUIRE(split("\t.loc\t123 127 42 is_good 0   # fea.c:4:6") ==
                strings{".loc", "123", "127", "42", "is_good", "0", "#", "fea.c:4:6"});
}
