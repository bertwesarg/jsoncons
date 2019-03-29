// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <string>
#include <iomanip>

using namespace jsoncons;

void serialize_to_cbor_buffer()
{
    std::vector<uint8_t> buffer;
    cbor::cbor_buffer_encoder writer(buffer);

    writer.begin_array(); // Indefinite length array
    writer.string_value("cat");
    writer.byte_string_value(byte_string({'p','u','r','r'}));
    writer.byte_string_value(byte_string({'h','i','s','s'}),
                             semantic_tag::base64); // suggested conversion to base64
    writer.int64_value(1431027667, semantic_tag::timestamp);
    writer.end_array();
    writer.flush();

    for (auto c : buffer)
    {
        std::cout << std::hex << std::setprecision(2) << std::setw(2) 
                  << std::noshowbase << std::setfill('0') << static_cast<int>(c);
    }
    std::cout << "\n\n";

/* 
    9f -- Start indefinte length array
      63 -- String value of length 3
        636174 -- "cat"
      44 -- Byte string value of length 4
        70757272 -- 'p''u''r''r'
      d6 - Expected conversion to base64
      44
        68697373 -- 'h''i''s''s'
      c1 -- Tag value 1 (seconds relative to 1970-01-01T00:00Z in UTC time)
        1a -- 32 bit unsigned integer
          554bbfd3 -- 1431027667
      ff -- "break" 
*/ 
}

void serialize_to_cbor_stream()
{
    std::ostringstream os;
    cbor::cbor_encoder writer(os);

    writer.begin_array(3); // array of length 3
    writer.big_integer_value("-18446744073709551617");
    writer.big_decimal_value("184467440737095516.16");
    writer.timestamp_value(1431027667);
    writer.end_array();
    writer.flush();

    for (auto c : os.str())
    {
        std::cout << std::hex << std::setprecision(2) << std::setw(2) 
                  << std::noshowbase << std::setfill('0') << (int)unsigned char(c);
    }
    std::cout << "\n\n";

/*
    83 -- array of length 3
      c3 -- Tag 3 (negative bignum)
      49 -- Byte string value of length 9
        010000000000000000 -- Bytes content
      c4 -- Tag 4 (decimal fraction)
        82 -- Array of length 2
          21 -- -2 (exponent)
          c2 Tag 2 (positive bignum)
          49 -- Byte string value of length 9
            010000000000000000
      c1 -- Tag 1 (seconds relative to 1970-01-01T00:00Z in UTC time)
        1a -- 32 bit unsigned integer
          554bbfd3 -- 1431027667
*/
}

void cbor_reputon_example()
{
    ojson j1 = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    // Encoding an unpacked (json) value to a packed CBOR value
    std::vector<uint8_t> data;
    cbor::encode_cbor(j1, data);

    // Decoding a packed CBOR value to an unpacked (json) value
    ojson j2 = cbor::decode_cbor<ojson>(data);
    std::cout << "(1)\n" << pretty_print(j2) << "\n\n";

    // Accessing the data items 

    const ojson& reputons = j2["reputons"];

    std::cout << "(2)\n";
    for (auto element : reputons.array_range())
    {
        std::cout << element.at("rated").as_string() << ", ";
        std::cout << element.at("rating").as_double() << "\n";
    }
    std::cout << std::endl;

    // Querying a packed CBOR value for a nested data item with jsonpointer
    std::error_code ec;
    auto const& rated = jsonpointer::get(j2, "/reputons/0/rated", ec);
    if (!ec)
    {
        std::cout << "(3) " << rated.as_string() << "\n";
    }

    std::cout << std::endl;
}

void decode_cbor_byte_string()
{
    // byte string of length 5
    std::vector<uint8_t> buf = {0x45,'H','e','l','l','o'};
    json j = cbor::decode_cbor<json>(buf);

    auto bs = j.as<byte_string>();

    // byte_string to ostream displays as hex
    std::cout << "(1) "<< bs << "\n\n";

    // byte string value to JSON text becomes base64url
    std::cout << "(2) " << j << std::endl;
}

void decode_byte_string_with_encoding_hint()
{
    // semantic tag indicating expected conversion to base64
    // followed by byte string of length 5
    std::vector<uint8_t> buf = {0xd6,0x45,'H','e','l','l','o'};
    json j = cbor::decode_cbor<json>(buf);

    auto bs = j.as<byte_string>();

    // byte_string to ostream displays as hex
    std::cout << "(1) "<< bs << "\n\n";

    // byte string value to JSON text becomes base64
    std::cout << "(2) " << j << std::endl;
}

void encode_cbor_byte_string()
{
    // construct byte string value
    json j(byte_string("Hello"));

    std::vector<uint8_t> buf;
    cbor::encode_cbor(j, buf);

    std::cout << std::hex << std::showbase << "(1) ";
    for (auto c : buf)
    {
        std::cout << (int)c;
    }
    std::cout << std::dec << "\n\n";

    json j2 = cbor::decode_cbor<json>(buf);
    std::cout << "(2) " << j2 << std::endl;
}

void encode_byte_string_with_encoding_hint()
{
    // construct byte string value
     json j1(byte_string("Hello"), semantic_tag::base64);

    std::vector<uint8_t> buf;
    cbor::encode_cbor(j1, buf);

    std::cout << std::hex << std::showbase << "(1) ";
    for (auto c : buf)
    {
        std::cout << (int)c;
    }
    std::cout << std::dec << "\n\n";

    json j2 = cbor::decode_cbor<json>(buf);
    std::cout << "(2) " << j2 << std::endl;
}

