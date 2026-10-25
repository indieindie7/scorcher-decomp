# judge.py: LLM playtest loop for bikesim assists.
#   py -3.13 judge.py [--rounds 8] [--dry]
# Each round: playtest.exe (4 headless bots) -> compact report -> local Qwen scores the feel checklist and
# proposes ONE assist change -> rerun -> keep the change only if the hard metrics do not get worse and the
# LLM score does not drop. Physics parameters are never touched (physics is sacred, assists are tunable).
# --dry replaces the LLM with a random proposer to test the loop without the GPU.
# Log: out\judge_log.md, best assists: out\judge_best.json.
import json, os, random, re, subprocess, sys

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out")

# assist knobs the judge may change: (min, max)
RANGES = {
    "drift_grip": (0.1, 0.6), "drift_spin": (4, 20), "hold_floor": (0.2, 0.8), "hold_gain": (1, 6),
    "hold_steer": (0.5, 2.5), "catch_gain": (4, 20), "drift_max_deg": (25, 45), "spin_angle_deg": (45, 70),
    "stability": (0, 1), "stab_angle_deg": (2, 10), "speed_keep": (0, 1), "tc": (0, 1), "abs": (0, 1),
}
KEYS = ["lap_times_s", "off_track_count", "respawns", "spins", "drifts", "longest_drift_s", "avg_drift_deg",
        "max_drift_deg", "avg_speed_lost_per_drift_kmh", "lean_wobble_per_min", "spin_catch_active_s"]

CHECKLIST = """Feel checklist (arcade bike racer where riders can choose to drift; physics stays realistic):
1 racer_clean: the clean racing bot (no drift requests) should almost never slide or leave the track.
2 easy_entry: the drifter bot (rear brake + stick into corners) gets a drift in most tight corners, 20-35 deg.
3 no_snap: no spins, and drifts end smoothly (spin catch rarely needed).
4 fair_cost: a drift costs some speed (5-15 km/h) but is not a disaster; laps not much slower than the racer.
5 no_wobble: low lean wobble per minute for all bots.
6 survivable: the masher (full throttle, bang-bang steering) and the novice (late, noisy) still finish without endless respawns."""

SYSTEM = ("You are a vehicle-handling playtester for a motorcycle racing game. You read telemetry from bot riders "
          "and judge the feel. Reply with ONE JSON object only, no prose outside it.")


