#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "huffman.h"
#include <string>
#include <map>

using namespace Huffman;


TEST_SUITE("bit_oseq") {
    TEST_CASE("void output"){
        std::stringstream ss;
        bit_oseq bos(ss);
        const seq_size_t* size;
        const byte_t* data;

        bos.destroy();
        auto str = ss.str();
        REQUIRE_EQ(str.size(), sizeof(seq_size_t));
        size = (const seq_size_t*)str.c_str();
        data = (const byte_t*) (size + 1);
        REQUIRE_EQ(*size, 0);    
    }

    TEST_CASE("zero byte output"){
        std::stringstream ss;
        bit_oseq bos(ss);
        const seq_size_t* size;
        const byte_t* data;

        for(int i = 0; i < 8; i++)
            bos.write(false);
        bos.destroy();

        auto str = ss.str();
        REQUIRE_EQ(str.size(), sizeof(seq_size_t) + 1);
        size = (const seq_size_t*)str.c_str();
        data = (const byte_t*) (size + 1);
        REQUIRE_EQ(*size, 8);     
        CHECK_EQ(data[0], 0);
    }

    TEST_CASE("test each bit"){
        std::stringstream ss;
        bit_oseq bos(ss);
        const seq_size_t* size;
        const byte_t* data;

        for(int k = 0; k < 8; k++)
            for(int i = 0; i < 8; i++)
                bos.write(i == k);
        bos.destroy();

        auto str = ss.str();
        REQUIRE_EQ(str.size(), sizeof(seq_size_t) + 8);
        size = (const seq_size_t*)str.c_str();
        data = (const byte_t*) (size + 1);
        REQUIRE_EQ(*size, 8 * 8);     
        for(int k = 0; k < 8; k++)
            CHECK_EQ(data[k], 1 << k);
    }

    TEST_CASE("not empty stream"){
        const char* initial_text = "abcde";
        std::stringstream ss(initial_text);
        ss.seekp(0, ss.end);
        bit_oseq bos(ss);
        const seq_size_t* size;
        const byte_t* data;

        for(int k = 0; k < 8; k++)
            for(int i = 0; i < 8; i++)
                bos.write(i == k);
        bos.destroy();

        auto str = ss.str();
        REQUIRE_EQ(str.size(), 5 + sizeof(seq_size_t) + 8);
        size = (const seq_size_t*)(str.c_str() + 5);
        data = (const byte_t*)(size + 1);
        REQUIRE_EQ(str.substr(0, 5), initial_text);
        REQUIRE_EQ(*size, 8 * 8);     
        for(int k = 0; k < 8; k++)
            CHECK_EQ(data[k], 1 << k);
    }
}


TEST_SUITE("bit_iseq") {
    TEST_CASE("void input"){
        std::stringstream ss;
        bit_oseq bos(ss);
        bos.destroy();

        bit_iseq bis(ss);
        CHECK_EQ(bis.size(), 0);
        CHECK(bis.end_of_seq());          
    }

    TEST_CASE("one bit input"){
        std::stringstream ss;
        bit_oseq bos(ss);
        bos.write(false);
        bos.destroy();

        bit_iseq bis(ss);
        CHECK_EQ(bis.size(), 1);
        CHECK_EQ(bis.read(), false);
        CHECK(bis.end_of_seq());
    }

    TEST_CASE("test each bit in byte"){
        std::stringstream ss;
        bit_oseq bos(ss);
        for(int i = 0; i < 8; i++)
            for(int j = 0; j < 8; j++)
                bos.write(i == j);
        bos.destroy();

        bit_iseq bis(ss);
        CHECK_EQ(bis.size(), 64);
        for(int i = 0; i < 8; i++)
            for(int j = 0; j < 8; j++)
                CHECK_EQ(bis.read(), i == j);
        CHECK(bis.end_of_seq());
    }

    TEST_CASE("not empty stream"){
        const char* initial_text = "abcde";
        std::stringstream ss(initial_text);
        ss.seekp(0, ss.end);
        ss.seekg(0, ss.end);
        bit_oseq bos(ss);
        for(int i = 0; i < 8; i++)
            for(int j = 0; j < 8; j++)
                bos.write(i == j);
        bos.destroy();

        bit_iseq bis(ss);
        CHECK_EQ(bis.size(), 64);
        for(int i = 0; i < 8; i++)
            for(int j = 0; j < 8; j++)
                CHECK_EQ(bis.read(), i == j);
        CHECK(bis.end_of_seq());
    }
}




