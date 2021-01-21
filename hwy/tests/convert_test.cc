// Copyright 2019 Google LLC
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

#include <stddef.h>
#include <stdint.h>

#include <cmath>

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/convert_test.cc"
#include "hwy/foreach_target.h"

#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

HWY_BEFORE_NAMESPACE();
namespace hwy {
namespace HWY_NAMESPACE {

// Cast and ensure bytes are the same. Called directly from TestAllBitCast or
// via TestBitCastFrom.
template <typename ToT>
struct TestBitCast {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const Repartition<ToT, D> dto;
    HWY_ASSERT_EQ(Lanes(d) * sizeof(T), Lanes(dto) * sizeof(ToT));
    const auto vf = Iota(d, 1);
    const auto vt = BitCast(dto, vf);
    // Must return the same bits
    auto from_lanes = AllocateAligned<T>(Lanes(d));
    auto to_lanes = AllocateAligned<ToT>(Lanes(dto));
    Store(vf, d, from_lanes.get());
    Store(vt, dto, to_lanes.get());
    HWY_ASSERT(
        BytesEqual(from_lanes.get(), to_lanes.get(), Lanes(d) * sizeof(T)));
  }
};

// From D to all types.
struct TestBitCastFrom {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T t, D d) {
    TestBitCast<uint8_t>()(t, d);
    TestBitCast<uint16_t>()(t, d);
    TestBitCast<uint32_t>()(t, d);
#if HWY_CAP_INTEGER64
    TestBitCast<uint64_t>()(t, d);
#endif
    TestBitCast<int8_t>()(t, d);
    TestBitCast<int16_t>()(t, d);
    TestBitCast<int32_t>()(t, d);
#if HWY_CAP_INTEGER64
    TestBitCast<int64_t>()(t, d);
#endif
    TestBitCast<float>()(t, d);
#if HWY_CAP_FLOAT64
    TestBitCast<double>()(t, d);
#endif
  }
};

HWY_NOINLINE void TestAllBitCast() {
  // For HWY_SCALAR and partial vectors, we can only cast to same-sized types:
  // the former can't partition its single lane, and the latter can be smaller
  // than a destination type.
  const ForPartialVectors<TestBitCast<uint8_t>> to_u8;
  to_u8(uint8_t());
  to_u8(int8_t());

  const ForPartialVectors<TestBitCast<int8_t>> to_i8;
  to_i8(uint8_t());
  to_i8(int8_t());

  const ForPartialVectors<TestBitCast<uint16_t>> to_u16;
  to_u16(uint16_t());
  to_u16(int16_t());

  const ForPartialVectors<TestBitCast<int16_t>> to_i16;
  to_i16(uint16_t());
  to_i16(int16_t());

  const ForPartialVectors<TestBitCast<uint32_t>> to_u32;
  to_u32(uint32_t());
  to_u32(int32_t());
  to_u32(float());

  const ForPartialVectors<TestBitCast<int32_t>> to_i32;
  to_i32(uint32_t());
  to_i32(int32_t());
  to_i32(float());

#if HWY_CAP_INTEGER64
  const ForPartialVectors<TestBitCast<uint64_t>> to_u64;
  to_u64(uint64_t());
  to_u64(int64_t());
#if HWY_CAP_FLOAT64
  to_u64(double());
#endif

  const ForPartialVectors<TestBitCast<int64_t>> to_i64;
  to_i64(uint64_t());
  to_i64(int64_t());
#if HWY_CAP_FLOAT64
  to_i64(double());
#endif
#endif  // HWY_CAP_INTEGER64

  const ForPartialVectors<TestBitCast<float>> to_float;
  to_float(uint32_t());
  to_float(int32_t());
  to_float(float());

#if HWY_CAP_FLOAT64
  const ForPartialVectors<TestBitCast<double>> to_double;
  to_double(double());
#if HWY_CAP_INTEGER64
  to_double(uint64_t());
  to_double(int64_t());
#endif  // HWY_CAP_INTEGER64
#endif  // HWY_CAP_FLOAT64

  // For non-scalar vectors, we can cast all types to all.
  ForAllTypes(ForGE128Vectors<TestBitCastFrom>());
}

template <typename ToT>
struct TestPromoteTo {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D from_d) {
    static_assert(sizeof(T) < sizeof(ToT), "Input type must be narrower");
    const Rebind<ToT, D> to_d;

    const size_t N = Lanes(from_d);
    auto from = AllocateAligned<T>(N);
    auto expected = AllocateAligned<ToT>(N);

    RandomState rng;
    for (size_t rep = 0; rep < 200; ++rep) {
      for (size_t i = 0; i < N; ++i) {
        const uint64_t bits = rng();
        memcpy(&from[i], &bits, sizeof(T));
        expected[i] = from[i];
      }

      HWY_ASSERT_VEC_EQ(to_d, expected.get(),
                        PromoteTo(to_d, Load(from_d, from.get())));
    }
  }
};

