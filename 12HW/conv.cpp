#include "Halide.h"
#include "common.h"

#include <stdio.h>

using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    const int N = 5, CI = 128, CO = 128, W = 100, H = 80, KW = 3, KH = 3;

    ImageParam input(type_of<float>(), 4);
    ImageParam filter(type_of<float>(), 4);

    // define convolution algorithm
    Var x("x"), y("y"), c("c"), n("n");

    Func conv("conv"), out("out");
    RDom r(0, CI, 0, KW, 0, KH);

    conv(c, x, y, n) = 0.0f;
    conv(c, x, y, n) += filter(c, r.y, r.z, r.x) * input(r.x, x + r.y, y + r.z, n);

    out(c, x, y, n) = conv(c, x, y, n);

    // schedules
    Target target = get_jit_target_from_environment();
    const int vec = target.natural_vector_size<float>();
    int tile_w = 3;
    int tile_h = 4;
    Var co("co"), ci("ci"), xo("xo"), xi("xi"), yo("yo"), yi("yi"), t("t");
    out.split(c, co, ci, vec * tile_w)
        .split(x, xo, xi, tile_h)
        .reorder(ci, xi, xo, y, n, co)
        .vectorize(ci, vec)
        .unroll(ci)
        .unroll(xi)
        .parallel(y)
        .parallel(n)
        .parallel(co);
    conv.compute_at(out, xo)
        .vectorize(c, vec)
        .unroll(c)
        .unroll(x)
        .unroll(y)
        .update()
        .reorder(c, x, y, r.x, r.y, r.z, n)
        .vectorize(c, vec)
        .unroll(c)
        .unroll(x)
        .unroll(y);
	.unroll(r.x, 2);
    filter.in().compute_at(conv, r.x).vectorize(_0, vec).unroll(_0).unroll(_3);
    input.in().compute_at(conv, x).unroll(_0);

    Buffer<float, 4> in(CI, W + KW - 1, H + KH - 1, N);
    Buffer<float, 4> fil(CO, KW, KH, CI);
    Buffer<float, 4> output_halide(CO, W, H, N);

    // init randomly
    random_data<float, 4>(in);
    random_data<float, 4>(fil);
    input.set(in);
    filter.set(fil);

    // jit compile and warm-up
    out.realize(output_halide);

    double t_halide = benchmark(10, 10, [&]() { out.realize(output_halide); });

    Buffer<float, 4> output_ref(CO, W, H, N);
    // create and execute a conv primitive using oneDNN
    double t_onednn = dnnl_dilated_conv_wrapper(in.data(), fil.data(), output_ref.data(), {N, CI, CO, W, H, KW, KH, 0, 0});

    // check results
    if (check_equal<float, 4>(output_ref, output_halide)) {
        printf("Halide results - OK\n");
    } else {
        printf("Halide results - FAIL\n");
        return 1;
    }

    float gflops = 2.0f * (N * CO * H * W) * (CI * KH * KW) / 1e9f;

    printf("Halide: %fms, %f GFLOP/s\n", t_halide * 1e3, (gflops / t_halide));
    printf("oneDNN: %fms, %f GFLOP/s\n\n", t_onednn * 1e3, (gflops / t_onednn));

    printf("Success!\n");

    return 0;
}
