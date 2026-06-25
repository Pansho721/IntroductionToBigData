#include "Halide.h"
#include "common.h"

#include <stdio.h>

using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    const int N = 5, CI = 128, CO = 128, W = 100, H = 80, KW = 3, KH = 3;
    const int dilation = 15;

    ImageParam input(type_of<float>(), 4);
    ImageParam filter(type_of<float>(), 4);

    // define dilated convolution
    // you can also rewrite algorithm definition part, as long as results are correct
    Var x("x"), y("y"), c("c"), n("n");

    Func dilated_conv("dilated_conv"), out("out");
    RDom r(0, CI, 0, KW, 0, KH);

    dilated_conv(c, x, y, n) = 0.0f;
    dilated_conv(c, x, y, n) += filter(c, r.y, r.z, r.x) * input(r.x, x + r.y * (dilation + 1), y + r.z * (dilation + 1), n);

    out(c, x, y, n) = dilated_conv(c, x, y, n);
    
    Var co("co"), ci("ci");
    Var xo("xo"), yo("yo"), xi("xi"), yi("yi");
    int vec = 16;
    // Output schedule: vectorize channels, parallelize over batches and output channels
    out.split(c, co, ci, vec)
        .reorder(ci, x, y, co, n)
        .vectorize(ci, vec)
        .tile(x,y,xo,yo,xi,yi,vec,vec)
        .parallel(n)
        .parallel(co);
    
    // Reduction schedule: process in order that maximizes input reuse
    dilated_conv.compute_at(out, n)
        .vectorize(c, vec)
        .update()
        .reorder(r.x, r.y, r.z, c, x, y)
        .vectorize(c, vec)
        .unroll(r.y)
        .unroll(r.z)
        .parallel(x)
        .parallel(y);
    
    // Prefetch input data
    input.in().compute_at(dilated_conv, x);
    filter.in().compute_at(dilated_conv, c);

    Buffer<float, 4> in(CI, W + (KW - 1) * (dilation + 1), H + (KH - 1) * (dilation + 1), N);
    Buffer<float, 4> fil(CO, KW, KH, CI);
    Buffer<float, 4> output_halide(CO, W, H, N);

    // init randomly
    random_data<float, 4>(in);
    random_data<float, 4>(fil);
    input.set(in);
    filter.set(fil);

    // jit compile and warm-up
    dilated_conv.realize(output_halide);
    // NOTE: uncomment next line if time is unstable
    // double t_halide = benchmark(10, 10, [&]() { dilated_conv.realize(output_halide); });
    double t_halide = benchmark(1, 1, [&]() { dilated_conv.realize(output_halide); });

    Buffer<float, 4> output_ref(CO, W, H, N);
    // create and execute a dilated conv primitive using oneDNN
    double t_onednn = dnnl_dilated_conv_wrapper(in.data(), fil.data(), output_ref.data(), {N, CI, CO, W, H, KW, KH, dilation, dilation});

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
