// Copyright 2019 Google LLC
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/logical_test.cc"
#include "hwy/foreach_target.h"  // IWYU pragma: keep
#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

HWY_BEFORE_NAMESPACE();
namespace hwy {
namespace HWY_NAMESPACE {

struct TestNot {
  template <class T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const auto v0 = Zero(d);
    const auto ones = VecFromMask(d, Eq(v0, v0));
    const auto v1 = Set(d, 1);
    const auto vnot1 = Set(d, T(~T(1)));

    HWY_ASSERT_VEC_EQ(d, v0, Not(ones));
    HWY_ASSERT_VEC_EQ(d, ones, Not(v0));
    HWY_ASSERT_VEC_EQ(d, v1, Not(vnot1));
    HWY_ASSERT_VEC_EQ(d, vnot1, Not(v1));
  }
};

HWY_NOINLINE void TestAllNot() {
  ForIntegerTypes(ForPartialVectors<TestNot>());
}

struct TestLogical {
  template <class T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const auto v0 = Zero(d);
    const auto vi = Iota(d, 0);

    auto v = vi;
    v = And(v, vi);
    HWY_ASSERT_VEC_EQ(d, vi, v);
    v = And(v, v0);
    HWY_ASSERT_VEC_EQ(d, v0, v);

    v = Or(v, vi);
    HWY_ASSERT_VEC_EQ(d, vi, v);
    v = Or(v, v0);
    HWY_ASSERT_VEC_EQ(d, vi, v);

    v = Xor(v, vi);
    HWY_ASSERT_VEC_EQ(d, v0, v);
    v = Xor(v, v0);
    HWY_ASSERT_VEC_EQ(d, v0, v);

    HWY_ASSERT_VEC_EQ(d, v0, And(v0, vi));
    HWY_ASSERT_VEC_EQ(d, v0, And(vi, v0));
    HWY_ASSERT_VEC_EQ(d, vi, And(vi, vi));

    HWY_ASSERT_VEC_EQ(d, vi, Or(v0, vi));
    HWY_ASSERT_VEC_EQ(d, vi, Or(vi, v0));
    HWY_ASSERT_VEC_EQ(d, vi, Or(vi, vi));

    HWY_ASSERT_VEC_EQ(d, vi, Xor(v0, vi));
    HWY_ASSERT_VEC_EQ(d, vi, Xor(vi, v0));
    HWY_ASSERT_VEC_EQ(d, v0, Xor(vi, vi));

    HWY_ASSERT_VEC_EQ(d, vi, AndNot(v0, vi));
    HWY_ASSERT_VEC_EQ(d, v0, AndNot(vi, v0));
    HWY_ASSERT_VEC_EQ(d, v0, AndNot(vi, vi));

    HWY_ASSERT_VEC_EQ(d, v0, Or3(v0, v0, v0));
    HWY_ASSERT_VEC_EQ(d, vi, Or3(v0, vi, v0));
    HWY_ASSERT_VEC_EQ(d, vi, Or3(v0, v0, vi));
    HWY_ASSERT_VEC_EQ(d, vi, Or3(v0, vi, vi));
    HWY_ASSERT_VEC_EQ(d, vi, Or3(vi, v0, v0));
    HWY_ASSERT_VEC_EQ(d, vi, Or3(vi, vi, v0));
    HWY_ASSERT_VEC_EQ(d, vi, Or3(vi, v0, vi));
    HWY_ASSERT_VEC_EQ(d, vi, Or3(vi, vi, vi));

    HWY_ASSERT_VEC_EQ(d, v0, Xor3(v0, v0, v0));
    HWY_ASSERT_VEC_EQ(d, vi, Xor3(v0, vi, v0));
    HWY_ASSERT_VEC_EQ(d, vi, Xor3(v0, v0, vi));
    HWY_ASSERT_VEC_EQ(d, v0, Xor3(v0, vi, vi));
    HWY_ASSERT_VEC_EQ(d, vi, Xor3(vi, v0, v0));
    HWY_ASSERT_VEC_EQ(d, v0, Xor3(vi, vi, v0));
    HWY_ASSERT_VEC_EQ(d, v0, Xor3(vi, v0, vi));
    HWY_ASSERT_VEC_EQ(d, vi, Xor3(vi, vi, vi));

    HWY_ASSERT_VEC_EQ(d, v0, OrAnd(v0, v0, v0));
    HWY_ASSERT_VEC_EQ(d, v0, OrAnd(v0, vi, v0));
    HWY_ASSERT_VEC_EQ(d, v0, OrAnd(v0, v0, vi));
    HWY_ASSERT_VEC_EQ(d, vi, OrAnd(v0, vi, vi));
    HWY_ASSERT_VEC_EQ(d, vi, OrAnd(vi, v0, v0));
    HWY_ASSERT_VEC_EQ(d, vi, OrAnd(vi, vi, v0));
    HWY_ASSERT_VEC_EQ(d, vi, OrAnd(vi, v0, vi));
    HWY_ASSERT_VEC_EQ(d, vi, OrAnd(vi, vi, vi));
  }
};

