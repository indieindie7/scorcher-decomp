// Scorcher's game layer for saturn.exe: the player car's state each game
// tick to a CSV (a dataset for a taught driver), and a driver that holds
// accelerate and steers by the pad toward a recorded racing line.
//
// The build also hooks the slave's idle loop where its job ring is empty
// (LORT:06005F9A, `mov #100,r0` before a delay and a recheck): the slave gives the
// CPU back at once instead of spinning out its slice (2/3 of a race's host time).
//
// The build hooks bilstyring_RateLoop's entry (recomp --hook LORT:060DAD38):
// r4 = the car's world (WorldRAMHeader), r5 = its wram, once a game tick
// (two VBlanks). Fields (NOTES.md "Native pass 14"):
//   world+0x20..0x28   position (z is up on The Dump)
//   wram+0x0C..0x14    velocity (world axes)
//   world+0x98         the shared record; its +0x24 the track record:
//     trk+0x1C         distance along the track (track_SegDistance)
//     trk+0x08..0x10   the nearest point of the track spline (not the road's middle)
//     trk+0x50         the track basis, 3x3 rows in 2.30: row 0 along the track,
//                      row 1 across, row 2 the third axis
//
//   --game state=FILE        one row per game tick: tick,vblank,pad,pos,vel,progress, the car
//                            from the spline point in track space (along,lat,up), the velocity
//                            in track space, the world, the spline point and the basis
//   --game line=FILE         a state CSV whose path is the racing line to follow (the game AI's
//                            or a human's run): its lateral offset by progress
//   --game drive=FROM        from VBlank FROM the driver holds accelerate and steers
//   --game accel=B           the accelerate button (B on the default pad)
//   --game kp=,kd=,dead=     steer = kp*(lat - line)/30000 + kd*vlat/|valong| (+ dead zone)
//   --game look=N            the line looked up N ticks of travel ahead
//   --game plan=FILE         a plan (scratchpad plan.py: bin,progress,lat,vcap,..): the line to follow
//                            and a speed cap ahead (above it coast, above 1.15x brake with DOWN)
//   --game sign=1            -1 flips LEFT/RIGHT
//   --game explore=SEED      random spells of throttle (B, none, A, DOWN, C) and steer (the controller,
//                            LEFT, RIGHT, none): data to fit a model of the car
//
// Flyover (concept mock-ups; hooks LORT:060F6F4E after cam_BuildFrameCamera's matrix copy,
// LORT:0600DF16 rend_CollectObjects' entry):
//   --game fly=dump          print CameraRAM (*060FCBA8: eye +4..+0xC, matrix +0x4C, +0x70..+0x8C) every 60 VBlanks
//   --game fly=1             the orbit camera below
//   --game all=1             every track object drawn (the LOD window 0600DE88/0600DE8C opened to the whole ring)
//   --game fog=K             the depth shade table (*060FC7C8, 2048 bytes) stretched K times; 0 = no fog
//   --game far=K             CameraRAM+0x70..+0x7C (fog/far distances) times K
//   --game cx=,cy=,cz=,h=,r=,yaw=,spin=,flyfrom=VB   with fly=1: orbit camera (world y down), from VBlank flyfrom
#include "saturn.h"
#include "video.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include "manx_bike.h"   // engine-agnostic Manx bike model (not wired in yet)

