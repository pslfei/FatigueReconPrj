import os, ctypes, time
from ctypes import c_int, c_char_p, c_float, c_ubyte, c_longlong, c_double, POINTER, byref, Structure

run_dir = os.path.dirname(os.path.abspath(file))
os.add_dll_directory(run_dir)
os.chdir(run_dir)  # Visage data root = PstechNative.dll dir

class PsTrackFaceData(Structure):
    fields = [
        ("isDetected",   c_int),
        ("eyeClosure",   c_float * 2),
        ("headPose",     c_float * 3),
        ("faceRect",     c_float * 4),
        ("origin",       c_float * 2),
        ("landmarkCount", c_int),
        ("landmarks",    POINTER(c_float)),
    ]

print("[1] Loading PstechNative.dll...", flush=True)
dll = ctypes.CDLL(os.path.join(run_dir, "PstechNative.dll"))
print("    OK", flush=True)

dll.Pstech_Camera_Init.argtypes = [c_int, c_char_p, c_char_p, c_char_p, c_int, c_int, c_int]
dll.Pstech_Camera_Init.restype = c_int
dll.Pstech_Camera_Start.restype = c_int
dll.Pstech_Camera_LockAndGet.argtypes = [POINTER(POINTER(c_ubyte)), POINTER(c_int), POINTER(c_int), POINTER(c_longlong)]
dll.Pstech_Camera_LockAndGet.restype = c_int
dll.Pstech_Camera_GetBrightness.restype = c_double
dll.Pstech_Alg_Init.argtypes = [c_char_p]
dll.Pstech_Alg_Track.argtypes = [POINTER(c_ubyte), c_int, c_int, POINTER(PsTrackFaceData)]

cfg = os.path.join(run_dir, "Facial Features Tracker.cfg").encode("utf-8")

print("[2] Pstech_Alg_Init (Visage)...", flush=True)
dll.Pstech_Alg_Init(cfg)
print("    OK", flush=True)

