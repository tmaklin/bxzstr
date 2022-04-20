/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#include "zstd_stream_wrapper_unittest.hpp"

#include "zstd.h"

#include "bxzstr.hpp"
#include <iostream>

TEST_F(ZstdExceptionTest, MsgConstructorWorks) {
    bxz::zstdException e(msgConstructorValue);
    const std::string &got = e.what();
    EXPECT_EQ(msgConstructorExpected, got);
}

TEST_F(ZstdExceptionTest, ErrcodeConstructorWorks) {
    bxz::zstdException e(errcodeConstructorValue);
    const std::string &got = e.what();
    EXPECT_EQ(errcodeConstructorExpected, got);
}

TEST_F(ZstdStreamWrapperTest, ConstructorDoesNotThrowOnInput) {
    EXPECT_NO_THROW(bxz::detail::zstd_stream_wrapper wrapper(testTrue));
}

TEST_F(ZstdStreamWrapperTest, ConstructorDoesNotThrowOnOutput) {
    EXPECT_NO_THROW(bxz::detail::zstd_stream_wrapper wrapper(testFalse));
}

TEST_F(DecompressTest, DecompressDoesNotThrowOnValidFrame) {
    EXPECT_NO_THROW(wrapper->decompress());
}

TEST_F(DecompressTest, DecompressThrowsOnInvalidFrame) {
    wrapper->set_avail_in(5);
    EXPECT_THROW(wrapper->decompress(), bxz::zstdException);
}

TEST_F(DecompressTest, DecompressUpdatesStreamState) {
    wrapper->decompress();
    EXPECT_EQ(wrapper->next_in(), &testIn[4]);
    EXPECT_EQ(wrapper->avail_in(), 0);
    EXPECT_EQ(wrapper->next_out(), &testOut[0]);
    EXPECT_EQ(wrapper->avail_out(), 4);
}

TEST_F(CompressTest, CompressEndsStream) {
    wrapper->set_avail_out(0);
    wrapper->set_next_out(&testOut[4]);
    EXPECT_NO_THROW(wrapper->compress(true));
}

TEST_F(CompressTest, CompressDoesNotThrowOnValidInput) {
    EXPECT_NO_THROW(wrapper->compress(false));
}

TEST_F(CompressTest, CompressUpdatesStreamState) {
    wrapper->compress(false);
    EXPECT_EQ(wrapper->next_in(), &testIn[4]);
    EXPECT_EQ(wrapper->avail_in(), 0);
    EXPECT_EQ(wrapper->next_out(), &testOut[0]);
    EXPECT_EQ(wrapper->avail_out(), 4);
}
