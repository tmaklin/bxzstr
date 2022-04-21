/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_TEST_Z_STREAM_WRAPPER_HPP
#define BXZSTR_TEST_Z_STREAM_WRAPPER_HPP

#include <string>
#include <fstream>

#include "gtest/gtest.h"
#include "zstd.h"

#include "bxzstr.hpp"

// Integration tests for z_stream_wrapper
//
// Decompression
class ZDecompressionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	// Fake gzip data with 10 1s on their own lines
	const unsigned char test[] = {0x1f, 0x8b, 0x08, 0x08, 0xf1, 0x0a, 0x61, 0x62, 0x00, 0x03, 0x74, 0x65, 0x73, 0x74, 0x7a, 0x2e,
	                              0x74, 0x78, 0x74, 0x00, 0x33, 0xe4, 0x32, 0xc4, 0x80, 0x00, 0x4c, 0xd2, 0xca, 0x03, 0x14, 0x00,
				      0x00, 0x00 };
	this->testInFile = "ZDecompressionTest_fake_data.txt.gz";
	std::ofstream of(testInFile);
	for (uint32_t i = 0; i < 34; ++i) {
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
class ZCompressionTest : public ::testing::Test {
  protected:
    void SetUp() override {
	// Raw data from running bxz::ofstream with bxz::z for this test set
	const unsigned char test[] = { 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0xe4, 0x32, 0x44, 0x87, 0x00,
	                               0xae, 0x30, 0x5a, 0x73, 0x13, 0x00, 0x00, 0x00, 0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
				       0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	this->testOutFile = "ZCompressionTest_fake_data.txt.gz";
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
