#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#define REQUIRE(cond)                                                          \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "REQUIRE failed: " #cond " at line " << __LINE__      \
                      << '\n';                                                 \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

// 不接真实时钟，全部用合成帧时间驱动，保证 CTest 结果可复现。
//
// 时间刻度：1 刻度 = 1/600 秒。帧时间与步长都换成整数刻度再累积——
// 浮点秒数累加会漂移（本例 60 步会被吃成 59 步），整数累积精确无偏。
constexpr int kTicksPerSecond = 600;
constexpr int kStepTicks = 10; // 步长 10 刻度 = 1/60 秒
constexpr int kMaxFrameTicks = 150; // 帧时间钳制 = 0.25 秒
constexpr int kFramesSpanTicks = 600; // 比较区间总长 = 1 秒

// ---------------------------------------------------------------------------
// 0. 被模拟的对象：一维弹道，半隐式欧拉积分
// ---------------------------------------------------------------------------

struct Ballistic {
    double x = 0.0;
    double v = 30.0;
};

constexpr double kGravity = -10.0;
constexpr double kStartV = 30.0;

void step(Ballistic& body, double dt) {
    body.v += kGravity * dt; // 半隐式欧拉：先更新速度
    body.x += body.v * dt;   // 再用新速度更新位置
}

// 解析精确解，用来量化数值误差。
double exact_x(double t) { return kStartV * t + 0.5 * kGravity * t * t; }

double abs_diff(double a, double b) { return a > b ? a - b : b - a; }

// ---------------------------------------------------------------------------
// 1. 变步长：dt 直接取帧时长，帧时长抖动放大误差
// ---------------------------------------------------------------------------

double run_variable_step(const std::vector<int>& frame_ticks) {
    Ballistic body;
    for (int ticks : frame_ticks) {
        step(body, static_cast<double>(ticks) / kTicksPerSecond);
    }
    return body.x;
}

// ---------------------------------------------------------------------------
// 2. 固定步长 + 累加器：模拟与渲染解耦
// ---------------------------------------------------------------------------

struct FixedLoopResult {
    double final_x = 0.0;
    int steps_run = 0;
    double alpha = 0.0;       // 帧内插值系数 [0, 1)
    double prev_step_x = 0.0; // 上一步模拟位置
    double curr_step_x = 0.0; // 当前步模拟位置
    double render_x = 0.0;    // 交给渲染的插值位置
};

FixedLoopResult run_fixed_step(const std::vector<int>& frame_ticks) {
    FixedLoopResult result;
    Ballistic body;
    Ballistic previous = body;
    int accumulator = 0; // 整数刻度累积，永不漂移

    for (int ticks : frame_ticks) {
        // 钳制帧时长：掉帧尖峰一次性补太多步会引发「死亡螺旋」
        // （补步耗时更长 → 下一帧欠账更多 → 永远追不上）。
        accumulator += std::min(ticks, kMaxFrameTicks);

        while (accumulator >= kStepTicks) {
            previous = body;
            step(body, static_cast<double>(kStepTicks) / kTicksPerSecond);
            accumulator -= kStepTicks;
            ++result.steps_run;
        }

        // 渲染插值：剩余刻度是「模拟领先渲染」的量，alpha 表示渲染
        // 时刻落在上一状态与当前状态之间的比例。
        result.alpha = static_cast<double>(accumulator) / kStepTicks;
        result.prev_step_x = previous.x;
        result.curr_step_x = body.x;
        result.render_x = previous.x + (body.x - previous.x) * result.alpha;
    }

    result.final_x = body.x;
    return result;
}

// 帧时间序列：0.1 倍与 1.9 倍步长交替（≈1.7 ms 快帧、≈31.7 ms 慢帧），
// 总和精确等于 kFramesSpanTicks——两个方案覆盖完全相同的模拟时长。
std::vector<int> jittered_frames(int total_ticks) {
    std::vector<int> frames;
    int emitted = 0;
    int toggle = 0;
    while (emitted + kStepTicks <= total_ticks) {
        const int pair[] = {kStepTicks / 10 * 1, kStepTicks / 10 * 19};
        const int next = pair[toggle % 2];
        toggle ^= 1;
        if (emitted + next > total_ticks) {
            break;
        }
        frames.push_back(next);
        emitted += next;
    }
    while (emitted < total_ticks) { // 余额补一个 1 刻度的帧
        frames.push_back(1);
        ++emitted;
    }
    return frames;
}

