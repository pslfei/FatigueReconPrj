import os, ctypes, time
from ctypes import c_int, c_char_p, c_float, c_ubyte, c_longlong, c_double, POINTER, byref, Structure
run_dir = os.path.dirname(os.path.abspath(__file__))
os.add_dll_directory(run_dir)
os.chdir(run_dir)
class PsTrackFaceData(Structure):
    _fields_ = [("isDetected",c_int),("eyeClosure",c_float*2),("headPose",c_float*3),("faceRect",c_float*4),("origin",c_float*2),("landmarkCount",c_int),("landmarks",POINTER(c_float))]
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
print("[2] Pstech_Alg_Init...", flush=True)
dll.Pstech_Alg_Init(cfg)
print("    OK", flush=True)
print("[3] Pstech_Camera_Init USB index=0 (CAP_DSHOW)...", flush=True)
r = dll.Pstech_Camera_Init(1, b"", b"", b"", 0, 640, 480)
print("    return=%d" % r, flush=True)
print("[4] Pstech_Camera_Start, waiting 5s for USB open + frames...", flush=True)
dll.Pstech_Camera_Start()
time.sleep(5)
print("[5] Grab + track 50 frames...", flush=True)
frames=0; detected=0; states={}
for i in range(50):
    ptr = POINTER(c_ubyte)()
    w=c_int(); h=c_int(); ts=c_longlong()
    state = dll.Pstech_Camera_LockAndGet(byref(ptr), byref(w), byref(h), byref(ts))
    states[state] = states.get(state,0)+1
    line=None
    if ptr and state==1:
        frames+=1
        n=w.value*h.value*3
        imgbuf=(c_ubyte*n)()
        ctypes.memmove(imgbuf, ptr, n)
        fd=PsTrackFaceData()
        dll.Pstech_Alg_Track(ctypes.cast(imgbuf, POINTER(c_ubyte)), w.value, h.value, byref(fd))
        line="  frame%d OK %dx%d det=%d" % (i, w.value, h.value, fd.isDetected)
        if fd.isDetected:
            detected+=1
            line+=" eye=(%.2f,%.2f) pose=(%.1f,%.1f,%.1f) lm=%d" % (fd.eyeClosure[0],fd.eyeClosure[1],fd.headPose[0],fd.headPose[1],fd.headPose[2],fd.landmarkCount)
    elif i%10==0:
        line="  frame%d state=%d" % (i, state)
    if line: print(line, flush=True)
    time.sleep(0.05)
print("", flush=True)
print("[RESULT] states=%s valid=%d faces=%d" % (states, frames, detected), flush=True)
print("[brightness] %.1f" % dll.Pstech_Camera_GetBrightness(), flush=True)
try: dll.Pstech_Camera_Stop()
except: pass
print("[DONE]", flush=True)
