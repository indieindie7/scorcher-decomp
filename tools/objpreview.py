# objpreview.py <in.obj> <out.png>: 4 flat-shaded views of an OBJ (matplotlib)
import sys, numpy as np, matplotlib; matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
V, F, G, g = [], [], [], 0
for l in open(sys.argv[1]):
    p = l.split()
    if not p: continue
    if p[0] == 'o': g += 1
    elif p[0] == 'v': V.append([float(x) for x in p[1:4]])
    elif p[0] == 'f': F.append([int(x.split('/')[0]) - 1 for x in p[1:]]); G.append(g)
V = np.array(V); cols = plt.cm.tab10(np.array(G) % 10)
fig = plt.figure(figsize=(12, 12))
for k, (el, az) in enumerate([(10, -90), (10, 0), (10, 90), (35, -135)]):
    ax = fig.add_subplot(2, 2, k + 1, projection='3d')
    ax.add_collection3d(Poly3DCollection([V[f][:, [0, 2, 1]] for f in F], facecolors=cols, edgecolors='k', linewidths=0.3))
    c = (V.max(0) + V.min(0)) / 2; r = (V.max(0) - V.min(0)).max() / 2
    ax.set_xlim(c[0] - r, c[0] + r); ax.set_ylim(c[2] - r, c[2] + r); ax.set_zlim(c[1] - r, c[1] + r)
    ax.view_init(el, az); ax.set_title(f'el {el} az {az}')
plt.tight_layout(); plt.savefig(sys.argv[2], dpi=80)