void compare_schemes() {
    const std::vector<int> frames = jittered_frames(kFramesSpanTicks);

    const double variable_x = run_variable_step(frames);
    const FixedLoopResult fixed = run_fixed_step(frames);

    const double total_seconds =
        static_cast<double>(kFramesSpanTicks) / kTicksPerSecond;
    const double variable_error = abs_diff(variable_x, exact_x(total_seconds));
    const double fixed_error = abs_diff(fixed.final_x, exact_x(total_seconds));

    std::cout << "steps=" << fixed.steps_run
              << " alpha=" << fixed.alpha
              << " variable dt error: " << variable_error
              << ", fixed dt error: " << fixed_error << '\n';

    // 整数刻度下步数精确：总长 600 刻度 / 步长 10 = 恰好 60 步。
    REQUIRE(fixed.steps_run == 60);
    // 步长忽大忽小时误差更大；等长小步的误差更小且可预测。半隐式欧
    // 拉对恒定加速度的速度是精确的，误差只出在位置的加权积分上。
    REQUIRE(fixed_error < variable_error);
    REQUIRE(fixed.alpha >= 0.0 && fixed.alpha < 1.0);
    const double lo = std::min(fixed.prev_step_x, fixed.curr_step_x);
    const double hi = std::max(fixed.prev_step_x, fixed.curr_step_x);
    REQUIRE(fixed.render_x >= lo && fixed.render_x <= hi); // 插值夹在两态间
}

// ---------------------------------------------------------------------------
// 3. 渲染插值：剩余刻度不足一步时，渲染读上一状态与当前状态的混合
// ---------------------------------------------------------------------------

void interpolation_demo() {
    // 一帧送进 15 刻度（1.5 步）：跑 1 步，剩 5 刻度 → alpha = 0.5。
    Ballistic body;
    Ballistic previous = body;
    int accumulator = 15;
    double alpha = 0.0;
    double render_x = 0.0;

    while (accumulator >= kStepTicks) {
        previous = body;
        step(body, static_cast<double>(kStepTicks) / kTicksPerSecond);
        accumulator -= kStepTicks;
    }
    alpha = static_cast<double>(accumulator) / kStepTicks;
    render_x = previous.x + (body.x - previous.x) * alpha;

    REQUIRE(alpha == 0.5);
    const double lo = std::min(previous.x, body.x);
    const double hi = std::max(previous.x, body.x);
    REQUIRE(render_x >= lo && render_x <= hi);

    // 纪律：插值状态只服务渲染。游戏逻辑读的永远是 body（当前模拟态），
    // 读 render_x 会让物理结果依赖渲染节奏。
}

// ---------------------------------------------------------------------------
// 4. 帧时间钳制：0.5 秒掉帧尖峰最多补 0.25 秒的步
// ---------------------------------------------------------------------------

void clamp_spike() {
    // 正常帧之后跟一个 0.5 秒的大尖峰，再恢复正常。
    const std::vector<int> frames{kStepTicks, kFramesSpanTicks / 2, kStepTicks};

    Ballistic body;
    int accumulator = 0;
    int steps_on_spike_frame = 0;

    for (std::size_t i = 0; i < frames.size(); ++i) {
        accumulator += std::min(frames[i], kMaxFrameTicks);
        int steps_this_frame = 0;
        while (accumulator >= kStepTicks) {
            step(body, static_cast<double>(kStepTicks) / kTicksPerSecond);
            accumulator -= kStepTicks;
            ++steps_this_frame;
        }
        if (i == 1) {
            steps_on_spike_frame = steps_this_frame;
        }
    }

    // 0.5 秒被钳到 0.25 秒：尖峰帧最多补 15 步（150/10），而不是 30 步；
    // 被裁掉的 0.25 秒表现为游戏整体慢放一瞬，而不是连续卡死。
    REQUIRE(steps_on_spike_frame == 15);
    std::cout << "spike frame ran " << steps_on_spike_frame
              << " steps (clamped from 30)\n";
}

int main() {
    compare_schemes();
    interpolation_demo();
    clamp_spike();
    std::cout << "ch04_game_loop: all checks passed\n";
    return 0;
}