TEST_CASE_FIXTURE(HuffmanTree, "construct and encode"){
    std::map<char, double> p{{'a', 1}, {'b', 2}, {'c', 4}, {'d', 6}, {'e', 8}};
    construct(p);
    REQUIRE_EQ(nodes.size(), 9);

    std::stringstream ss;
    std::istringstream iss("abcdeedcba");

    encode(iss, ss);

    bit_iseq bis(ss);
    bool expected[] = {
        1, 1, 0, 0,    // a
        1, 1, 0, 1,
        1, 1, 1,
        1, 0,
        0, 
        0,
        1, 0,
        1, 1, 1,
        1, 1, 0, 1,
        1, 1, 0, 0, 
    };

    REQUIRE_EQ(bis.size(), sizeof(expected));

    for(int i = 0; i < bis.size(); i++){
        CHECK_EQ(bis.read(), expected[i]);
    }
}


std::string encode_and_decode(HuffmanTree& tree, const char* text){
    std::stringstream initial_text(text);
    std::stringstream encoded_text;
    std::stringstream decoded_text;
    tree.encode(initial_text, encoded_text);
    tree.decode(encoded_text, decoded_text);
    return decoded_text.str();
}


TEST_CASE("huffman tree, decode"){
    std::map<char, double> p{{'a', 1}, {'b', 2}, {'c', 4}, {'d', 6}, {'e', 8}};
    HuffmanTree tree(p);

    #define CHECK_ENCODE_DECODE(text) \
        CHECK_EQ(text, encode_and_decode(tree, text))

    CHECK_ENCODE_DECODE("a");
    CHECK_ENCODE_DECODE("aaaaa");
    CHECK_ENCODE_DECODE("aaaaaaaaaa");
    CHECK_ENCODE_DECODE("bbbbbbbbbbb");
    CHECK_ENCODE_DECODE("abcde");
    CHECK_ENCODE_DECODE("abcdeabcdeabcde");
    CHECK_ENCODE_DECODE("eabdceabacdebdcadbceabdcbebdabce"); 

    #undef CHECK_ENCODE_DECODE
}

TEST_CASE("huffman tree: save and load"){
    std::map<char, double> p{{'a', 1}, {'b', 2}, {'c', 4}, {'d', 6}, {'e', 8}};
    HuffmanTree initial_tree(p);
    std::stringstream ss;
    initial_tree.save(ss);
    HuffmanTree result_tree;
    result_tree.load(ss);
    CHECK_EQ(initial_tree, result_tree);
}


TEST_CASE("huffman counts"){
    std::stringstream ss("abbcccddddeeeee");
    std::map<char, double> expected{{'a', 1}, {'b', 2}, {'c', 3}, {'d', 4}, {'e', 5}};
    auto pos = ss.tellg();
    auto res = counts(ss);
    REQUIRE_EQ(pos, ss.tellg());
    CHECK_EQ(res.size(), expected.size());
    for(auto iter = expected.cbegin(); iter != expected.cend(); ++iter){
        REQUIRE_EQ(res.count(iter->first), 1);
        CHECK_EQ(res[iter->first], iter->second);
    }
}


std::string encode_and_decode(const char* text){
    std::stringstream initial_text(text);
    std::stringstream encoded_text;
    std::stringstream decoded_text;
    encode(initial_text, encoded_text);
    decode(encoded_text, decoded_text);
    return decoded_text.str();
}


TEST_CASE("final test: encode and decode text"){
    #define CHECK_ENCODE_DECODE(text) \
        CHECK_EQ(text, encode_and_decode(text))

    CHECK_ENCODE_DECODE("");
    CHECK_ENCODE_DECODE("a");
    CHECK_ENCODE_DECODE("ab");
    CHECK_ENCODE_DECODE("aaaaa");
    CHECK_ENCODE_DECODE("abcde");
    CHECK_ENCODE_DECODE("text text text text text");

    #undef CHECK_ENCODE_DECODE
}



