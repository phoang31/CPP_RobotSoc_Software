import numpy as np
import cv2
def nothing(x):                                     
    pass



cap = cv2.VideoCapture(1)                             #create object of Video Capture           
cap.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH, 320.0)              #resize frame width and height for quicker processing
cap.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT, 240)

cv2.namedWindow('image',1)                            #create named window
cv2.createTrackbar('H center','image',0,255,nothing)        #create trackbar for hsv upper and lower parameters
cv2.createTrackbar('S center','image',0,255,nothing)
cv2.createTrackbar('V center','image',0,255,nothing)

cv2.createTrackbar('H span','image',0,255,nothing)
cv2.createTrackbar('S span','image',0,255,nothing)
cv2.createTrackbar('V span','image',0,255,nothing)

# allocate arrays to iterate over
center = np.array([0,0,0])
span = np.array([0,0,0])
lower_thresh = np.array([0,0,0])
upper_thresh = np.array([0,0,0])

while(cap.isOpened()):                                #while a webcam is connected

    ret,frame = cap.read()                            #cap.read() returns boolean ret and frame
    if(ret == False):
        print "Invalid frame caught."
        continue
    hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)       #change frame from RGB to HSV space
    hsv[:,:,2]-=60
    #Get hsv upper and lower values from trackbar
    center[0] = cv2.getTrackbarPos('H center','image')             
    center[1] = cv2.getTrackbarPos('S center','image') 
    center[2] = cv2.getTrackbarPos('V center','image')
    
    span[0] = cv2.getTrackbarPos('H span','image')
    span[1] = cv2.getTrackbarPos('S span','image')
    span[2] = cv2.getTrackbarPos('V span','image')
    
    # calculate HSV ranges from input params
    for i in range(0,3):
        lower_thresh[i] = center[i] - 0.5 * span[i]
        upper_thresh[i] = center[i] + 0.5 * span[i]
        if lower_thresh[i] < 0:
            lower_thresh[i] = 0
        if upper_thresh[i] > 255:
            upper_thresh[i] = 255

    mask = cv2.inRange(hsv,lower_thresh,upper_thresh) #isolate pixels within upper and lower hsv values
    mask = cv2.GaussianBlur(mask,(3,3),2)             #apply Gaussian blurring to remove noise within isolated image
    mask = cv2.dilate(mask, np.ones((5,5),np.uint8)) 
    mask = cv2.erode(mask, np.ones((5,5),np.uint8))
    res = cv2.bitwise_and(frame,frame,mask = mask)    #perform bitwise and to create res image that retains color of ball
    #define circles
    circles = cv2.HoughCircles(mask, cv2.cv.CV_HOUGH_GRADIENT,1.1,20,param1=50,param2=30,minRadius=3,maxRadius=16)
    if circles is not None:                           #as long as there is at least one circle: 
        circles = np.round(circles[0,:]).astype("int")#round elements of circles array to integers

        for (x,y,r) in circles:                       #place circle around detected circles 
                                                      #place a 5x5 square at center of circle
            cv2.circle(frame,(x,y),r,(0,255,0),4)     #display circle center coordinates
            cv2.rectangle(frame, (x-5,y-5),(x+5,y+5),(0,128,255),-1)
            print "ball position x = " + str(x) + ", y = " + str(y) + ", radius = " + str(r)

    cv2.imshow('frame',frame)                         #show original frame
    cv2.imshow('mask',mask)                           #show masked frame
    cv2.imshow('res',res)                             #show res frame

    k = cv2.waitKey(1)                                #wait for escape key
    if k == 27:
       break                                          #if escape key was pressed end process 

cv2.destroyAllWindows()                               #close all windows and remove memory
cap.release()

    

   

