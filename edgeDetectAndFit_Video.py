import cv2
import numpy as np

LOWER_BINARY_THRESH = 150
UPPER_BINARY_THRESH = 255

## [start webcam]
cap = cv2.VideoCapture(0)   #use 1 for external webcam, 0 for built in
## [frame operations]
invalidFrameCount = 0
while cap.isOpened():
    retval,frame=cap.read()
    if(retval == False):            #check if we have a valid frame. try again if not
        invalidFrameCount += 1
        print "invalid frame: %d" % invalidFrameCount
        continue
    #convert to grayscale
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    #binary(black and white)thresholding
    ret, thresh = cv2.threshold(gray, LOWER_BINARY_THRESH,UPPER_BINARY_THRESH,cv2.THRESH_BINARY)
    #getting list of contour vectors from frame
    res, contours, hierarchy = cv2.findContours(thresh, cv2.RETR_LIST,cv2.CHAIN_APPROX_SIMPLE)

    #only draw a rectangle around the largest contour found
    largestArea = 0.0
    largestCnt = None#what an unfortunate variable name
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area >= largestArea:
            largestArea = area
            largestCnt = cnt
    if largestCnt != None:       
        rect = cv2.minAreaRect(largestCnt)
        box = cv2.boxPoints(rect)
        box = np.int0(box)

        ##TODO: make this code section more readable and efficient
        (x1,y1), (width,height), angle = rect
        if width < height:
            angle-=90
        print("angle=%6.2f degrees\twidth=%6.2fpx\theight=%6.2fpx" % (-angle,width,height))
        mag = 0.5*np.sqrt(width**2+height**2)
        angle = np.deg2rad(angle)
        x2 = int(mag*np.cos(angle)+x1)
        y2 = int(mag*np.sin(angle)+y1)
        
        
        cv2.drawContours(frame,[box],-1,(0,0,255),3)
        cv2.line(frame, (int(x1),int(y1)),(x2,y2),(0,255,0),3)
        cv2.putText(frame, str(round(np.rad2deg(-angle), 2))+" deg", (x2,y2), cv2.FONT_HERSHEY_SIMPLEX,1, (255,0,0))
        ##

    k=cv2.waitKey(5) & 0xff     #display for 5ms, then automatically close
    if k==27:                   #escape key ascii value
        break
    cv2.imshow("markup",frame)
    cv2.imshow('contour',res)
    

## [clean up]
cv2.destroyAllWindows()
cap.release()