HWY_NOINLINE void TestAllLogical() {
  ForAllTypes(ForPartialVectors<TestLogical>());
}

struct TestCopySign {
  template <class T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const auto v0 = Zero(d);
    const auto vp = Iota(d, 1);
    const auto vn = Iota(d, T(-1E5));  // assumes N < 10^5

    // Zero remains zero regardless of sign
    HWY_ASSERT_VEC_EQ(d, v0, CopySign(v0, v0));
    HWY_ASSERT_VEC_EQ(d, v0, CopySign(v0, vp));
    HWY_ASSERT_VEC_EQ(d, v0, CopySign(v0, vn));
    HWY_ASSERT_VEC_EQ(d, v0, CopySignToAbs(v0, v0));
    HWY_ASSERT_VEC_EQ(d, v0, CopySignToAbs(v0, vp));
    HWY_ASSERT_VEC_EQ(d, v0, CopySignToAbs(v0, vn));

    // Positive input, positive sign => unchanged
    HWY_ASSERT_VEC_EQ(d, vp, CopySign(vp, vp));
    HWY_ASSERT_VEC_EQ(d, vp, CopySignToAbs(vp, vp));

    // Positive input, negative sign => negated
    HWY_ASSERT_VEC_EQ(d, Neg(vp), CopySign(vp, vn));
    HWY_ASSERT_VEC_EQ(d, Neg(vp), CopySignToAbs(vp, vn));

    // Negative input, negative sign => unchanged
    HWY_ASSERT_VEC_EQ(d, vn, CopySign(vn, vn));

    // Negative input, positive sign => negated
    HWY_ASSERT_VEC_EQ(d, Neg(vn), CopySign(vn, vp));
  }
};

HWY_NOINLINE void TestAllCopySign() {
  ForFloatTypes(ForPartialVectors<TestCopySign>());
}

struct TestBroadcastSignBit {
  template <class T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const auto s0 = Zero(d);
    const auto s1 = Set(d, -1);  // all bit set
    const auto vpos = And(Iota(d, 0), Set(d, LimitsMax<T>()));
    const auto vneg = Sub(s1, vpos);

    HWY_ASSERT_VEC_EQ(d, s0, BroadcastSignBit(vpos));
    HWY_ASSERT_VEC_EQ(d, s0, BroadcastSignBit(Set(d, LimitsMax<T>())));

    HWY_ASSERT_VEC_EQ(d, s1, BroadcastSignBit(vneg));
    HWY_ASSERT_VEC_EQ(d, s1, BroadcastSignBit(Set(d, LimitsMin<T>())));
    HWY_ASSERT_VEC_EQ(d, s1, BroadcastSignBit(Set(d, LimitsMin<T>() / 2)));
  }
};

HWY_NOINLINE void TestAllBroadcastSignBit() {
  ForSignedTypes(ForPartialVectors<TestBroadcastSignBit>());
}

struct TestTestBit {
  template <class T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const size_t kNumBits = sizeof(T) * 8;
    for (size_t i = 0; i < kNumBits; ++i) {
      const auto bit1 = Set(d, T(1ull << i));
      const auto bit2 = Set(d, T(1ull << ((i + 1) % kNumBits)));
      const auto bit3 = Set(d, T(1ull << ((i + 2) % kNumBits)));
      const auto bits12 = Or(bit1, bit2);
      const auto bits23 = Or(bit2, bit3);
      HWY_ASSERT(AllTrue(d, TestBit(bit1, bit1)));
      HWY_ASSERT(AllTrue(d, TestBit(bits12, bit1)));
      HWY_ASSERT(AllTrue(d, TestBit(bits12, bit2)));

      HWY_ASSERT(AllFalse(d, TestBit(bits12, bit3)));
      HWY_ASSERT(AllFalse(d, TestBit(bits23, bit1)));
      HWY_ASSERT(AllFalse(d, TestBit(bit1, bit2)));
      HWY_ASSERT(AllFalse(d, TestBit(bit2, bit1)));
      HWY_ASSERT(AllFalse(d, TestBit(bit1, bit3)));
      HWY_ASSERT(AllFalse(d, TestBit(bit3, bit1)));
      HWY_ASSERT(AllFalse(d, TestBit(bit2, bit3)));
      HWY_ASSERT(AllFalse(d, TestBit(bit3, bit2)));
    }
  }
};

HWY_NOINLINE void TestAllTestBit() {
  ForIntegerTypes(ForPartialVectors<TestTestBit>());
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
}  // namespace HWY_NAMESPACE
}  // namespace hwy
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace hwy {
HWY_BEFORE_TEST(HwyLogicalTest);
HWY_EXPORT_AND_TEST_P(HwyLogicalTest, TestAllNot);
HWY_EXPORT_AND_TEST_P(HwyLogicalTest, TestAllLogical);
HWY_EXPORT_AND_TEST_P(HwyLogicalTest, TestAllCopySign);
HWY_EXPORT_AND_TEST_P(HwyLogicalTest, TestAllBroadcastSignBit);
HWY_EXPORT_AND_TEST_P(HwyLogicalTest, TestAllTestBit);
}  // namespace hwy

#endif
