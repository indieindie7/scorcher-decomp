# plot.py: bikesim scenario plots -> out\scenarios.png  (py -3.13 plot.py)
import csv, os, math
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

here = os.path.dirname(os.path.abspath(__file__))
names = ["launch", "corner", "slalom", "brake", "brakedrift", "powerdrift", "drifthold"]
fig, ax = plt.subplots(len(names), 3, figsize=(15, 3 * len(names)))
for i, n in enumerate(names):
    rows = list(csv.DictReader(open(os.path.join(here, "out", n + ".csv"))))
    f = lambda k: [float(r[k]) for r in rows]
    t, x, y, yaw = f("t"), f("x"), f("y"), f("yaw")
    a = ax[i][0]
    a.plot(x, y, lw=1)
    for j in range(0, len(rows), 15):   # heading ticks every 0.25 s
        a.plot([x[j], x[j] + 2 * math.cos(yaw[j])], [y[j], y[j] + 2 * math.sin(yaw[j])], "r", lw=0.8)
    a.set_aspect("equal"); a.set_title(n + ": path (red = heading)")
    a = ax[i][1]
    a.plot(t, f("beta"), label="drift deg"); a.plot(t, f("delta"), label="steer deg"); a.plot(t, f("lean"), label="lean deg")
    a.legend(fontsize=7); a.grid(alpha=0.3)
    a = ax[i][2]
    a.plot(t, f("speed"), label="speed m/s"); a.plot(t, [10 * v for v in f("kr")], label="rear slip x10")
    a.plot(t, [10 * v for v in f("thr_eff")], label="throttle x10"); a.plot(t, f("gear"), label="gear")
    a.legend(fontsize=7); a.grid(alpha=0.3)
fig.tight_layout()
fig.savefig(os.path.join(here, "out", "scenarios.png"), dpi=70)
print("out\\scenarios.png")
