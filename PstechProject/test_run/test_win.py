# -*- coding: utf-8 -*-
# Minimal test: drive PstechNative.dll C ABI directly, bypass C# auth, verify file path + Visage track
import os, ctypes, time
from ctypes import c_int, c_char_p, c_float, c_ubyte, c_longlong, POINTER, byref, Structure

run_dir = os.path.dirname(os.path.abspath(__file__))
os.add_dll_directory(run_dir)
os.chdir(run_dir)  # Visage data root = PstechNative.dll dir (run_dir); models flattened here

class PsTrackFaceData(Structure):
    _fields_ = [
        ("isDetected",   c_int),
        ("eyeClosure",   c_float * 2),
        ("headPose",     c_float * 3),
        ("faceRect",     c_float * 4),
        ("origin",       c_float * 2),
        ("landmarkCount", c_int),
        ("landmarks",    POINTER(c_float)),
    ]

print("[1] Loading PstechNative.dll (with all dependent dlls)...", flush=True)
dll = ctypes.CDLL(os.path.join(run_dir, "PstechNative.dll"))
print("    OK - all dependent dlls resolved", flush=True)

dll.Pstech_Camera_Init.argtypes = [c_int, c_char_p, c_char_p, c_char_p, c_int, c_int, c_int]
dll.Pstech_Camera_Init.restype = c_int
dll.Pstech_Camera_Start.restype = c_int
dll.Pstech_Camera_LockAndGet.argtypes = [POINTER(POINTER(c_ubyte)), POINTER(c_int), POINTER(c_int), POINTER(c_longlong)]
dll.Pstech_Camera_LockAndGet.restype = c_int
dll.Pstech_Alg_Init.argtypes = [c_char_p]
dll.Pstech_Alg_Track.argtypes = [POINTER(c_ubyte), c_int, c_int, POINTER(PsTrackFaceData)]

cfg   = os.path.join(run_dir, "Facial Features Tracker.cfg").encode("utf-8")  # data flattened to dll dir
video = os.path.join(run_dir, "test.mp4").encode("utf-8")

print("[2] Pstech_Alg_Init (Visage tracker)...", flush=True)
dll.Pstech_Alg_Init(cfg)
print("    OK", flush=True)

print("[3] Pstech_Camera_Init (file path, test.mp4)...", flush=True)
r = dll.Pstech_Camera_Init(0, video, b"", b"", 0, 640, 480)
print(f"    return={r} (0=ok)", flush=True)

print("[4] Pstech_Camera_Start...", flush=True)
dll.Pstech_Camera_Start()
time.sleep(0.5)

print("[5] Grab + Visage track (15 frames, per-frame log; 1st frame may load models)...", flush=True)
fd = PsTrackFaceData()
frames = 0
detected = 0
states = {}
for i in range(15):
    ptr = POINTER(c_ubyte)()
    w = c_int(); h = c_int(); ts = c_longlong()
    state = dll.Pstech_Camera_LockAndGet(byref(ptr), byref(w), byref(h), byref(ts))
    states[state] = states.get(state, 0) + 1
    msg = f"  frame{i} state={state}"
    if ptr and state == 1:
        frames += 1
        n = w.value * h.value * 3
        imgbuf = (c_ubyte * n)()
        ctypes.memmove(imgbuf, ptr, n)   # copy out of RingBuffer (avoid overwrite during long track)
        fd = PsTrackFaceData()           # fresh per frame
        t0 = time.time()
        dll.Pstech_Alg_Track(ctypes.cast(imgbuf, POINTER(c_ubyte)), w.value, h.value, byref(fd))
        dt = (time.time() - t0) * 1000
        msg += f" {w.value}x{h.value} track={dt:.0f}ms det={fd.isDetected}"
        if fd.isDetected:
            detected += 1
            msg += f" eye=({fd.eyeClosure[0]:.2f},{fd.eyeClosure[1]:.2f}) pose=({fd.headPose[0]:.1f},{fd.headPose[1]:.1f},{fd.headPose[2]:.1f}) lm={fd.landmarkCount}"
    print(msg, flush=True)
    time.sleep(0.02)

print(f"\n[RESULT] frame states={states}", flush=True)
print(f"[RESULT] valid frames={frames}, face-detected frames={detected}", flush=True)
try:
    dll.Pstech_Camera_Stop()
except Exception:
    pass
print("[DONE]", flush=True)
