"""Align functions across the three Scorcher builds using the facts exported by ghidra/scripts/ExportFuncInfo.java.

usage: py -3.13 xalign.py <outdir> <label>=<funcinfo.tsv> [<label>=<tsv> ...]
Seeds: equal user-defined names (routine-table slots), equal imported names, and string literals referenced by exactly one
function in each of two builds. Propagation: aligned functions with the same number of calls align their callees
position by position (needs 2 votes between different compilers, 1 between the two Watcom builds).
Output: <outdir>/xalign_<label>.txt in ApplyNames format (addr func name xalign <counterparts...>) and a summary.
"""
import re, sys, os
from collections import defaultdict, Counter

def load(path):
    funcs = {}; cur = None
    for line in open(path, encoding="utf-8", errors="replace"):
        t = line.rstrip("\n").split(" ", 1)
        if t[0] == "F":
            addr, name, src, size = t[1].split(" ")
            cur = funcs[addr] = dict(addr=addr, name=name, src=src, size=int(size), calls=[], strs=[])
        elif t[0] == "C" and cur: cur["calls"].append(t[1])
        elif t[0] == "S" and cur: cur["strs"].append(t[1])
    return funcs

def slug(s):
    s = re.sub(r"[^A-Za-z0-9]+", "_", s).strip("_")
    return s[:24] if s else ""

def main():
    outdir = sys.argv[1]; progs = {}
    for spec in sys.argv[2:]:
        lab, path = spec.split("=", 1); progs[lab] = load(path)
    labels = list(progs)
    watcom = {"scordos", "SCORWIN"}
    # evidence[(labA, addrA, labB, addrB)] = votes
    votes = Counter(); reasons = defaultdict(list)
    def add(la, a, lb, b, n, why):
        key = (la, a, lb, b) if la < lb else (lb, b, la, a)
        votes[key] += n; reasons[key].append(why)
    for i, la in enumerate(labels):
        for lb in labels[i + 1:]:
            A, B = progs[la], progs[lb]
            # name seeds
            byname = defaultdict(list)
            for f in A.values():
                if f["src"] in ("USER_DEFINED", "IMPORTED"): byname[f["name"]].append(f["addr"])
            for f in B.values():
                if f["src"] in ("USER_DEFINED", "IMPORTED") and len(byname.get(f["name"], [])) == 1:
                    add(la, byname[f["name"]][0], lb, f["addr"], 3, "name:" + f["name"])
            # unique string seeds
            sa = defaultdict(set); sb = defaultdict(set)
            for f in A.values():
                for s in f["strs"]: sa[s].add(f["addr"])
            for f in B.values():
                for s in f["strs"]: sb[s].add(f["addr"])
            for s in sa:
                if len(sa[s]) == 1 and len(sb.get(s, ())) == 1 and len(s) >= 3:
                    add(la, next(iter(sa[s])), lb, next(iter(sb[s])), 2, "str:" + s)
    def resolve():
        """Pick, per (labA,labB), a one-to-one mapping by best votes with a clear margin."""
        best = {}
        cand = defaultdict(list)
        for (la, a, lb, b), n in votes.items(): cand[(la, lb)].append((n, a, b))
        pairs = {}
        for key, lst in cand.items():
            lst.sort(reverse=True); usedA = set(); usedB = set()
            topA = defaultdict(list); topB = defaultdict(list)
            for n, a, b in lst: topA[a].append(n); topB[b].append(n)
            for n, a, b in lst:
                if a in usedA or b in usedB: continue
                if len(topA[a]) > 1 and topA[a][1] == n: continue   # ambiguous
                if len(topB[b]) > 1 and topB[b][1] == n: continue
                pairs[(key[0], a, key[1], b)] = n; usedA.add(a); usedB.add(b)
        return pairs
    pairs = resolve()
    for rnd in range(4):
        before = len(pairs)
        for (la, a, lb, b), n in list(pairs.items()):
            same = la in watcom and lb in watcom
            def calls(P, f):   # across compilers, ignore calls to the (Watcom-only) runtime
                return [c for c in P[f]["calls"] if c in P and (same or P[c]["src"] != "IMPORTED")]
            ca, cb = calls(progs[la], a), calls(progs[lb], b)
            if ca and len(ca) == len(cb):
                for x, y in zip(ca, cb):
                    if x == a and y == b: continue
                    if not same:
                        sx = set(progs[la][x]["strs"]); sy = set(progs[lb][y]["strs"])
                        if sx and sy and not (sx & sy): continue   # contradicting string literals
                        if progs[la][x]["src"] == "IMPORTED" or progs[lb][y]["src"] == "IMPORTED": continue
                    add(la, x, lb, y, 1, f"call-order via {a}/{b}")
        pairs = resolve()
        # cross-compiler pairs need >= 2 votes
        pairs = {k: v for k, v in pairs.items() if v >= (1 if (k[0] in watcom and k[2] in watcom) else 2)}
        print(f"round {rnd}: {before} -> {len(pairs)} pairs")
    # union-find into groups, one function per build
    parent = {}
    def find(x):
        while parent.setdefault(x, x) != x: x = parent[x]
        return x
    for (la, a, lb, b) in pairs:
        ra, rb = find((la, a)), find((lb, b))
        if ra != rb: parent[ra] = rb
    groups = defaultdict(list)
    for node in list(parent): groups[find(node)].append(node)
    out = {lab: [] for lab in labels}; named = Counter(); synthetic = 0; conflicts = 0
    for g in groups.values():
        per = defaultdict(list)
        for lab, addr in g: per[lab].append(addr)
        if any(len(v) > 1 for v in per.values()): conflicts += 1; continue
        members = {lab: progs[lab][addr] for lab, addr in g}
        name = None
        for pri in ("USER_DEFINED", "IMPORTED"):
            for f in members.values():
                if f["src"] == pri: name = f["name"]; break
            if name: break
        if not name:
            common = None
            for f in members.values():
                ss = set(s for s in f["strs"] if len(s) >= 3)
                common = ss if common is None else common & ss
            key = sorted(common, key=len)[0] if common else None
            canon = members.get("scordos") or members.get("SCORWIN") or next(iter(members.values()))
            name = ("s_" + slug(key) if key and slug(key) else "fn") + "_" + canon["addr"].lstrip("0")
            synthetic += 1
        for lab, f in members.items():
            others = " ".join(f"{l}:{m['addr']}" for l, m in members.items() if l != lab)
            if f["name"] == name: continue
            out[lab].append(f"0x{f['addr']} func {name} xalign {others}")
            named[lab] += 1
    for lab in labels:
        with open(os.path.join(outdir, f"xalign_{lab}.txt"), "w") as fh: fh.write("\n".join(out[lab]) + "\n")
    print("groups", len(groups), "conflicting", conflicts, "synthetic names", synthetic, "to apply", dict(named))
    for lab in labels:
        sizes = Counter(len(g) for g in groups.values())
    print("group sizes", dict(Counter(len(g) for g in groups.values())))

main()
