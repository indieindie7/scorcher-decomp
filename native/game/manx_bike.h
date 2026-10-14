// Manx TT Super Bike (Saturn, PAL) bike model as one engine-agnostic function.
// Sources: ManxTT/HANDOFF-scorcher.md, ManxTT/ghidra/study/study_handling.md (functions car_UpdateThrottleSpeed 0x06032918,
// bike_UpdateGear 0x06037728, car_UpdateLeanBalance 0x06031F14, bike_UpdateSteeringTorque 0x060384CC).
// Units: metres, seconds, radians. 10 Manx world units = 1 m (one tick moves speed*0.4 units); dt = 0.04 s (PAL tick).
// To get Scorcher units: position per tick = speed * dt * 10 * 5796 (the mxcam port's 5796 per Manx unit) = 2318 per m/s.
// Not wired into the game yet. STATUS: the engine/gear/drag/brake/slope chain follows the decompile; the steering and
// lean-to-heading part is an APPROXIMATION (spring 300 and damping 0.51, lean angle -atan2(lean, 1.41) are exact;
// the steer shaping gain, speed scaling and the lean -> heading rate are fitted, see the comments).
#pragma once
#include <algorithm>
#include <cmath>

namespace manx {

struct BikeIn {
    double throttle = 0;     // 0..1
    double brake = 0;        // 0..1 (Manx uses 1.0 or 1.5)
    double steer = 0;        // -1 (left) .. 1 (right), already read through the game's key bindings
    double slope = 0;        // gravity along the slope, nz' of the surface normal in the bike frame (uphill < 0 decelerates, Manx y is down)
    int surface = 0;         // 0 road, 1 rough (engine x0.9), 3 grass (engine x0.6)
    bool airborne = false;   // after 5+ airborne ticks only drag acts
};

struct BikeState {
    double speed = 0;        // m/s along the bike
    double rpm = 1300;       // 1300..15000
    int gear = 1;            // 1..6
    int shift = 0;           // shift delay ticks left
    double steer = 0;        // shaped steer axis, decays 0x22/4096 per tick when released
    double lean = 0;         // lean displacement (m), spring-damper
    double lean_v = 0;
    double roll = 0;         // lean angle (rad), + = lean right
    double heading = 0;      // yaw (rad), + = turn right; integrate it into the car's forward vector
    int air_ticks = 0;
};

// rpm/100 indexed torque curve (0x0603AF7C), 151 entries, 0 beyond.
static const double k_torque[151] = {
    3.43, 3.56, 3.7, 3.83, 3.96, 4.09, 4.22, 4.35, 4.48, 4.6, 4.73, 4.85, 4.98, 5.1, 5.21, 5.33, 5.44, 5.56, 5.67, 5.77, 5.88,
    5.98, 6.08, 6.17, 6.26, 6.35, 6.44, 6.52, 6.6, 6.67, 6.74, 6.81, 6.87, 6.93, 6.98, 7.03, 7.08, 7.12, 7.16, 7.19, 7.22, 7.25,
    7.27, 7.28, 7.3, 7.3, 7.3, 7.33, 7.36, 7.38, 7.41, 7.43, 7.46, 7.48, 7.51, 7.53, 7.55, 7.57, 7.59, 7.61, 7.63, 7.64, 7.65,
    7.67, 7.68, 7.69, 7.7, 7.7, 7.71, 7.71, 7.71, 7.7, 7.69, 7.66, 7.63, 7.59, 7.54, 7.49, 7.44, 7.38, 7.32, 7.27, 7.22, 7.17,
    7.13, 7.09, 7.07, 7.06, 7.05, 7.06, 7.08, 7.11, 7.15, 7.21, 7.27, 7.34, 7.42, 7.5, 7.58, 7.66, 7.74, 7.82, 7.89, 7.95, 8.01,
    8.05, 8.08, 8.1, 8.11, 8.11, 8.1, 8.08, 8.06, 8.04, 8.01, 7.97, 7.93, 7.89, 7.84, 7.79, 7.73, 7.67, 7.61, 7.54, 7.48, 7.41,
    7.34, 7.27, 7.19, 7.1, 7.0, 6.91, 6.81, 6.72, 6.62, 6.53, 6.43, 6.34, 6.24, 6.15, 6.05, 5.96, 5.86, 5.77, 5.67, 5.58, 5.48,
    5.39, 5.29, 5.2};
static const double k_ratio[7] = {0, 2.4, 1.941, 1.631, 1.434, 1.291, 1.192};
static const double k_eff[7] = {1.0, 0.92, 0.9, 0.88, 0.86, 0.83, 0.8};            // 0x0606F680 (0x0606F69C is the 1.125.. set)
constexpr double k_mass = 237 + 60, k_mass_extra = 12;                            // a = F*9.81/(m+12)
constexpr double k_cd = 0.502;                                                     // 0x8083 (0.3765 on the other bike); drag = 0.0334*Cd*v^2
constexpr double k_brake = 500;                                                    // kgf (stat table 0x0606F6D4, 0x1f4)
constexpr double k_wheelbase = 1.41;
constexpr double k_dt = 0.04;

inline void step(BikeState& s, const BikeIn& in, double dt = k_dt) {
    // rpm: target = speed*4.5605*30.715*ratio, eased 0.54 falling / 0.67 rising, snap within 200
    const double ratio = k_ratio[s.gear];
    const double target = std::clamp(s.speed * 4.5605 * 30.715 * ratio, 1300.0, 15000.0);
    if (std::fabs(target - s.rpm) < 200) s.rpm = target;
    else s.rpm += (target - s.rpm) * (target > s.rpm ? 0.67 : 0.54);
    if (s.shift > 0) --s.shift;
    else if (s.rpm > 13000 && s.gear < 6) { ++s.gear; s.shift = 26; }
    else if (s.rpm < 10000 && s.gear > 1) { --s.gear; s.shift = 26; }

    if (in.airborne) ++s.air_ticks; else s.air_ticks = 0;
    const double drag = 0.0334 * k_cd * s.speed * s.speed;
    double F;
    if (s.air_ticks >= 5) F = -drag;
    else {
        const int idx = std::clamp((int)(s.rpm / 100), 0, 150);
        double eng = k_torque[idx] * in.throttle * 14.67 * ratio * k_eff[s.gear];
        eng -= s.rpm * 0.01 * (1 - in.throttle) * ratio;                          // engine braking
        eng *= in.surface == 3 ? 0.6 : in.surface == 1 ? 0.9 : 1.0;
        F = eng + k_mass * in.slope + 0.012 * k_mass - drag;       // m*nz' (uphill nz' < 0) + small forward push
        F -= in.brake * k_brake;
    }
    s.speed = std::max(0.0, s.speed + F * 9.81 / (k_mass + k_mass_extra) * dt);

    // steering: cubic-ish shaping, scaled down with speed (fitted), steer decays when released
    const double x = in.steer;
    if (x != 0) s.steer = std::clamp(s.steer + (x + x * std::fabs(x) * x) * 0.5 * 0.3, -1.0, 1.0);
    else s.steer -= std::clamp(s.steer, -0.0085, 0.0085) * 1.0;                    // 0x22/4096 per tick
    const double sp = std::max(s.speed, 0.01);
    const double gain = sp > 25 ? 25 / sp : 1;                                     // divide by speed above 25 m/s
    // lean spring-damper: spring 300, damping 0.51 (Manx car_UpdateLeanBalance); the lean target is a fitted gain
    const double target_lean = s.steer * gain * 0.5;
    s.lean_v = (s.lean_v + 300 * dt * (target_lean - s.lean)) * 0.51;               // damping 0.51 per tick (as the native lean mod)
    s.lean += s.lean_v * dt;
    s.roll = -std::atan2(s.lean, k_wheelbase);
    // heading from lean (fitted): bicycle turn rate = v*tan(lean angle)/wheelbase
    s.heading += s.speed * std::tan(-s.roll) / (k_wheelbase * 10) * dt;
}

}  // namespace manx