print("[3] Pstech_Camera_Init (HIK 192.168.1.69:8000 admin)...", flush=True)
r = dll.Pstech_Camera_Init(2, b"192.168.1.69", b"admin 480)
print(f"    return={r}", flush=True)

print("[4] Pstech_Camera_Start...", flush=True)
dll.Pstech_Camera_Start()
print("    waiting 6s for HIK login + RTSP stream + PlayM4 decode...", flush=True)
time.sleep(6)

print("[5] Grab + Visage track (50 frames)...", flush=True)
frames = 0; detected = 0; states = {}
for i in range(50):
    ptr = POINTER(c_ubyte)()
    w = c_int(); h = c_int(); ts = c_longlong()
    state = dll.Pstech_Camera_LockAndGet(byref(ptr), byref(w), byref(h), byref(ts))
    states[state] = states.get(state, 0) + 1
    line = None
    if ptr and state == 1:
        frames += 1
        n = w.value * h.value * 3
        imgbuf = (c_ubyte * n)()
        ctypes.memmove(imgbuf, ptr, n)
        fd = PsTrackFaceData()
        dll.Pstech_Alg_Track(ctypes.cast(imgbuf, POINTER(c_ubyte)), w.value, h.value, byref(fd))
        line = f"  frame{i} state=OK {w.value}x{h.value} det={fd.isDetected}"
        if fd.isDetected:
            detected += 1
            line += f" eye=({fd.eyeClosure[0]:.2f},{fd.eyeClosure[1]:.2f}) pose=({fd.headPose[0]:.1f},{fd.headPose[1]:.1f},{fd.headPose[2]:.1f}) lm={fd.landmarkCount}"
    elif i % 10 == 0:
        line = f"  frame{i} state={state} (0=empty 2=blackscreen/mock)"
    if line:
        print(line, flush=True)
    time.sleep(0.05)

print(f"\n[RESULT] frame states={states}", flush=True)
print(f"[RESULT] valid(OK) frames={frames}, face-detected={detected}", flush=True)
print(f"[brightness] {dll.Pstech_Camera_GetBrightness():.1f}", flush=True)
try:
    dll.Pstech_Camera_Stop()
except Exception:
    pass
print("[DONE]", flush=True)
</parameter>
<parameter name="description">写海康摄像头测试脚本</parameter>
</invoke>

✻ Worked for 42s

❯ 帮我生成文件并测试

court
<invoke name="Write">
<parameter name="file_path">E:\FatigueReconPrj-mumu\PstechProject\test_run\test_hik.py</parameter>
<parameter name="content"># -- coding: utf-8 --
HIK camera test: drive PstechNative.dll C ABI, login 1sage track
.dirname(os.path.abspath(file))
os.add_dll_directory(run_dir)
os.chdir(run_dir)

class PsTrackFaceData(Structure):
    fields = [
        ("isDetected",   c_int),
        ("eyeClosure",   c_float * 2),
        ("headPose",     c_float * 3),
        ("faceRect",     c_float * 4),
        ("origin",       c_float * 2),
        ("landmarkCount", c_int),
        ("landmarks",    POINTER(c_float)),
    ]

print("[1] Loading PstechNative.dll...", flush=True)
dll = ctypes.CDLL(os.path.join(run_dir, "PstechNative.dll"))
print("    OK", flush=True)

dll.Pstech_Camera_Init.argtypes = [c_int, c_char_p, c_char_p, c_char_p, c_int, c_int, c_int]
dll.Pstech_Camera_Init.restype = c_int
dll.Pstech_Camera_Start.restype = c_int
dll.Pstech_Camera_LockAndGet.argtypes = [POINTER(POINT POINTER(c_int), POINTER(c_longlong)]
n(run_dir, "Facial Features Tracker.cfg").encode("utf-8")

print("[2] Pstech_Alg_Init (Visage)...", flush=True)
dll.Pstech_Alg_Init(cfg)
print("    OK", flush=True)

print("[3] Pstech_Camera_Init (HIK 192.168.1.69:8000 admin)...", flush=True)
r = dll.Pstech_Camera_Init(2, b"192.168.1.69", b"admin", b"admin123456", 8000, 640, 480)
print(f"    return={r}", flush=True)

print("[4] Pstech_Camera_Start...", flush=True)
dll.Pstech_Camera_Start()
print("    waiting 6s for HIK login + stream + PlayM4 decode...", flush=True)
time.sleep(6)

print("[5] Grab + Visage track (50 frames)...", flush=
frames = 0; detected = 0; states = {}
for i in range(50):
    ptr = POINTER(c_ubyte)()                                                   w = c_int(); h = c_int(); ts = c_longlong()
    state = dll.Pstech_Camera_LockAndGet(byref(ptr), byref(w), byref(h), byref(ts))
    states[state] = states.get(state, 0) + 1
    line = None
    if ptr and state == 1:
        frames += 1
        n = w.value * h.value * 3
        imgbuf = (c_ubyte * n)()
        ctypes.memmove(imgbuf, ptr, n)
        fd = PsTrackFaceData()
        dll.Pstech_Alg_Track(ctypes.cast(imgbuf, POINTER(c_ubyte)), w.value, h.value, byref(fd))
        line = f"  frame{i} state=OK {w.value}x{h.value} det={fd.isDetected}"
        if fd.isDetected:
            detected += 1
            line += f" eye=({fd.eyeClosure[0]:.2f},{fd.eyeClosure[1]:.2f}) pose=({fd.headPose[0]:.1f},{fd.headPose[1]:.1f},{fd.herkCount}"
    elif i % 10 == 0:
        line = f"  frame{i} state={state} (0=empty 2=blackscreen/mock)"
    if line:
        print(line, flush=True)
    time.sleep(0.05)

print(f"\n[RESULT] frame states={states}", flush=True)
print(f"[RESULT] valid(OK) frames={frames}, face-detected={detected}", flush=True)
print(f"[brightness] {dll.Pstech_Camera_GetBrightness():.1f}", flush=True)
try:
    dll.Pstech_Camera_Stop()
except Exception:
    pass
print("[DONE]", flush=True)