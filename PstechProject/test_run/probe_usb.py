import cv2, sys
print("OpenCV:", cv2.__version__, flush=True)
for idx in range(4):
    for backend, name in [(cv2.CAP_DSHOW,"DSHOW"), (cv2.CAP_MSMF,"MSMF")]:
        try:
            cap = cv2.VideoCapture(idx, backend)
            ok = cap.isOpened()
            if ok:
                ret, frame = cap.read()
                shape = frame.shape if ret and frame is not None else None
                print("idx=%d %s opened=%s read=%s shape=%s" % (idx, name, ok, ret, shape), flush=True)
            cap.release()
        except Exception as e:
            print("idx=%d %s EXC %s" % (idx, name, e), flush=True)
