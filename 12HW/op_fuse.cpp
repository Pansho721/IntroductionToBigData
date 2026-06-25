#include "Halide.h"
#include "common.h"

#include <stdio.h>

using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    const int N = 5, CI = 128, CO = 128, W = 100, H = 80, KW = 3, KH = 3;
    const int dilation = 31;
    const float epsilon = 1.e-9f;

    ImageParam input(type_of<float>(), 4);
    ImageParam filter(type_of<float>(), 4);

    // define a fused operator
    // TODO: you may rewrite this part
    Var x("x"), y("y"), c("c"), n("n");

    Func dilated_conv("dilated_conv");
    RDom r(0, CI, 0, KW, 0, KH);

    dilated_conv(c, x, y, n) = 0.0f;
    dilated_conv(c, x, y, n) += filter(c, r.y, r.z, r.x) * input(r.x, x + r.y * (dilation + 1), y + r.z * (dilation + 1), n);

    Func mu("mu"), sigma("sigma"), out("out");
    Func inv_sqrt("inv_sqrt");
    RDom s(0, W, 0, H, 0, N);

    mu(c) = Halide::sum(dilated_conv(c, s.x, s.y, s.z)) / (N * H * W);

    sigma(c) = Halide::sum(pow((dilated_conv(c, s.x, s.y, s.z) - mu(c)), 2)) / (N * H * W);
    inv_sqrt(c) = 1 / sqrt(sigma(c) + epsilon);

    out(c, x, y, n) = (dilated_conv(c, x, y, n) - mu(c)) * inv_sqrt(c);
    
    // TODO: write Halide schedules below
    dilated_conv.compute_root();
    mu.compute_root();
    inv_sqrt.compute_root();

    Buffer<float, 4> in(CI, W + (KW - 1) * (dilation + 1), H + (KH - 1) * (dilation + 1), N);
    Buffer<float, 4> fil(CO, KW, KH, CI);
    Buffer<float, 4> output_halide(CO, W, H, N);

    // init randomly
    random_data<float, 4>(in);
    random_data<float, 4>(fil);
    input.set(in);
    filter.set(fil);

    // jit compile and warm-up
    out.realize(output_halide);
    // NOTE: uncomment next line if time is unstable
    // double t_halide = benchmark(10, 10, [&]() { dilated_conv.realize(output_halide); });
    double t_halide = benchmark(1, 1, [&]() { out.realize(output_halide); });

    Buffer<float, 4> output_ref(CO, W, H, N);
    // call dilated conv and bnorm seperately in oneDNN
    double t_onednn = dnnl_dilated_conv_wrapper(in.data(), fil.data(), output_ref.data(), {N, CI, CO, W, H, KW, KH, dilation, dilation});
    t_onednn += dnnl_batch_normalization_wrapper(output_ref.data(), epsilon, {N, CO, H, W});

    // check results
    if (check_equal<float, 4>(output_ref, output_halide)) {
        printf("Halide results - OK\n");
    } else {
        printf("Halide results - FAIL\n");
        return 1;
    }

    printf("Halide: %fms\n", t_halide * 1e3);
    printf("oneDNN: %fms\n\n", t_onednn * 1e3);

    printf("Success!\n");

    return 0;
}