namespace {

FILE* g_state;
uint64_t g_ticks;
uint16_t g_pad_seen;            // the buttons the program read last (after the driver)
uint64_t g_car_vb = ~0ull;      // the VBlank on_car last ran (a race is on; menu_keys)
uint64_t g_drive_from = ~0ull;
uint16_t g_accel = 0x0100;      // B
uint16_t g_steer;               // LEFT/RIGHT the driver holds now
double g_kp = 1.0, g_kd = 2.0, g_dead = 0.05, g_look = 6;
int g_sign = 1;
uint32_t g_explore;             // --game explore=SEED: random throttle and steer spells (data to fit a car model)
uint16_t g_throttle = 0x0100;   // the throttle buttons held now
int g_spell, g_steer_mode;
uint32_t rnd() { g_explore ^= g_explore << 13; g_explore ^= g_explore >> 17; g_explore ^= g_explore << 5; return g_explore; }

// the racing line: lateral offset by progress, in bins
const double kBin = 100000;
std::vector<double> g_line;     // NaN where the line never went
double g_line_len;              // the progress the bins span

uint16_t button(const std::string& n) {
    static const struct { const char* name; uint16_t bit; } k[] = {
        {"A", 0x0400}, {"B", 0x0100}, {"C", 0x0200}, {"X", 0x0040}, {"Y", 0x0020}, {"Z", 0x0010},
        {"R", 0x0080}, {"L", 0x0008}, {"UP", 0x1000}, {"DOWN", 0x2000}};
    for (auto& b : k) if (n == b.name) return b.bit;
    sat_fatal("--game accel=%s: no such button", n.c_str());
}

// into the track's basis (rows, 2.30): what vec_ToBasis does
void to_basis(const double m[9], const double v[3], double out[3]) {
    for (int r = 0; r < 3; ++r) out[r] = m[r * 3] * v[0] + m[r * 3 + 1] * v[1] + m[r * 3 + 2] * v[2];
}

void load_line(const std::string& path) {
    std::ifstream f(path);
    if (!f) sat_fatal("--game line: cannot read %s", path.c_str());
    std::string row;
    std::getline(f, row);                                  // the header
    std::vector<double> sum, n;
    while (std::getline(f, row)) {
        std::vector<double> c;
        std::stringstream ss(row);
        std::string cell;
        while (std::getline(ss, cell, ',')) c.push_back(std::strtod(cell.c_str(), nullptr));
        if (c.size() < 12) continue;
        double progress = c[9], lat = c[11];
        if (progress < 0) continue;
        size_t b = (size_t)(progress / kBin);
        if (b >= sum.size()) { sum.resize(b + 1, 0); n.resize(b + 1, 0); }
        sum[b] += lat; n[b] += 1;
    }
    g_line.assign(sum.size(), NAN);
    for (size_t i = 0; i < sum.size(); ++i) if (n[i]) g_line[i] = sum[i] / n[i];
    g_line_len = sum.size() * kBin;
    sat_note("scorcher: racing line %s, %zu bins", path.c_str(), g_line.size());
}

// --game plan=FILE (plan.py): per bin of progress the target lateral offset and a speed cap
std::vector<double> g_plan_lat, g_plan_cap;
double g_plan_bin = 1, g_plan_len;

void load_plan(const std::string& path) {
    std::ifstream f(path);
    if (!f) sat_fatal("--game plan: cannot read %s", path.c_str());
    std::string row;
    std::getline(f, row);                                  // bin,progress,lat,vcap,nsafe,ndoom
    while (std::getline(f, row)) {
        double c[4] = {};
        std::stringstream ss(row);
        std::string cell;
        for (int i = 0; i < 4 && std::getline(ss, cell, ','); ++i) c[i] = std::strtod(cell.c_str(), nullptr);
        if (g_plan_lat.size() == 1) g_plan_bin = c[1];
        g_plan_lat.push_back(c[2]);                        // NaN ("nan") where no car went
        g_plan_cap.push_back(c[3]);
    }
    if (g_plan_lat.size() < 2) sat_fatal("--game plan: %s has no bins", path.c_str());
    g_plan_len = g_plan_lat.size() * g_plan_bin;
    sat_note("scorcher: plan %s, %zu bins of %.0f", path.c_str(), g_plan_lat.size(), g_plan_bin);
}

size_t plan_bin(double progress) {
    double p = std::fmod(progress, g_plan_len);
    if (p < 0) p += g_plan_len;
    return (size_t)(p / g_plan_bin) % g_plan_lat.size();
}

double line_at(double progress) {
    if (!g_plan_lat.empty()) {
        size_t b = plan_bin(progress), m = g_plan_lat.size();
        for (size_t k = 0; k < m; ++k) if (!std::isnan(g_plan_lat[(b + k) % m])) return g_plan_lat[(b + k) % m];
        return 0;
    }
    if (g_line.empty()) return 0;
    double p = std::fmod(progress, g_line_len);
    if (p < 0) p += g_line_len;
    size_t b = (size_t)(p / kBin), m = g_line.size();
    for (size_t k = 0; k < m; ++k) {                       // the nearest bin the line went through
        if (!std::isnan(g_line[(b + k) % m])) return g_line[(b + k) % m];
        if (!std::isnan(g_line[(b + m - k) % m])) return g_line[(b + m - k) % m];
    }
    return 0;
}

// Handling mods (10-13, ideas from Scorcher/physics_compare.md), applied at the start of each car tick:
//   --game lean=1    steer through lean (Manx TT car_UpdateLeanBalance): LEFT/RIGHT are taken away from the
//                    game; they ramp an input r (leanramp per tick), shaped 0.3r + 0.7r^3 (Manx cubic shaping),
//                    which pulls a lean spring-damper (leank 300 /s^2, leand 0.51 kept per tick, dt = 1/30 s);
//                    the lean turns the heading by steeryaw degrees per tick at full lean, scaled down above yawv
//                    (speed-scaled clamp). The car's orientation bases (wram+0x40/+0x4C/+0x58, forward and side
//                    as 2.14 shorts) and its body matrix (wram+0x88, rows 2.30) are rotated about the car's up.
//   --game grip=G    Sonic R grip (car_UpdatePhysics keeps forward and lateral velocity apart): on the ground
//                    (wram+0x3C > 0) the velocity across the car loses G of itself per tick; gripkeep=K of
//                    what it loses goes into forward speed. Holding L or R uses driftgrip instead (a drift).
//   --game cspdrift=1 Cyber Speedway drift (car_UpdatePlayer MAP1_1 0x06020074, CyberSpeedway/NOTES.md "Movement
//                    physics"): on the ground the car-plane velocity is blended toward its own size along the nose,
//                    v = t*(256-g)/256 + v*g/256; g is the old-velocity weight from the grip table 0x0609f7b6
//                    (cspclass 0..2) indexed by a slide timer = max(timer - 2, slide angle in 1024ths / 2), capped
//                    0x1f -> 0x1c, index timer/2. Small slide: g 64..110 (snaps to the nose); big slide: g up to
//                    231..241 (keeps sliding = drift) until the timer runs down.
//   --game manx=1    Manx TT air pitch (bike_UpdateRacerPhysics 0x0602f7d0, ManxTT/HANDOFF-scorcher.md "Slope pitch and
//                    gravity"): on takeoff above kickv speed the target pitch is the takeoff pitch + kick degrees
//                    (Manx 227/4096 = 20 deg off crests); in the air pitch += pitchk 0.3 of the error per tick (about the
//                    car's side axis). airgrav=A keeps A of the game's gravity (+0x1200 on wram+0x10, y down) in the
//                    air; Manx itself has none (A = 0).
int g_lean, g_lean_sign = 1;
double g_lean_k = 300, g_lean_d = 0.51, g_lean_ramp = 0.15, g_yaw = 1.5, g_yaw_v = 60000;
double g_grip = -1, g_grip_keep = 0.5, g_drift_grip = 0.02;
int g_csp_drift, g_csp_class = 1;
double g_csp_slide;                              // Cyber Speedway slide timer (car +0x30)
double g_csp_scale = 1, g_csp_keep = 1;         // cspscale: slide angle factor; cspkeep: 1 = keep speed, 0 = only forward speed
int g_manx;
double g_kick = 20, g_kick_v = 20000, g_pitch_k = 0.3, g_air_grav = 1;
bool g_was_ground; double g_pitch_target; uint64_t g_kicks;
const int k_csp_grip[3][16] = {{110, 110, 110, 110, 110, 110, 120, 130, 140, 150, 160, 170, 180, 190, 222, 225},
                               {64, 90, 110, 120, 130, 140, 150, 160, 200, 220, 230, 230, 231, 231, 231, 231},
                               {64, 90, 110, 120, 130, 140, 150, 160, 200, 220, 230, 230, 241, 241, 241, 241}};
uint16_t g_steer_in, g_buttons_in;              // what the pad held before the lean mod took LEFT/RIGHT
double g_lean_r, g_lean_x, g_lean_w;            // ramped input, lean (-1..1 at rest), lean speed
uint64_t g_mod_ticks;

void read_vec16(uint32_t a, double v[3]) { for (int i = 0; i < 3; ++i) v[i] = (int16_t)ld16(a + 2 * i) / 16384.0; }
void write_vec16(uint32_t a, const double v[3]) {
    for (int i = 0; i < 3; ++i) st16(a + 2 * i, (uint16_t)(int16_t)std::lround(std::clamp(v[i], -1.99, 1.99) * 16384));
}
// v rotated by angle a about the unit axis k (Rodrigues)
void rotate(double v[3], const double k[3], double a) {
    const double c = std::cos(a), s = std::sin(a), d = k[0] * v[0] + k[1] * v[1] + k[2] * v[2];
    const double x[3] = {k[1] * v[2] - k[2] * v[1], k[2] * v[0] - k[0] * v[2], k[0] * v[1] - k[1] * v[0]};
    for (int i = 0; i < 3; ++i) v[i] = v[i] * c + x[i] * s + k[i] * d * (1 - c);
}

// rotates the car's orientation bases and body matrix by a about the unit axis k
void rotate_car(uint32_t wram, const double k[3], double a) {
    for (uint32_t base : {0x40u, 0x4Cu, 0x58u})
        for (uint32_t off : {0u, 6u}) {
            double v[3];
            read_vec16(wram + base + off, v);
            rotate(v, k, a);
            write_vec16(wram + base + off, v);
        }
    for (int r = 0; r < 3; ++r) {
        double v[3];
        for (int i = 0; i < 3; ++i) v[i] = (int32_t)ld32(wram + 0x88 + 12 * r + 4 * i) / 1073741824.0;
        rotate(v, k, a);
        for (int i = 0; i < 3; ++i)
            st32(wram + 0x88 + 12 * r + 4 * i, (uint32_t)(int32_t)std::lround(std::clamp(v[i], -1.99, 1.99) * 1073741824.0 * 0.999999));
    }
}
double pitch_of(const double f[3]) { return std::asin(std::clamp(-f[1], -1.0, 1.0)); }   // nose up > 0 (y is down)

void handling(uint32_t wram) {
    double f[3], s[3], up[3], vel[3];
    read_vec16(wram + 0x40, f);
    read_vec16(wram + 0x46, s);
    up[0] = f[1] * s[2] - f[2] * s[1]; up[1] = f[2] * s[0] - f[0] * s[2]; up[2] = f[0] * s[1] - f[1] * s[0];
    const double n = std::sqrt(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]);
    const double fn = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]), sn = std::sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    if (n < 0.5 || fn < 0.5 || sn < 0.5) return;                  // no orientation yet
    for (int i = 0; i < 3; ++i) { up[i] /= n; f[i] /= fn; s[i] /= sn; }
    for (int i = 0; i < 3; ++i) vel[i] = (int32_t)ld32(wram + 0x0C + 4 * i);
    const double vf = vel[0] * f[0] + vel[1] * f[1] + vel[2] * f[2];
    const double vs = vel[0] * s[0] + vel[1] * s[1] + vel[2] * s[2];
    const bool ground = (int32_t)ld32(wram + 0x3C) > 0;
    ++g_mod_ticks;

    if (g_lean) {
        const double u = (g_steer_in & 0x4000) ? -1 : (g_steer_in & 0x8000) ? 1 : 0;   // LEFT : RIGHT
        g_lean_r = u ? std::clamp(g_lean_r + u * g_lean_ramp, -1.0, 1.0) : g_lean_r * 0.5;
        const double target = 0.3 * g_lean_r + 0.7 * g_lean_r * g_lean_r * g_lean_r;
        const double dt = 1.0 / 30;
        g_lean_w = (g_lean_w + g_lean_k * dt * (target - g_lean_x)) * g_lean_d;
        g_lean_x = std::clamp(g_lean_x + g_lean_w * dt, -1.2, 1.2);
        const double speed = std::fabs(vf);
        const double a = g_lean_sign * g_lean_x * g_yaw * 3.14159265358979 / 180 * std::min(1.0, speed / 8000) / (1 + speed / g_yaw_v);
        if (std::fabs(a) > 1e-6) {
            for (uint32_t base : {0x40u, 0x4Cu, 0x58u})
                for (uint32_t off : {0u, 6u}) {
                    double v[3];
                    read_vec16(wram + base + off, v);
                    rotate(v, up, a);
                    write_vec16(wram + base + off, v);
                }
            for (int r = 0; r < 3; ++r) {
                double v[3];
                for (int i = 0; i < 3; ++i) v[i] = (int32_t)ld32(wram + 0x88 + 12 * r + 4 * i) / 1073741824.0;
                rotate(v, up, a);
                for (int i = 0; i < 3; ++i)
                    st32(wram + 0x88 + 12 * r + 4 * i, (uint32_t)(int32_t)std::lround(std::clamp(v[i], -1.99, 1.99) * 1073741824.0 * 0.999999));
            }
        }
    }

    if (g_grip >= 0 && ground) {
        const double g = (g_buttons_in & 0x0088) ? g_drift_grip : g_grip;   // L or R held: drift
        const double lost = vs * g;
        const double gain = std::fabs(lost) * g_grip_keep * (vf < 0 ? -1 : 1);
        for (int i = 0; i < 3; ++i) vel[i] += -lost * s[i] + gain * f[i];
        for (int i = 0; i < 3; ++i) st32(wram + 0x0C + 4 * i, (uint32_t)(int32_t)std::lround(vel[i]));
    }

    if (g_manx) {
        constexpr double deg = 3.14159265358979 / 180;
        const double p = pitch_of(f);
        if (!ground) {
            if (g_was_ground) {                                    // takeoff
                g_pitch_target = p;
                if (std::hypot(vf, vs) > g_kick_v) { g_pitch_target += g_kick * deg; ++g_kicks; }
            }
            const double err = std::clamp(g_pitch_target - p, -10 * deg, 10 * deg);
            if (std::fabs(err) > 1e-4) {
                double f2[3] = {f[0], f[1], f[2]};
                rotate(f2, s, 0.01);
                const double sg = pitch_of(f2) > p ? 1 : -1;       // which way about s raises the nose
                rotate_car(wram, s, sg * g_pitch_k * err);
            }
            if (g_air_grav < 1) {
                const int32_t vy = (int32_t)ld32(wram + 0x10);
                st32(wram + 0x10, (uint32_t)(int32_t)(vy - std::lround((1 - g_air_grav) * 0x1200)));
            }
        }
        g_was_ground = ground;
    }

    if (g_csp_drift && ground) {
        const double vf2 = vel[0] * f[0] + vel[1] * f[1] + vel[2] * f[2], vs2 = vel[0] * s[0] + vel[1] * s[1] + vel[2] * s[2];
        const double sp = std::hypot(vf2, vs2);
        if (sp > 500) {
            const double slide = g_csp_scale * std::atan2(std::fabs(vs2), std::fabs(vf2)) * 1024 / (2 * 3.14159265358979);   // 1024ths
            g_csp_slide = std::max(g_csp_slide - 2, slide / 2);
            if (g_csp_slide > 0x1f) g_csp_slide = 0x1c;
            const double g = k_csp_grip[g_csp_class][std::clamp((int)(g_csp_slide / 2), 0, 15)] / 256.0;
            const double tm = std::fabs(vf2) + g_csp_keep * (sp - std::fabs(vf2));
            const double tf = vf2 < 0 ? -tm : tm;                  // along the nose (or the tail reversing)
            const double nf = tf * (1 - g) + vf2 * g, ns = vs2 * g;
            for (int i = 0; i < 3; ++i) vel[i] += (nf - vf2) * f[i] + (ns - vs2) * s[i];
            for (int i = 0; i < 3; ++i) st32(wram + 0x0C + 4 * i, (uint32_t)(int32_t)std::lround(vel[i]));
        }
    }
}

