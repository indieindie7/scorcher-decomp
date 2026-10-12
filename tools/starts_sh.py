# starts_sh.py <unowned.txt> <bin> <base>: SH-2 function starts in unowned ranges that are not only READ/DATA-referenced.
# A start = the first 2-aligned word in the first 24 bytes that is a push (mov.l rN,@-r15 = 2fN6) or sts.l pr,@-r15
# (4f22), when the words before it are only rts/nop/zero or literal-pool longwords (4-aligned).
import sys, struct, re
d = open(sys.argv[2], "rb").read(); base = int(sys.argv[3], 16)
def w(a): return struct.unpack(">H", d[a - base:a - base + 2])[0]
out = []
for l in open(sys.argv[1]):
    p = l.split()
    if len(p) < 5 or p[0] != "U": continue
    s, e, n = int(p[1], 16), int(p[2], 16), int(p[3])
    refs = l.split("|", 1)[1] if "|" in l else ""
    if n < 8: continue
    if refs.strip() and ("READ" in refs or "DATA" in refs) and "CALL" not in refs and "JUMP" not in refs: continue
    if s % 2 == 0 and re.search(">%08x@[^ ]*:DATA" % s, refs): out.append("%x" % s); continue
    a = s + (s & 1)
    while a < min(e, s + 24) - 2:
        x = w(a)
        if (x & 0xff0f) == 0x2f06 or x == 0x4f22: out.append("%x" % a); break
        a += 2
print(" ".join(x for x in out if x not in sys.argv[4:]))