void query_cbor()
{
    // Construct a json array of numbers
    json j = json::array();

    j.emplace_back(5.0);

    j.emplace_back(0.000071);

    j.emplace_back("-18446744073709551617",semantic_tag::big_integer);

    j.emplace_back("1.23456789012345678901234567890", semantic_tag::big_decimal);

    j.emplace_back(json::array({-1,3}), semantic_tag::big_float);

    // Serialize to JSON
    std::cout << "(1)\n";
    std::cout << pretty_print(j);
    std::cout << "\n\n";

    // as<std::string>() and as<double>()
    std::cout << "(2)\n";
    std::cout << std::dec << std::setprecision(15);
    for (const auto& item : j.array_range())
    {
        std::cout << item.as<std::string>() << ", " << item.as<double>() << "\n";
    }
    std::cout << "\n";

    // Encode to CBOR
    std::vector<uint8_t> v;
    cbor::encode_cbor(j,v);

    std::cout << "(3)\n";
    for (auto c : v)
    {
        std::cout << std::hex << std::setprecision(2) << std::setw(2)
                  << std::setfill('0') << static_cast<int>(c);
    }
    std::cout << "\n\n";
/*
    85 -- Array of length 5     
      fa -- float 
        40a00000 -- 5.0
      fb -- double 
        3f129cbab649d389 -- 0.000071
      c3 -- Tag 3 (negative bignum)
        49 -- Byte string value of length 9
          010000000000000000
      c4 -- Tag 4 (decimal fraction)
        82 -- Array of length 2
          38 -- Negative integer of length 1
            1c -- -29
          c2 -- Tag 2 (positive bignum)
            4d -- Byte string value of length 13
              018ee90ff6c373e0ee4e3f0ad2
      c5 -- Tag 5 (bigfloat)
        82 -- Array of length 2
          20 -- -1
          03 -- 3   
*/

    // Decode back to json
    json other = cbor::decode_cbor<json>(v);
    assert(other == j);

    // Query with JSONPath
    std::cout << "(4)\n";
    json result = jsonpath::json_query(other,"$.[?(@ < 1.5)]");
    std::cout << pretty_print(result) << "\n\n";
}

void query_cbor2()
{
    std::vector<uint8_t> v = {0x85,0xfa,0x40,0x0,0x0,0x0,0xfb,0x3f,0x12,0x9c,0xba,0xb6,0x49,0xd3,0x89,0xc3,0x49,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc4,0x82,0x38,0x1c,0xc2,0x4d,0x1,0x8e,0xe9,0xf,0xf6,0xc3,0x73,0xe0,0xee,0x4e,0x3f,0xa,0xd2,0xc5,0x82,0x20,0x3};
/*
    85 -- Array of length 5     
      fa -- float 
        40a00000 -- 5.0
      fb -- double 
        3f129cbab649d389 -- 0.000071
      c3 -- Tag 3 (negative bignum)
        49 -- Byte string value of length 9
          010000000000000000
      c4 -- Tag 4 (decimal fraction)
        82 -- Array of length 2
          38 -- Negative integer of length 1
            1c -- -29
          c2 -- Tag 2 (positive bignum)
            4d -- Byte string value of length 13
              018ee90ff6c373e0ee4e3f0ad2
      c5 -- Tag 5 (bigfloat)
        82 -- Array of length 2
          20 -- -1
          03 -- 3   
*/

    // Decode to a json value (despite its name, it is not JSON specific.)
    json j = cbor::decode_cbor<json>(v);

    // Serialize to JSON
    std::cout << "(1)\n";
    std::cout << pretty_print(j);
    std::cout << "\n\n";

    // as<std::string>() and as<double>()
    std::cout << "(2)\n";
    std::cout << std::dec << std::setprecision(15);
    for (const auto& item : j.array_range())
    {
        std::cout << item.as<std::string>() << ", " << item.as<double>() << "\n";
    }
    std::cout << "\n";

    // Query with JSONPath
    std::cout << "(3)\n";
    json result = jsonpath::json_query(j,"$.[?(@ < 1.5)]");
    std::cout << pretty_print(result) << "\n\n";

    // Encode result as CBOR
    std::vector<uint8_t> val;
    cbor::encode_cbor(result,val);

    std::cout << "(4)\n";
    for (auto c : val)
    {
        std::cout << std::hex << std::setprecision(2) << std::setw(2)
                  << std::setfill('0') << static_cast<int>(c);
    }
    std::cout << "\n\n";

/*
    83 -- Array of length 3
      fb -- double
        3f129cbab649d389 -- 0.000071
    c3 -- Tag 3 (negative bignum)
      49 -- Byte string value of length 9
        010000000000000000
    c4 -- Tag 4 (decimal fraction)
      82 -- Array of length 2
        38 -- Negative integer of length 1
          1c -- -29
        c2 -- Tag 2 (positive bignum)
          4d -- Byte string value of length 13
            018ee90ff6c373e0ee4e3f0ad2
*/
}

void cbor_examples()
{
/*    std::cout << "\ncbor examples\n\n";
    decode_byte_string_with_encoding_hint();
    encode_byte_string_with_encoding_hint();
    decode_cbor_byte_string();
    encode_cbor_byte_string();
    serialize_to_cbor_buffer();
    serialize_to_cbor_stream();
    cbor_reputon_example();
    query_cbor();
*/
    query_cbor2();
    std::cout << std::endl;
}