// --game mxcam=1|2: the Manx TT Super Bike cameras (ManxTT study_chasecam.md): 1 = near chase (state 3), 2 = first person (state 2).
// Manx units: y down, offset (0,-4,-26) from the pivot (0,-12,+7) in the bike frame (yaw/pitch only), the camera yaw follows the
// bike's by 1/8 per tick, its pitch by 1/18.048 per tick, pull-back = speed/8 behind. mxscale = Scorcher units per Manx unit,
// mxpull scales the speed pull-back (Manx speed unit unknown), mxeye = first-person eye height above the car point (Manx units),
// mxcamview=Z toggles 1 <-> 2 in hand play (Z = 0x0010, rising edge). mxcam=0 (default) leaves the stock camera alone.
int g_mxcam;
double g_mxscale = 5796, g_mxpull = 1, g_mxeye = 8, g_mxoy = 0, g_mxfwd = 3, g_mxtilt = 0;
bool g_mxtoggle; uint32_t g_mx_world; bool g_mx_have; uint64_t g_mx_ticks;
double g_mx_pos[3], g_mx_f[3], g_mx_d[3], g_mx_speed;     // the bike: position, nose, down (unit), forward speed
double g_mx_yaw, g_mx_pitch, g_mx_cyaw, g_mx_cpitch;      // the bike's heading and nose-up pitch, and the camera's smoothed ones
void mx_track(uint32_t world, uint32_t wram) {
    if (!g_mx_world) g_mx_world = world;                  // the first car seen = the player
    if (world != g_mx_world) return;
    double f[3], s[3];
    read_vec16(wram + 0x40, f);
    read_vec16(wram + 0x46, s);
    double d[3] = {f[1] * s[2] - f[2] * s[1], f[2] * s[0] - f[0] * s[2], f[0] * s[1] - f[1] * s[0]};
    const double dn = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]), fn = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    if (dn < 0.5 || fn < 0.5) return;
    for (int i = 0; i < 3; ++i) { d[i] /= dn; f[i] /= fn; }
    if (d[1] < 0) for (double& x : d) x = -x;             // down = +y (Saturn y down); the sign of f x s is not known
    for (int i = 0; i < 3; ++i) {
        g_mx_pos[i] = (int32_t)ld32(world + 0x20 + 4 * i);
        g_mx_f[i] = f[i]; g_mx_d[i] = d[i];
    }
    g_mx_speed = 0;
    for (int i = 0; i < 3; ++i) g_mx_speed += (int32_t)ld32(wram + 0x0C + 4 * i) * f[i];
    g_mx_yaw = std::atan2(f[2], f[0]);
    g_mx_pitch = std::asin(std::clamp(-f[1], -1.0, 1.0));
    if (!g_mx_have) { g_mx_have = true; g_mx_cyaw = g_mx_yaw; g_mx_cpitch = g_mx_pitch; }
    double e = g_mx_cyaw - g_mx_yaw;                      // yaw lag 1/8, pitch lag 1/18.048 per tick (Manx 0x0602d2c0)
    e -= 2 * 3.14159265358979 * std::round(e / (2 * 3.14159265358979));
    g_mx_cyaw -= e / 8;
    g_mx_cpitch += (g_mx_pitch - g_mx_cpitch) / 18.048;
    ++g_mx_ticks;
}

