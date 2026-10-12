# starts.py <unowned.txt> <lo> <hi>: print function starts (hex) for unowned ranges that begin, after padding,
# with an x86 prologue: push n; call (Watcom __CHK), push-register runs, mov ebp,esp, sub esp, fld [esp+n].
import sys, re
lo, hi = int(sys.argv[2], 16), int(sys.argv[3], 16)
PRO = re.compile(r"^(68..000000e8|6a..e8|(5[0-7]){2}|5589e5|83ec|81ec|dd4424|8b4424|5[3-7]8[9b])")
out = []
for l in open(sys.argv[1]):
    p = l.split()
    if len(p) < 5 or p[0] != "U": continue
    s, n, h = int(p[1], 16), int(p[3]), p[4]
    if n < 8 or not (lo <= s < hi): continue
    a = s
    while True:
        if h.startswith("8d4000"): h = h[6:]; a += 3
        elif h.startswith("8bc0"): h = h[4:]; a += 2
        elif h.startswith("90") or h.startswith("00") or h.startswith("cc"): h = h[2:]; a += 1
        else: break
    if len(h) >= 6 and PRO.match(h): out.append("%x" % a)
print(" ".join(out))
