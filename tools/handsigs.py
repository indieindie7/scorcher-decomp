# handsigs.py <hand.txt> <funcinfo.tsv> <out.txt>: turn hand comments that start with an argument list
# "(a, &b, c)" into SetHandSigs lines "0xaddr a,*b,c". Skips lists that look like caller lists (entries
# that are function names), register notes (=, ;, esi/edi/ecx...), or anything with spaces inside an entry.
import re, sys
hand, tsv, out = sys.argv[1:4]
funcs = {l.split("\t")[2] if "\t" in l else l.split()[2] for l in open(tsv, encoding="utf-8", errors="replace") if l.startswith("F")}
regs = {"eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp", "r4", "r5", "r6", "r7"}
rx = re.compile(r"^(0x[0-9a-fA-F]+) func (\S+) hand \(([^()]*)\)")
lines, skipped = [], 0
for l in open(hand, encoding="utf-8", errors="replace"):
    m = rx.match(l)
    if not m: continue
    addr, name, args = m.groups()
    args = args.strip()
    if args == "":
        lines.append(f"{addr} -"); continue
    if any(c in args for c in "=;:") or "..." in args:
        skipped += 1; continue
    out_p, ok, seen = [], True, set()
    for i, e in enumerate(x.strip() for x in args.split(",")):
        ptr = e.startswith("&") or e.startswith("*")
        e = e.lstrip("&*").strip()
        if e in ("?", ""): e = f"arg{i+1}"
        if e in funcs or e.lower() in regs or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", e):
            ok = False; break
        if e in seen: e = f"{e}{i+1}"
        seen.add(e)
        out_p.append(("*" if ptr else "") + e)
    if not ok or len(out_p) > 12:
        skipped += 1; continue
    lines.append(f"{addr} " + ",".join(out_p))
open(out, "w").write("\n".join(lines) + "\n")
print(f"{hand}: {len(lines)} signatures, {skipped} skipped")