void on_car(SH2Context& c, uint32_t) {
    uint32_t world = c.r[4], wram = c.r[5];
    if (!SH2_IS_WRAM_H(world) || !SH2_IS_WRAM_H(wram)) return;
    if (g_lean || g_grip >= 0 || g_csp_drift || g_manx) handling(wram);
    g_car_vb = sat_vblanks();
    if (g_mxcam) mx_track(world, wram);
    uint32_t rec = ld32(world + 0x98);
    if (!SH2_IS_WRAM_H(rec)) return;
    uint32_t trk = ld32(rec + 0x24);
    if (!SH2_IS_WRAM_H(trk)) return;
    ++g_ticks;
    int32_t pos[3], vel[3], spl[3];
    double d[3], v[3], m[9], t[3], tv[3];
    for (int i = 0; i < 3; ++i) {
        pos[i] = (int32_t)ld32(world + 0x20 + 4 * i);
        vel[i] = (int32_t)ld32(wram + 0x0C + 4 * i);
        spl[i] = (int32_t)ld32(trk + 0x08 + 4 * i);
        d[i] = (double)pos[i] - spl[i];
        v[i] = vel[i];
    }
    for (int i = 0; i < 9; ++i) m[i] = (int32_t)ld32(trk + 0x50 + 4 * i) / 1073741824.0;
    to_basis(m, d, t);
    to_basis(m, v, tv);
    int32_t progress = (int32_t)ld32(trk + 0x1C);

    if (sat_vblanks() >= g_drive_from) {
        double along = std::fabs(tv[0]) > 1 ? std::fabs(tv[0]) : 1;
        double target = line_at(progress + g_look * along);
        double s = g_sign * (g_kp * (t[1] - target) / 30000.0 + g_kd * tv[1] / along);
        g_steer = s > g_dead ? 0x4000 : s < -g_dead ? 0x8000 : 0;   // LEFT : RIGHT
        if (!g_explore) g_throttle = g_accel;
        if (!g_explore && !g_plan_cap.empty()) {                   // the speed cap a little ahead
            double cap = std::min(g_plan_cap[plan_bin(progress)], g_plan_cap[plan_bin(progress + g_look * along)]);
            g_throttle = tv[0] < cap ? g_accel : tv[0] > 1.15 * cap ? 0x2000 : 0;   // B, DOWN, coast
        }
        else {
            if (--g_spell <= 0) {                                   // a new spell of 3..12 ticks
                static const uint16_t th[] = {0x0100, 0x0100, 0x0100, 0, 0x0400, 0x2000, 0x0200};
                g_throttle = th[rnd() % 7];                         // B x3, none, A, DOWN, C
                g_steer_mode = rnd() % 11;  // 0-2 the controller, 3 LEFT, 4 RIGHT, 5 none, 6-10 L R X Y Z (+ the controller)
                g_spell = 3 + rnd() % 10;
            }
            static const uint16_t extra[] = {0x0008, 0x0080, 0x0040, 0x0020, 0x0010};   // L R X Y Z
            if (g_steer_mode >= 6) g_steer |= extra[g_steer_mode - 6];
            else if (g_steer_mode >= 3) g_steer = g_steer_mode == 3 ? 0x4000 : g_steer_mode == 4 ? 0x8000 : 0;
        }
    }
    if (g_state) {
        std::fprintf(g_state, "%llu,%llu,%04X,%d,%d,%d,%d,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%08X,%d,%d,%d",
                     (unsigned long long)g_ticks, (unsigned long long)sat_vblanks(), g_pad_seen,
                     pos[0], pos[1], pos[2], vel[0], vel[1], vel[2], progress,
                     t[0], t[1], t[2], tv[0], tv[1], tv[2], world, spl[0], spl[1], spl[2]);
        for (int i = 0; i < 9; ++i) std::fprintf(g_state, ",%d", (int32_t)ld32(trk + 0x50 + 4 * i));
        std::fprintf(g_state, ",%d,%d,%d,%d", (int32_t)ld32(wram + 0x20), (int16_t)ld16(wram + 0x24),
                     (int16_t)ld16(wram + 0x26), (int32_t)ld32(wram + 0x3C));
        for (int i = 0; i < 12; ++i) std::fprintf(g_state, ",%d", (int16_t)ld16(wram + 0x40 + 2 * i));
        for (int i = 0; i < 9; ++i) std::fprintf(g_state, ",%d", (int32_t)ld32(wram + 0x88 + 4 * i));
        std::fputc('\n', g_state);
    }
}

