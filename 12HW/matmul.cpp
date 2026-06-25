#include "Halide.h"
#include "common.h"
#include <cstdio>

using namespace Halide;
using namespace Halide::Tools;

void simple_version(float *A, float *B, float *C, int width, int stride) {
    for (int iy = 0; iy < width; iy++) {
        for (int ix = 0; ix < width; ix++) {
            float *cc = C + iy * stride + ix;
            *cc = 0.0f;

            for (int ik = 0; ik < width; ik++) {
                *cc = *cc + A[iy * stride + ik] * B[ik * stride + ix];
            }
        }
    }
}

int main(int argc, char **argv) {
    const int matrix_size = 992;

    ImageParam A(type_of<float>(), 2);
    ImageParam B(type_of<float>(), 2);

    // define matrix multiplication algorithm
    Var x("x"), xi("xi"), xo("xo"), y("y"), yo("yo"), yi("yi"), yii("yii"), xii("xii");
    Func matrix_mul("matrix_mul");

    RDom k(0, matrix_size);
    RVar ki;

    matrix_mul(x, y) += A(k, y) * B(x, k);

    Func out;
    out(x, y) = matrix_mul(x, y);

    Var xy;

    // schedules
    out.tile(x, y, xi, yi, 24, 32)
        .fuse(x, y, xy)
        .parallel(xy)
        .split(yi, yi, yii, 4)
        .vectorize(xi, 8)
        .unroll(xi)
        .unroll(yii);

    matrix_mul.compute_at(out, yi).vectorize(x, 8).unroll(y);

    matrix_mul.update(0)
        .reorder(x, y, k)
        .vectorize(x, 8)
        .unroll(x)
        .unroll(y)
        .unroll(k, 2);

    Buffer<float, 2> mat_A(matrix_size, matrix_size);
    Buffer<float, 2> mat_B(matrix_size, matrix_size);
    Buffer<float, 2> output_halide(matrix_size, matrix_size);

    // init randomly
    random_data<float, 2>(mat_A);
    random_data<float, 2>(mat_B);
    A.set(mat_A);
    B.set(mat_B);

    // jit compile and warm-up
    out.realize(output_halide);

    double t_halide = benchmark(10, 10, [&]() { out.realize(output_halide); });

    // call dnn sgemm
    Buffer<float, 2> output_ref(matrix_size, matrix_size);
    double t_onednn = benchmark([&]() {
        // simple_version(mat_A.data(), mat_B.data(), output_ref.data(), mat_A.width(), mat_A.stride(1));
        dnnl_sgemm('N', 'N', matrix_size, matrix_size, matrix_size, 1.0f,
                   mat_A.data(), matrix_size, mat_B.data(), matrix_size, 0.0f,
                   output_ref.data(), matrix_size);
    });

    // check results
    if (check_equal<float, 2>(output_ref, output_halide)) {
        printf("Halide results - OK\n");
    } else {
        printf("Halide results - FAIL\n");
        return 1;
    }

    float gflops = 2.0f * matrix_size * matrix_size * matrix_size / 1e9f;

    printf("Halide: %fms, %f GFLOP/s\n", t_halide * 1e3, (gflops / t_halide));
    printf("oneDNN: %fms, %f GFLOP/s\n\n", t_onednn * 1e3, (gflops / t_onednn));

    printf("Success!\n");
    return 0;
}