HWY_NOINLINE void TestAllPromoteTo() {
  const ForPartialVectors<TestPromoteTo<uint16_t>, 2> to_u16div2;
  to_u16div2(uint8_t());

  const ForPartialVectors<TestPromoteTo<uint32_t>, 4> to_u32div4;
  to_u32div4(uint8_t());

  const ForPartialVectors<TestPromoteTo<uint32_t>, 2> to_u32div2;
  to_u32div2(uint16_t());

  const ForPartialVectors<TestPromoteTo<int16_t>, 2> to_i16div2;
  to_i16div2(uint8_t());
  to_i16div2(int8_t());

  const ForPartialVectors<TestPromoteTo<int32_t>, 2> to_i32div2;
  to_i32div2(uint16_t());
  to_i32div2(int16_t());

  const ForPartialVectors<TestPromoteTo<int32_t>, 4> to_i32div4;
  to_i32div4(uint8_t());
  to_i32div4(int8_t());

#if HWY_CAP_INTEGER64
  const ForPartialVectors<TestPromoteTo<uint64_t>, 2> to_u64div2;
  to_u64div2(uint32_t());

  const ForPartialVectors<TestPromoteTo<int64_t>, 2> to_i64div2;
  to_i64div2(int32_t());
#endif

#if HWY_CAP_FLOAT64
  const ForPartialVectors<TestPromoteTo<double>, 2> to_f64div2;
  to_f64div2(int32_t());
  to_f64div2(float());
#endif
}

template <typename ToT>
struct TestDemoteTo {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, D from_d) {
    static_assert(sizeof(T) > sizeof(ToT), "Input type must be wider");
    const Rebind<ToT, D> to_d;

    const size_t N = Lanes(from_d);
    auto from = AllocateAligned<T>(N);
    auto expected = AllocateAligned<ToT>(N);

    // Narrower range in the wider type, for clamping before we cast
    const T min = LimitsMin<ToT>();
    const T max = LimitsMax<ToT>();

    RandomState rng;
    for (size_t rep = 0; rep < 200; ++rep) {
      for (size_t i = 0; i < N; ++i) {
        do {
          const uint64_t bits = rng();
          memcpy(&from[i], &bits, sizeof(T));
        } while (!std::isfinite(from[i]));
        expected[i] = static_cast<ToT>(std::min(std::max(min, from[i]), max));
      }

      HWY_ASSERT_VEC_EQ(to_d, expected.get(),
                        DemoteTo(to_d, Load(from_d, from.get())));
    }
  }
};

HWY_NOINLINE void TestAllDemoteTo() {
  ForDemoteVectors<TestDemoteTo<uint8_t>, 2>()(int16_t());
  ForDemoteVectors<TestDemoteTo<uint8_t>, 4>()(int32_t());

  ForDemoteVectors<TestDemoteTo<int8_t>, 2>()(int16_t());
  ForDemoteVectors<TestDemoteTo<int8_t>, 4>()(int32_t());

  const ForDemoteVectors<TestDemoteTo<int16_t>, 2> to_i16;
  to_i16(int32_t());

  const ForDemoteVectors<TestDemoteTo<uint16_t>, 2> to_u16;
  to_u16(int32_t());

#if HWY_CAP_FLOAT64
  const ForDemoteVectors<TestDemoteTo<float>, 2> to_float;
  to_float(double());

  const ForDemoteVectors<TestDemoteTo<int32_t>, 2> to_i32;
  to_i32(double());
#endif
}

struct TestConvertU8 {
  template <typename T, class D>
  HWY_NOINLINE void operator()(T /*unused*/, const D du32) {
    const Rebind<uint8_t, D> du8;
    auto lanes8 = AllocateAligned<uint8_t>(Lanes(du8));
    Store(Iota(du8, 0), du8, lanes8.get());
    HWY_ASSERT_VEC_EQ(du8, Iota(du8, 0), U8FromU32(Iota(du32, 0)));
    HWY_ASSERT_VEC_EQ(du8, Iota(du8, 0x7F), U8FromU32(Iota(du32, 0x7F)));
  }
};

HWY_NOINLINE void TestAllConvertU8() {
  ForDemoteVectors<TestConvertU8, 4>()(uint32_t());
}

struct TestIntFromFloat {
  template <typename TF, class DF>
  HWY_NOINLINE void operator()(TF /*unused*/, const DF df) {
    using TI = MakeSigned<TF>;
    const Rebind<TI, DF> di;
    const size_t N = Lanes(df);

    // Integer positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, TI(4)), ConvertTo(di, Iota(df, TF(4.0))));

    // Integer negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N)), ConvertTo(di, Iota(df, -TF(N))));

    // Above positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, TI(2)), ConvertTo(di, Iota(df, TF(2.001))));

    // Below positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, TI(3)), ConvertTo(di, Iota(df, TF(3.9999))));

    const TF eps = static_cast<TF>(0.0001);
    // Above negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N)),
                      ConvertTo(di, Iota(df, -TF(N + 1) + eps)));

