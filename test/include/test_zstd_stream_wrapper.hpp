/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_TEST_ZSTD_STREAM_WRAPPER_HPP
#define BXZSTR_TEST_ZSTD_STREAM_WRAPPER_HPP

#include <string>
#include <fstream>

#include "gtest/gtest.h"
#include "zstd.h"

#include "bxzstr.hpp"

// Integration tests for zstd_stream_wrapper
//
// Decompression
class ZstdDecompressionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	// Fake zstd data with 10 1s on their own lines
	const unsigned char test[] = { 0x28, 0xb5, 0x2f, 0xfd, 0x00, 0x58, 0x45,
	                               0x00, 0x00, 0x10, 0x31, 0x0a, 0x01, 0x00,
				       0x79, 0x0e, 0x0b, 0x28, 0xb5, 0x2f, 0xfd,
				       0x20, 0x00, 0x01, 0x00, 0x00 };
	this->testInFile = "ZstdDecompressionTest_fake_data.txt.zst";
	std::ofstream of(testInFile);
	for (uint32_t i = 0; i < 26; ++i) {
	    of << test[i];
	}
	of.close();

	this->nInVals = 10;
	this->expected = std::vector<char>(nInVals, '1');
    }
    void TearDown() override {
    }
    // Test values
    std::string testInFile;
    uint32_t nInVals;
    std::vector<char> expected;
};

// Compression
class ZstdCompressionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	// Raw data from running bxz::ofstream with bxz::zstd for this test set
	const unsigned char test[] = { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00,
	                               0x00, 0x00, 0x03, 0x33, 0xe4, 0x32, 0x44,
				       0x87, 0x00, 0xae, 0x30, 0x5a, 0x73, 0x13,
				       0x00, 0x00, 0x00, 0x1f, 0x8b, 0x08, 0x00,
				       0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03,
				       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				       0x00, 0x00 };

	this->testOutFile = "ZstdCompressionTest_fake_data.txt.zst";
	this->nOutVals = 10;
	this->expected = std::vector<char>(44);

	bxz::ofstream out(testOutFile);
	for (uint32_t i = 0; i < nOutVals; ++i) {
	    out << 1;
	    if (i < nOutVals - 1)
		out << '\n';
	}
	out.flush();

	for (uint32_t i = 0; i < 44; ++i) {
	    expected[i] = test[i];
	}
    }
    void TearDown() override {
    }
    // Test values
    std::string testOutFile;
    uint32_t nOutVals;
    std::vector<char> expected;
};

#endif
