// Scorcher's game layer for saturn.exe: the player car's state each game
// tick to a CSV (a dataset for a taught driver), and a driver that holds
// accelerate and steers by the pad toward a recorded racing line.
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
//   --game sign=1            -1 flips LEFT/RIGHT
#include "saturn.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

FILE* g_state;
uint64_t g_ticks;
uint16_t g_pad_seen;            // the buttons the program read last (after the driver)
uint64_t g_drive_from = ~0ull;
uint16_t g_accel = 0x0100;      // B
uint16_t g_steer;               // LEFT/RIGHT the driver holds now
double g_kp = 1.0, g_kd = 2.0, g_dead = 0.05, g_look = 6;
int g_sign = 1;

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

double line_at(double progress) {
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

void on_car(SH2Context& c, uint32_t) {
    uint32_t world = c.r[4], wram = c.r[5];
    if (!SH2_IS_WRAM_H(world) || !SH2_IS_WRAM_H(wram)) return;
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
    }
    if (g_state) {
        std::fprintf(g_state, "%llu,%llu,%04X,%d,%d,%d,%d,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%08X,%d,%d,%d",
                     (unsigned long long)g_ticks, (unsigned long long)sat_vblanks(), g_pad_seen,
                     pos[0], pos[1], pos[2], vel[0], vel[1], vel[2], progress,
                     t[0], t[1], t[2], tv[0], tv[1], tv[2], world, spl[0], spl[1], spl[2]);
        for (int i = 0; i < 9; ++i) std::fprintf(g_state, ",%d", (int32_t)ld32(trk + 0x50 + 4 * i));
        std::fputc('\n', g_state);
    }
}

uint16_t pad(uint16_t pressed) {
    if (sat_vblanks() >= g_drive_from && !(pressed & 0x0800))      // START passes through alone
        pressed = (uint16_t)((pressed & ~0xC000) | g_accel | g_steer);
    g_pad_seen = pressed;
    return pressed;
}

void init(const SaturnConfig&) {
    std::string f = sat_game_option("state", "");
    if (!f.empty()) {
        if (!(g_state = std::fopen(f.c_str(), "w"))) sat_fatal("--game state: cannot write %s", f.c_str());
        std::fprintf(g_state, "tick,vblank,pad,x,y,z,vx,vy,vz,progress,along,lat,up,valong,vlat,vup,"
                              "world,sx,sy,sz,m0,m1,m2,m3,m4,m5,m6,m7,m8\n");
    }
    std::string v = sat_game_option("line", "");
    if (!v.empty()) load_line(v);
    v = sat_game_option("drive", ""); if (!v.empty()) g_drive_from = std::strtoull(v.c_str(), nullptr, 10);
    v = sat_game_option("accel", ""); if (!v.empty()) g_accel = button(v);
    v = sat_game_option("kp", "");    if (!v.empty()) g_kp = std::atof(v.c_str());
    v = sat_game_option("kd", "");    if (!v.empty()) g_kd = std::atof(v.c_str());
    v = sat_game_option("dead", "");  if (!v.empty()) g_dead = std::atof(v.c_str());
    v = sat_game_option("look", "");  if (!v.empty()) g_look = std::atof(v.c_str());
    v = sat_game_option("sign", "");  if (!v.empty()) g_sign = std::atoi(v.c_str());
    sh2_hook_add(0x060DAD38, on_car);
    smpc_pad_filter(pad);
}

struct Register { Register() { sat_add_game_init(init); } } g_register;

}  // namespace