def run(params):
    args = [os.path.join(HERE, "playtest.exe")] + [f"{k}={v:g}" for k, v in params.items()]
    subprocess.run(args, cwd=HERE, check=True, capture_output=True,
                   creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    return json.load(open(os.path.join(OUT, "playtest.json")))


def report(d):
    lines = [f"track {d['track']['length_m']} m, {d['track']['laps']} laps"]
    for b in ("racer", "drifter", "masher", "novice"):
        lines.append(b + ": " + ", ".join(f"{k}={d[b][k]}" for k in KEYS))
    return "\n".join(lines)


def hard(d):
    """Hard penalty, lower is better. The LLM never sees or sets this."""
    r, dr, m, n = d["racer"], d["drifter"], d["masher"], d["novice"]
    h = 3 * r["respawns"] + r["off_track_count"] + r["drifts"] + r["lean_wobble_per_min"] / 10
    h += 5 * sum(d[b]["spins"] for b in ("racer", "drifter", "masher", "novice"))
    h += 0.5 * (m["respawns"] + n["respawns"])
    h += 2 * max(0, 12 - dr["drifts"]) + 0.5 * abs(dr["avg_drift_deg"] - 28)
    h += sum(b["laps"] < 3 for b in (r, dr, m, n)) * 20
    return round(h, 1)


def parse(text):
    m = re.search(r"\{.*\}", text, re.S)
    if not m:
        return None
    try:
        j = json.loads(m.group(0))
    except json.JSONDecodeError:
        return None
    c = j.get("change") or {}
    if c.get("name") not in RANGES or not isinstance(c.get("value"), (int, float)):
        j["change"] = None
    else:
        lo, hi = RANGES[c["name"]]
        c["value"] = round(min(hi, max(lo, float(c["value"]))), 3)
    if not isinstance(j.get("overall"), (int, float)):
        j["overall"] = 0
    return j


def llm_judge(ask, rep, params, history):
    user = (CHECKLIST + "\n\nCurrent assist settings (tunable, with allowed range):\n"
            + "\n".join(f"  {k} = {params[k]:g}  [{RANGES[k][0]}..{RANGES[k][1]}]" for k in RANGES)
            + "\n\nWhat the knobs do: drift_grip = rear grip cut inside a drift window; drift_spin = allowed rear wheelspin "
              "in a drift; hold_floor/hold_gain = throttle the drift hold keeps; hold_steer = drift hold counter-steer; "
              "catch_gain = spin catch strength; drift_max_deg = drift angle target at full stick; spin_angle_deg = "
              "catch fully on; stability/stab_angle_deg = catch for UNINTENDED slides outside a drift window; "
              "speed_keep = drive kept during drifts; tc/abs = traction control / anti-lock."
            + "\n\nTelemetry:\n" + rep
            + ("\n\nEarlier rounds:\n" + "\n".join(history[-6:]) if history else "")
            + '\n\nReply JSON: {"scores": {"racer_clean": 0-10, "easy_entry": 0-10, "no_snap": 0-10, "fair_cost": 0-10, '
              '"no_wobble": 0-10, "survivable": 0-10}, "overall": 0-10, "worst": "<checklist item>", '
              '"change": {"name": "<one knob>", "value": <number>}, "why": "<one sentence>"}\n'
              "Change exactly ONE knob, aimed at the worst item. Do not repeat a change that was rejected.")
    return parse(ask(user))


def main():
    rounds = int(sys.argv[sys.argv.index("--rounds") + 1]) if "--rounds" in sys.argv else 8
    dry = "--dry" in sys.argv
    proc = None
    if dry:
        def ask(user):
            k = random.choice(list(RANGES))
            lo, hi = RANGES[k]
            return json.dumps({"scores": {}, "overall": random.randint(3, 8), "worst": "dry",
                               "change": {"name": k, "value": random.uniform(lo, hi)}, "why": "dry run"})
    else:
        sys.path.insert(0, r"C:\Users\john\Documents\Tools\llm")
        import localread
        proc = localread.start_own(need_mb=localread.QWEN_MB, model=localread.QWEN)
        ask = lambda user: localread.ask(localread.OWN, SYSTEM, user, 600)

    base = run({})
    params = {k: base["params"][k] for k in RANGES}
    cur, cur_h = base, hard(base)
    log = [f"# judge log ({'dry' if dry else 'Qwen'})", "", f"start: hard={cur_h}", "```", report(cur), "```", ""]
    history = []
    try:
        j = llm_judge(ask, report(cur), params, history)
        for rnd in range(1, rounds + 1):
            if not j or not j.get("change"):
                log.append(f"## round {rnd}: no valid proposal, asking again")
                j = llm_judge(ask, report(cur), params, history)
                continue
            c = j["change"]
            trial = dict(params, **{c["name"]: c["value"]})
            new = run(trial)
            new_h = hard(new)
            jn = llm_judge(ask, report(new), trial, history)
            new_o = jn["overall"] if jn else 0
            keep = new_h < cur_h - 0.5 or (new_h <= cur_h + 1 and new_o > j["overall"])
            line = (f"round {rnd}: {c['name']} {params[c['name']]:g} -> {c['value']:g}: hard {cur_h} -> {new_h}, "
                    f"llm {j['overall']} -> {new_o}: {'KEPT' if keep else 'rejected'} ({j.get('why', '')})")
            print(line)
            history.append(line)
            log += [f"## round {rnd}", "", f"scores before: {j.get('scores')} worst: {j.get('worst')}", "", line, ""]
            if keep:
                params, cur, cur_h, j = trial, new, new_h, jn
                log += ["```", report(cur), "```", ""]
            else:
                j = llm_judge(ask, report(cur), params, history)
    finally:
        if proc:
            proc.kill()
    run(params)   # leave out\pt_*.csv and playtest.json on the best settings
    changed = {k: v for k, v in params.items() if v != base["params"][k]}
    log += ["## result", "", f"hard {hard(base)} -> {cur_h}", f"changed: {changed}", "```", report(cur), "```"]
    open(os.path.join(OUT, "judge_log.md"), "w", encoding="utf-8").write("\n".join(log) + "\n")
    json.dump(params, open(os.path.join(OUT, "judge_best.json"), "w"), indent=1)
    print("hard", hard(base), "->", cur_h, "changed", changed)


if __name__ == "__main__":
    main()