int g_fly;                      // --game fly: 1 = whole track, 2 = dump the camera
double g_far = 1;
double g_fog = -1;               // --game fog=K: the depth shade ramp stretched K times (0 = no fog)
int g_all;                      // --game all=1: the LOD window opened to the whole ring
uint64_t g_fly_dump_at;

uint64_t g_ncam, g_ncol, g_ncam_dumped;
void fly_camera(uint32_t cam);
void mx_camera(uint32_t cam);
extern uint64_t g_fly_from;
void on_camera(SH2Context&, uint32_t) {
    ++g_ncam;
    const uint32_t cam = ld32(0x060FCBA8);
    if (g_fly == 1 && sat_vblanks() >= g_fly_from) fly_camera(cam);
    if (g_mxcam && g_mx_have) mx_camera(cam);
    if (g_fly == 2 && sat_vblanks() > 4100 && !g_ncam_dumped++) {
        const uint32_t t = ld32(0x060FC7C8);
        std::fprintf(stderr, "shade table %08X:", t);
        for (int i = 0; i < 16384; i += 64) std::fprintf(stderr, " %d:%02X", i, ld8(t + i));
        std::fprintf(stderr, "\n");
    }
    if (g_fly == 2 && sat_vblanks() >= g_fly_dump_at) {
        g_fly_dump_at = sat_vblanks() + 60;
        std::fprintf(stderr, "cam vb %llu eye %d %d %d m", (unsigned long long)sat_vblanks(),
                     (int32_t)ld32(cam + 4), (int32_t)ld32(cam + 8), (int32_t)ld32(cam + 12));
        for (int i = 0; i < 9; ++i) std::fprintf(stderr, " %d", (int32_t)ld32(cam + 0x4C + 4 * i));
        std::fprintf(stderr, " | 70:");
        for (int i = 0; i < 8; ++i) std::fprintf(stderr, " %d", (int32_t)ld32(cam + 0x70 + 4 * i));
        std::fprintf(stderr, " | 28: %d 2c: %d 30: %d\n", (int32_t)ld32(cam + 0x28), (int32_t)ld32(cam + 0x2C), (int32_t)ld32(cam + 0x30));
    }
}

void on_collect(SH2Context&, uint32_t) {
    ++g_ncol;
    const uint32_t cam = ld32(0x060FCBA8);
    if (g_all) { st32(0x0600DE88, 0x80000000u); st32(0x0600DE8C, 0x7FFFFFFFu); }
    if (g_fog >= 0 && g_fly == 1 && sat_vblanks() >= g_fly_from) {   // the depth shade table *060FC7C8: 2048 bytes, 0 near, ramp to 0x7F (dark) at ~620..1024
        static uint8_t orig[2048];
        static uint32_t at;
        const uint32_t t = ld32(0x060FC7C8);
        if (t && t != at) { at = t; for (int i = 0; i < 2048; ++i) orig[i] = ld8(t + i); }
        if (t)
            for (int i = 0; i < 2048; ++i)
                st8(t + i, g_fog == 0 ? 0 : orig[std::min(2047, (int)(i / g_fog))]);
    }
    if (g_far != 1) {                   // +0x70..+0x7C (3200000 x2, 4160000 x2 on The Dump), rewritten by 060F6BC0
        static uint32_t base[4];
        for (int i = 0; i < 4; ++i) {
            const uint32_t f = ld32(cam + 0x70 + 4 * i);
            if (f != (uint32_t)(int64_t)(base[i] * g_far)) base[i] = f;
            st32(cam + 0x70 + 4 * i, (uint32_t)(int64_t)(base[i] * g_far));
        }
    }
}

