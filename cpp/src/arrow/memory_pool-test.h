// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "gtest/gtest.h"

#include <limits>

#include "arrow/memory_pool.h"
#include "arrow/test-util.h"

namespace arrow {

namespace test {

class TestMemoryPoolBase : public ::testing::Test {
 public:
  virtual ::arrow::MemoryPool* memory_pool() = 0;

  void TestMemoryTracking() {
    auto pool = memory_pool();

    uint8_t* data;
    ASSERT_OK(pool->Allocate(100, &data));
    EXPECT_EQ(static_cast<uint64_t>(0), reinterpret_cast<uint64_t>(data) % 64);
    ASSERT_EQ(100, pool->bytes_allocated());

    pool->Free(data, 100);
    ASSERT_EQ(0, pool->bytes_allocated());
  }

  void TestOOM() {
    auto pool = memory_pool();

    uint8_t* data;
    int64_t to_alloc = std::numeric_limits<int64_t>::max();
    ASSERT_RAISES(OutOfMemory, pool->Allocate(to_alloc, &data));
  }

  void TestReallocate() {
    auto pool = memory_pool();

    uint8_t* data;
    ASSERT_OK(pool->Allocate(10, &data));
    ASSERT_EQ(10, pool->bytes_allocated());
    data[0] = 35;
    data[9] = 12;

    // Expand
    ASSERT_OK(pool->Reallocate(10, 20, &data));
    ASSERT_EQ(data[9], 12);
    ASSERT_EQ(20, pool->bytes_allocated());

    // Shrink
    ASSERT_OK(pool->Reallocate(20, 5, &data));
    ASSERT_EQ(data[0], 35);
    ASSERT_EQ(5, pool->bytes_allocated());

    // Free
    pool->Free(data, 5);
    ASSERT_EQ(0, pool->bytes_allocated());
  }
};

}  // namespace test
}  // namespace arrow
