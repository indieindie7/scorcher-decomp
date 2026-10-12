# Rips the CD-audio tracks of a raw (2352-byte sector) .cue/.img to WAV files:
#   py -3.13 -I ripaudio.py DISC.cue OUTDIR   -> OUTDIR\track02.wav ... (44.1 kHz 16-bit stereo)
# Each track runs from its INDEX 01 to the next track's INDEX 00 (the pregap is left out).
import os, re, sys, wave

cue, out = sys.argv[1], sys.argv[2]
img = os.path.join(os.path.dirname(cue), re.search(r'FILE "([^"]+)"', open(cue).read()).group(1))
tracks, cur = [], None
for line in open(cue):
    m = re.match(r'\s*TRACK (\d+) (\S+)', line)
    if m:
        cur = {'n': int(m.group(1)), 'audio': m.group(2) == 'AUDIO'}; tracks.append(cur)
    m = re.match(r'\s*INDEX (\d+) (\d+):(\d+):(\d+)', line)
    if m:
        cur['i%d' % int(m.group(1))] = (int(m.group(2)) * 60 + int(m.group(3))) * 75 + int(m.group(4))
total = os.path.getsize(img) // 2352
os.makedirs(out, exist_ok=True)
with open(img, 'rb') as f:
    for k, t in enumerate(tracks):
        if not t['audio']:
            continue
        end = tracks[k + 1].get('i0', tracks[k + 1]['i1']) if k + 1 < len(tracks) else total
        f.seek(t['i1'] * 2352)
        data = f.read((end - t['i1']) * 2352)
        p = os.path.join(out, 'track%02d.wav' % t['n'])
        with wave.open(p, 'wb') as w:
            w.setnchannels(2); w.setsampwidth(2); w.setframerate(44100); w.writeframes(data)
        print(p, '%.1f s' % ((end - t['i1']) / 75))