// the flyover camera: an orbit around (cx, cz) at height h above y = cy (world y points down), radius r,
// looking at the centre; yaw = yaw0 + spin * VBlanks (degrees)
double g_cx = -14006940, g_cy = -574914, g_cz = -12444444, g_h = 20e6, g_r = 12e6, g_yaw0 = 0, g_spin = 0;
uint64_t g_fly_from = 0;

void fly_camera(uint32_t cam) {
    const double a = (g_yaw0 + g_spin * (double)(sat_vblanks() - g_fly_from)) * 3.14159265358979 / 180;
    const double e[3] = {g_cx + g_r * std::cos(a), g_cy - g_h, g_cz + g_r * std::sin(a)};
    double f[3] = {g_cx - e[0], g_cy - e[1], g_cz - e[2]};
    double n = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    for (double& v : f) v /= n;
    if (std::fabs(f[1]) > 0.9999) { f[2] += 0.01; n = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]); for (double& v : f) v /= n; }
    double u[3] = {-f[1] * f[0], 1 - f[1] * f[1], -f[1] * f[2]};   // world down minus its part along f
    n = std::sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
    for (double& v : u) v /= n;
    const double s[3] = {u[1] * f[2] - u[2] * f[1], u[2] * f[0] - u[0] * f[2], u[0] * f[1] - u[1] * f[0]};
    const double* rows[3] = {f, u, s};   // row 0 = view direction (z), row 1 = screen down, row 2 = row1 x row0
    for (int i = 0; i < 3; ++i) st32(cam + 4 + 4 * i, (uint32_t)(int32_t)e[i]);
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c) {
            const uint32_t v = (uint32_t)(int32_t)std::lround(rows[r][c] * 1073741824.0 * 0.999999);
            st32(cam + 0x4C + 4 * (3 * r + c), v);
            st32(cam + 0xF8 + 4 * (3 * r + c), v);
        }
}

// writes the eye e and the view matrix (rows: view direction f, screen down u, u x f) like fly_camera
void set_camera(uint32_t cam, const double e[3], const double f[3], const double u[3]) {
    const double s[3] = {u[1] * f[2] - u[2] * f[1], u[2] * f[0] - u[0] * f[2], u[0] * f[1] - u[1] * f[0]};
    const double* rows[3] = {f, u, s};
    for (int i = 0; i < 3; ++i) st32(cam + 4 + 4 * i, (uint32_t)(int32_t)e[i]);
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c) {
            const uint32_t v = (uint32_t)(int32_t)std::lround(rows[r][c] * 1073741824.0 * 0.999999);
            st32(cam + 0x4C + 4 * (3 * r + c), v);
            st32(cam + 0xF8 + 4 * (3 * r + c), v);
        }
}
// yaw/pitch frame: nose f, down d, right r = d x f
void yp_frame(double yaw, double pitch, double f[3], double d[3], double r[3]) {
    const double cp = std::cos(pitch), sp = std::sin(pitch), cy = std::cos(yaw), sy = std::sin(yaw);
    f[0] = cp * cy; f[1] = -sp; f[2] = cp * sy;
    d[0] = sp * cy; d[1] = cp;  d[2] = sp * sy;
    r[0] = d[1] * f[2] - d[2] * f[1]; r[1] = d[2] * f[0] - d[0] * f[2]; r[2] = d[0] * f[1] - d[1] * f[0];
}
void mx_camera(uint32_t cam) {
    const double K = g_mxscale;
    static uint64_t dn_; if (g_fly == 2 && ++dn_ % 60 == 0)   // fly=dump: the stock camera (before it is replaced) next to the bike
        std::fprintf(stderr, "mxcam vb %llu stock eye %d %d %d f %d %d %d u %d %d %d | bike pos %.0f %.0f %.0f f %.3f %.3f %.3f d %.3f %.3f %.3f speed %.0f\n",
                     (unsigned long long)sat_vblanks(), (int)ld32(cam + 4), (int)ld32(cam + 8), (int)ld32(cam + 12),
                     (int)ld32(cam + 0x4C) >> 20, (int)ld32(cam + 0x50) >> 20, (int)ld32(cam + 0x54) >> 20,
                     (int)ld32(cam + 0x58) >> 20, (int)ld32(cam + 0x5C) >> 20, (int)ld32(cam + 0x60) >> 20,
                     g_mx_pos[0], g_mx_pos[1], g_mx_pos[2], g_mx_f[0], g_mx_f[1], g_mx_f[2], g_mx_d[0], g_mx_d[1], g_mx_d[2], g_mx_speed);
    double e[3];
    if (g_mxcam == 2) {                                   // first person: the car point, the bike's whole frame (rolls with it)
        const double* f = g_mx_f; const double* d = g_mx_d;
        for (int i = 0; i < 3; ++i) e[i] = g_mx_pos[i] - d[i] * g_mxeye * K + f[i] * g_mxfwd * K;
        set_camera(cam, e, f, d);
        return;
    }
    double f[3], d[3], r[3], fc[3], dc[3], rc[3];
    yp_frame(g_mx_yaw, g_mx_pitch, f, d, r);              // the bike's frame (pivot), no roll
    yp_frame(g_mx_cyaw, g_mx_cpitch, fc, dc, rc);         // the camera's lagged frame
    const double pull = std::max(0.0, g_mx_speed) / K / 8 * g_mxpull;   // Manx units behind, along the camera heading
    const double hz[3] = {std::cos(g_mx_cyaw), 0, std::sin(g_mx_cyaw)};
    for (int i = 0; i < 3; ++i)
        e[i] = g_mx_pos[i] + K * (-12.0 * d[i] + 7.0 * f[i]) - K * pull * hz[i] + K * (-4.0 * dc[i] - 26.0 * fc[i]) + 0 * rc[i];
    e[1] += g_mxoy * K;
    if (g_mxtilt != 0) yp_frame(g_mx_cyaw, g_mx_cpitch - g_mxtilt * 3.14159265358979 / 180, fc, dc, rc);   // aim only
    set_camera(cam, e, fc, dc);
}

