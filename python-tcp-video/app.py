import socket

import cv2

if __name__ == "__main__":
    print('请保障PC和开发板访问同一个局域网！')
    ip = input('请输入开发板IP地址：')
    cap = cv2.VideoCapture('video.mp4')
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, 8888))
    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]
    while cap.isOpened():
        _, frame = cap.read()
        dst = cv2.resize(frame, (160, 80))
        _, jpg = cv2.imencode('.jpg', dst, encode_param)
        ba = bytearray(jpg)
        cv2.imshow('frame', frame)
        s.send(ba)
        if cv2.waitKey(25) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()
    s.close()
