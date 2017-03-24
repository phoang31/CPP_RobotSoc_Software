import cv2
import numpy as np
#load an image and convert it to grayscale
img = cv2.imread("zumoTest.png")
gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
#canny = cv2.Canny(gray, 400, 400)

ret, thresh = cv2.threshold(gray, 220,255,cv2.THRESH_BINARY)

#res, contours, hierarchy = cv2.findContours(canny, cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
res, contours, hierarchy = cv2.findContours(thresh, cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)
cv2.imshow('contour',res)


largestArea = 0.0
largestCnt = cv2.contourArea(contours[0])#what an unfortunate variable name
for cnt in contours:
    area = cv2.contourArea(cnt)
    if area >= largestArea:
        largestArea = area
        largestCnt = cnt
        
rect = cv2.minAreaRect(largestCnt)
box = cv2.boxPoints(rect)
print("angle = %.2f degrees" % rect[2])
box = np.int0(box)
cv2.drawContours(img,[box],-1,(0,0,255),3)
cv2.imshow("grayscale", gray)
cv2.imshow("original",img)

#clean up
cv2.waitKey(0)
cv2.destroyAllWindows()