// Menu keys (10-13): the Saturn menus select with C (key 3) and go back with B (key 2); START only
// passes "press start" (titlemain flag 060FBBF0 == 1). Outside a race Enter (START) becomes C and
// Esc (host bit 0x0004) becomes B + START; in a race Esc is START (pause) and Enter is dead. An Enter held on from "press start" or a race stays dead until let go.

uint16_t menu_keys(uint16_t pressed) {
    static bool held_over;
    const bool race = g_car_vb != ~0ull && sat_vblanks() - g_car_vb < 30;
    const bool esc = pressed & 0x0004;
    pressed &= (uint16_t)~0x0004;
    if (!(pressed & 0x0800)) held_over = false;
    if (race) {                       // Esc pauses; Enter does nothing in a race
        if (pressed & 0x0800) held_over = true;
        pressed &= (uint16_t)~0x0800;
        if (esc) pressed |= 0x0800;
        return pressed;
    }
    const bool press_start = ld32(0x060FBBF0) == 1;
    if (pressed & 0x0800) {
        if (press_start) held_over = true;
        else { pressed &= (uint16_t)~0x0800; if (!held_over) pressed |= 0x0200; }
    }
    if (esc && !press_start) pressed |= 0x0900;   // B and START: the pause and options pages leave on START
    return pressed;
}

uint16_t pad(uint16_t pressed) {
    pressed = menu_keys(pressed);
    if (sat_vblanks() >= g_drive_from && !(pressed & 0x0800))      // START passes through alone
        pressed = (uint16_t)((pressed & ~0xC000) | g_throttle | g_steer);
    g_buttons_in = pressed;
    if (g_mxtoggle) { static bool z; const bool zn = pressed & 0x0010; if (zn && !z) g_mxcam = 3 - g_mxcam; z = zn; }
    if (g_lean) { g_steer_in = pressed & 0xC000; pressed &= (uint16_t)~0xC000; }   // the lean steers, not the game
    g_pad_seen = g_buttons_in;   // logged as held, before the lean mod takes LEFT/RIGHT
    return pressed;
}

// vdp1_BuildCmdList's entry (LORT:0600CE6C): the 0x2C-byte draw records it turns into VDP1
// commands, record i -> the command at *(*0600D088) + i * 32. --game recdump=VB prints them for a
// few frames from VBlank VB (to find what stays the same for one polygon across frames).
// With --interp the layer hands the draw keys (vdp1_next_draw_keys): a polygon's record address and
// its +0x18 word stay the same from frame to frame (run105: 218 of 220 found in the frame before),
// while its texture address does not (the depth shade step, 120+ of 220 change each frame).
uint64_t g_recdump = ~0ull;
bool g_interp;
void on_cmdlist(SH2Context&, uint32_t) {
    const uint32_t blk = ld32(ld32(0x0600D080)), base = ld32(ld32(0x0600D088));
    if (!blk) return;
    const uint32_t lo = ld32(blk + 0x2C), hi = ld32(blk + 0x30);
    if (g_interp && hi > lo) {
        const uint32_t cap = ld32(0x0600D074), first = (base & 0x7FFFF) / 32;
        const uint32_t n = std::min((hi - lo) / 4, cap);
        std::vector<uint64_t> keys(first + n, 0);
        for (uint32_t i = 0; i < n; ++i) {
            const uint32_t r = ld32(lo + 4 * i);
            keys[first + i] = (uint64_t)r << 32 | ld32(r + 0x18);
        }
        vdp1_next_draw_keys(std::move(keys));
    }
    const uint64_t vb = sat_vblanks();
    if (vb < g_recdump || vb > g_recdump + 12) return;
    std::fprintf(stderr, "recdump vb %llu: %u records, cmds at %08X\n", (unsigned long long)vb, (hi - lo) / 4, base);
    for (uint32_t p = lo, i = 0; p < hi && i < 400; p += 4, ++i) {
        const uint32_t r = ld32(p);
        std::fprintf(stderr, " %3u %08X  %04X %04X %04X  %08X %08X  tex %04X %04X  v %08X %08X\n", i, r, ld16(r), ld16(r + 2),
                     ld16(r + 4), ld32(r + 8), ld32(r + 0x18), ld16(r + 0x10), ld16(r + 0x12), ld32(r + 0x1C), ld32(r + 0x24));
    }
}

// --game manxbike=1 [manxdir=DIR]: the converted Manx bike (tools/manx2ting.py) replaces world "ting".
// ting.wl3 is copied over the loaded file right after file_LoadTagged in world_SlotByName (hook
// 060F3650: r8 = path, @(4,r14) = the file); TING.0 is served by cdrom_override_add.
std::vector<uint8_t> g_manx_wl3;
unsigned g_manx_hits = 0;
void on_slot_loaded(SH2Context& c, uint32_t) {
    std::string path;
    for (uint32_t a = c.r[8]; path.size() < 200 && ld8(a); ++a) path += (char)std::tolower((int)ld8(a));
    size_t n = path.size();
    if (n < 8 || path.compare(n - 8, 8, "ting.wl3") != 0) return;
    uint32_t file = ld32(c.r[14] + 4);
    for (size_t i = 0; i < g_manx_wl3.size(); ++i) st8(file + (uint32_t)i, g_manx_wl3[i]);
    ++g_manx_hits;
    std::fprintf(stderr, "manxbike: %s at %08X replaced by %zu bytes\n", path.c_str(), file, g_manx_wl3.size());
}

