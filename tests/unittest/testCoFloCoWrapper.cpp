#include "catch.hpp"

#include <gpscat/CoFloCoWrapper.h>
#include <string>

using gpscat::CoFloCoWrapper;

TEST_CASE("CoFloCoWrapper: removeNat", "[coflocoWrapper]") {
    CoFloCoWrapper wrapper;
    REQUIRE(wrapper.removeNat("nat(x)") == "(x)");
    REQUIRE(wrapper.removeNat("nat(a_b0)") == "(a_b0)");
    REQUIRE(wrapper.removeNat("nat(v_0-1)") == "(v_0-1)");
    REQUIRE(wrapper.removeNat("nat(nat(x))") == "((x))");

    REQUIRE(wrapper.removeNat("nas(x)") == "nas(x)");
    REQUIRE(wrapper.removeNat("mat(x)") == "mat(x)");
    REQUIRE(wrapper.removeNat("max([x,3])") == "max([x,3])");
    REQUIRE(wrapper.removeNat("x") == "x");
    REQUIRE(wrapper.removeNat("1") == "1");
    REQUIRE(wrapper.removeNat("") == "");

    REQUIRE(wrapper.removeNat("max([nat(x+1)-1,3])-1") == "max([(x+1)-1,3])-1");
    REQUIRE(wrapper.removeNat("max([nat(max([nat(x),nat(2*x-2)])),1466])") == "max([(max([(x),(2*x-2)])),1466])");
}

TEST_CASE("CoFloCoWrapper: replaceNatWithMax", "[coflocoWrapper]") {
    CoFloCoWrapper wrapper;
    REQUIRE(wrapper.replaceNatWithMax("nat(x)") == "max([x,0])");
    REQUIRE(wrapper.replaceNatWithMax("nat(a_b0)") == "max([a_b0,0])");
    REQUIRE(wrapper.replaceNatWithMax("nat(v_0-1)") == "max([v_0-1,0])");
    REQUIRE(wrapper.replaceNatWithMax("nat(nat(x))") == "max([max([x,0]),0])");

    REQUIRE(wrapper.replaceNatWithMax("nas(x)") == "nas(x)");
    REQUIRE(wrapper.replaceNatWithMax("mat(x)") == "mat(x)");
    REQUIRE(wrapper.replaceNatWithMax("max([x,3])") == "max([x,3])");
    REQUIRE(wrapper.replaceNatWithMax("x") == "x");
    REQUIRE(wrapper.replaceNatWithMax("1") == "1");
    REQUIRE(wrapper.replaceNatWithMax("") == "");

    REQUIRE(wrapper.replaceNatWithMax("max([nat(x+1)-1,3])-1") == "max([max([x+1,0])-1,3])-1");
    REQUIRE(wrapper.replaceNatWithMax("x+max([nat(max([nat(x),nat(2*x-2)])),1466])-1") == "x+max([max([max([max([x,0]),max([2*x-2,0])]),0]),1466])-1");
}
