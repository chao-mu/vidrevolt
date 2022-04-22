#!/usr/bin/env python3

import cv2

def main():
    index = 0
    arr = []
    i = 10

    while i > 0:
        cap = cv2.VideoCapture(index)
        if cap.read()[0]:
            arr.append(index)
            cap.release()
        index += 1
        i -= 1

    return arr


if __name__ == "__main__":
    main()