void init(const SaturnConfig& cfg) {
    g_interp = cfg.interp && sat_game_option("keys", "") == "1";   // off: the record address is a pool slot, not a polygon (run106)
    std::string f = sat_game_option("state", "");
    if (!f.empty()) {
        if (!(g_state = std::fopen(f.c_str(), "w"))) sat_fatal("--game state: cannot write %s", f.c_str());
        std::fprintf(g_state, "tick,vblank,pad,x,y,z,vx,vy,vz,progress,along,lat,up,valong,vlat,vup,"
                              "world,sx,sy,sz,m0,m1,m2,m3,m4,m5,m6,m7,m8,speed,st0,st1,gnd,"
                              "a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,b0,b1,b2,b3,b4,b5,b6,b7,b8\n");
    }
    std::string v = sat_game_option("line", "");
    if (!v.empty()) load_line(v);
    v = sat_game_option("plan", ""); if (!v.empty()) load_plan(v);
    v = sat_game_option("drive", ""); if (!v.empty()) g_drive_from = std::strtoull(v.c_str(), nullptr, 10);
    v = sat_game_option("accel", ""); if (!v.empty()) g_accel = button(v);
    v = sat_game_option("kp", "");    if (!v.empty()) g_kp = std::atof(v.c_str());
    v = sat_game_option("kd", "");    if (!v.empty()) g_kd = std::atof(v.c_str());
    v = sat_game_option("dead", "");  if (!v.empty()) g_dead = std::atof(v.c_str());
    v = sat_game_option("look", "");  if (!v.empty()) g_look = std::atof(v.c_str());
    v = sat_game_option("explore", ""); if (!v.empty()) g_explore = (uint32_t)std::strtoul(v.c_str(), nullptr, 10) | 1;
    v = sat_game_option("sign", "");  if (!v.empty()) g_sign = std::atoi(v.c_str());
    v = sat_game_option("fly", "");   if (!v.empty()) g_fly = v == "dump" ? 2 : std::atoi(v.c_str());
    v = sat_game_option("all", "");   if (!v.empty()) g_all = std::atoi(v.c_str());
    v = sat_game_option("fog", "");   if (!v.empty()) g_fog = std::atof(v.c_str());
    v = sat_game_option("far", "");   if (!v.empty()) g_far = std::atof(v.c_str());
    struct { const char* k; double* p; } fo[] = {{"cx", &g_cx}, {"cy", &g_cy}, {"cz", &g_cz}, {"h", &g_h}, {"r", &g_r},
                                                  {"yaw", &g_yaw0}, {"spin", &g_spin}};
    for (auto& o : fo) { v = sat_game_option(o.k, ""); if (!v.empty()) *o.p = std::atof(v.c_str()); }
    struct { const char* k; double* p; } ho[] = {{"leank", &g_lean_k}, {"leand", &g_lean_d}, {"leanramp", &g_lean_ramp},
                                                  {"steeryaw", &g_yaw}, {"yawv", &g_yaw_v}, {"grip", &g_grip},
                                                  {"gripkeep", &g_grip_keep}, {"driftgrip", &g_drift_grip},
                                                  {"cspscale", &g_csp_scale}, {"cspkeep", &g_csp_keep},
                                                  {"kick", &g_kick}, {"kickv", &g_kick_v}, {"pitchk", &g_pitch_k}, {"airgrav", &g_air_grav}};
    for (auto& o : ho) { v = sat_game_option(o.k, ""); if (!v.empty()) *o.p = std::atof(v.c_str()); }
    v = sat_game_option("lean", ""); if (!v.empty()) g_lean = std::atoi(v.c_str());
    v = sat_game_option("leansign", ""); if (!v.empty()) g_lean_sign = std::atoi(v.c_str());
    v = sat_game_option("manx", ""); if (!v.empty()) g_manx = std::atoi(v.c_str());
    v = sat_game_option("cspdrift", ""); if (!v.empty()) g_csp_drift = std::atoi(v.c_str());
    v = sat_game_option("cspclass", ""); if (!v.empty()) g_csp_class = std::clamp(std::atoi(v.c_str()), 0, 2);
    if (g_lean || g_grip >= 0 || g_csp_drift || g_manx)
        std::atexit([] { std::fprintf(stderr, "handling: lean %d grip %.3f, %llu car ticks\n", g_lean, g_grip, (unsigned long long)g_mod_ticks); });
    v = sat_game_option("flyfrom", ""); if (!v.empty()) g_fly_from = std::strtoull(v.c_str(), nullptr, 10);
    if (sat_game_option("manxbike", "0") == "1") {
        std::string dir = sat_game_option("manxdir", "C:/Users/john/Documents/Scorcher/extracted/bike/manx");
        std::ifstream f(dir + "/ting.wl3", std::ios::binary);
        g_manx_wl3.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        if (g_manx_wl3.empty() || !cdrom_override_add("TING.0", (dir + "/TING.0").c_str()))
            std::fprintf(stderr, "manxbike: cannot read %s/ting.wl3 or TING.0\n", dir.c_str());
        else
            sh2_hook_add(0x060F3650, on_slot_loaded);
    }
    v = sat_game_option("mxcam", ""); if (!v.empty()) g_mxcam = std::clamp(std::atoi(v.c_str()), 0, 2);
    g_mxtoggle = g_mxcam && sat_game_option("mxcamview", "") == "Z";
    for (auto& o : std::initializer_list<std::pair<const char*, double*>>{{"mxcamscale", &g_mxscale}, {"mxpull", &g_mxpull}, {"mxeye", &g_mxeye}, {"mxfwd", &g_mxfwd}, {"mxtilt", &g_mxtilt}, {"mxoy", &g_mxoy}})
        { v = sat_game_option(o.first, ""); if (!v.empty()) *o.second = std::atof(v.c_str()); }
    sh2_hook_add(0x060DAD38, on_car);
    sh2_hook_add(0x060F6F4E, on_camera);
    if (g_fly) std::atexit([] { std::fprintf(stderr, "fly: camera hook %llu, collect hook %llu, fly %d\n", (unsigned long long)g_ncam, (unsigned long long)g_ncol, g_fly); });
    sh2_hook_add(0x0600DF16, on_collect);
    sh2_hook_add(0x06005F9A, [](SH2Context&, uint32_t) { slave_yield(); });   // the slave's job ring is empty
    v = sat_game_option("recdump", ""); if (!v.empty()) g_recdump = std::strtoull(v.c_str(), nullptr, 10);
    sh2_hook_add(0x0600CE6C, on_cmdlist);
    smpc_pad_filter(pad);
}

struct Register { Register() { sat_add_game_init(init); } } g_register;

}  // namespace