    // Below negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N + 1)),
                      ConvertTo(di, Iota(df, -TF(N + 1) - eps)));
  }
};

struct TestFloatFromInt {
  template <typename TI, class DI>
  HWY_NOINLINE void operator()(TI /*unused*/, const DI di) {
    using TF = MakeFloat<TI>;
    const Rebind<TF, DI> df;
    const size_t N = Lanes(df);

    // Integer positive
    HWY_ASSERT_VEC_EQ(df, Iota(df, TF(4.0)), ConvertTo(df, Iota(di, TI(4))));

    // Integer negative
    HWY_ASSERT_VEC_EQ(df, Iota(df, -TF(N)), ConvertTo(df, Iota(di, -TI(N))));
  }
};

struct TestI32F64 {
  template <typename TF, class DF>
  HWY_NOINLINE void operator()(TF /*unused*/, const DF df) {
    using TI = int32_t;
    const Rebind<TI, DF> di;
    const size_t N = Lanes(df);

    // Integer positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, TI(4)), DemoteTo(di, Iota(df, TF(4.0))));
    HWY_ASSERT_VEC_EQ(df, Iota(df, TF(4.0)), PromoteTo(df, Iota(di, TI(4))));

    // Integer negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N)), DemoteTo(di, Iota(df, -TF(N))));
    HWY_ASSERT_VEC_EQ(df, Iota(df, -TF(N)), PromoteTo(df, Iota(di, -TI(N))));

    // Above positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, TI(2)), DemoteTo(di, Iota(df, TF(2.001))));
    HWY_ASSERT_VEC_EQ(df, Iota(df, TF(2.0)), PromoteTo(df, Iota(di, TI(2))));

    // Below positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, TI(3)), DemoteTo(di, Iota(df, TF(3.9999))));
    HWY_ASSERT_VEC_EQ(df, Iota(df, TF(4.0)), PromoteTo(df, Iota(di, TI(4))));

    const TF eps = static_cast<TF>(0.0001);
    // Above negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N)),
                      DemoteTo(di, Iota(df, -TF(N + 1) + eps)));
    HWY_ASSERT_VEC_EQ(df, Iota(df, TF(-4.0)), PromoteTo(df, Iota(di, TI(-4))));

    // Below negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N + 1)),
                      DemoteTo(di, Iota(df, -TF(N + 1) - eps)));
    HWY_ASSERT_VEC_EQ(df, Iota(df, TF(-2.0)), PromoteTo(df, Iota(di, TI(-2))));
  }
};

HWY_NOINLINE void TestAllConvertFloatInt() {
  ForFloatTypes(ForPartialVectors<TestIntFromFloat>());

  ForPartialVectors<TestFloatFromInt>()(int32_t());
#if HWY_CAP_FLOAT64 && HWY_CAP_INTEGER64
  ForPartialVectors<TestFloatFromInt>()(int64_t());
#endif

#if HWY_CAP_FLOAT64
  ForDemoteVectors<TestI32F64, 2>()(double());
#endif
}

struct TestNearestInt {
  template <typename TI, class DI>
  HWY_NOINLINE void operator()(TI /*unused*/, const DI di) {
    using TF = MakeFloat<TI>;
    const Rebind<TF, DI> df;
    const size_t N = Lanes(df);

    // Integer positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, 4), NearestInt(Iota(df, 4.0f)));

    // Integer negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -32), NearestInt(Iota(df, -32.0f)));

    // Above positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, 2), NearestInt(Iota(df, 2.001f)));

    // Below positive
    HWY_ASSERT_VEC_EQ(di, Iota(di, 4), NearestInt(Iota(df, 3.9999f)));

    const TF eps = static_cast<TF>(0.0001);
    // Above negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N)), NearestInt(Iota(df, -TF(N) + eps)));

    // Below negative
    HWY_ASSERT_VEC_EQ(di, Iota(di, -TI(N)), NearestInt(Iota(df, -TF(N) - eps)));
  }
};

HWY_NOINLINE void TestAllNearestInt() {
  ForPartialVectors<TestNearestInt>()(int32_t());
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
}  // namespace HWY_NAMESPACE
}  // namespace hwy
HWY_AFTER_NAMESPACE();

#if HWY_ONCE
HWY_BEFORE_TEST(HwyConvertTest);
HWY_EXPORT_AND_TEST_P(HwyConvertTest, TestAllBitCast);
HWY_EXPORT_AND_TEST_P(HwyConvertTest, TestAllPromoteTo);
HWY_EXPORT_AND_TEST_P(HwyConvertTest, TestAllDemoteTo);
HWY_EXPORT_AND_TEST_P(HwyConvertTest, TestAllConvertU8);
HWY_EXPORT_AND_TEST_P(HwyConvertTest, TestAllConvertFloatInt);
HWY_EXPORT_AND_TEST_P(HwyConvertTest, TestAllNearestInt);
HWY_AFTER_TEST();
#endif
