# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'TrackColors.ui'
#
# Created by: PyQt4 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!


#############################################
from PyQt4.QtGui import QMainWindow, QIcon, QDesktopWidget, QMessageBox, QApplication, QPushButton, QSlider
from PyQt4 import QtCore, QtGui
import sys
##############################################

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)
    
####################################################
class Ui_MainWindow(QMainWindow):
    def __init__(self):									#constructor for inherited class
        super(Ui_MainWindow,self).__init__()			#constructor for parent class	
        self.setupUi(self)
#####################################################
    def setupUi(self, MainWindow):
        MainWindow.setObjectName(_fromUtf8("MainWindow"))
        MainWindow.resize(311, 531)
        MainWindow.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.layoutWidget = QtGui.QWidget(self.centralwidget)
        self.layoutWidget.setGeometry(QtCore.QRect(30, 30, 251, 470))
        self.layoutWidget.setObjectName(_fromUtf8("layoutWidget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.layoutWidget)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.pushButton_Start = QtGui.QPushButton(self.layoutWidget)
        self.pushButton_Start.setObjectName(_fromUtf8("pushButton_Start"))
        self.verticalLayout.addWidget(self.pushButton_Start)
        self.pushButton_Stop = QtGui.QPushButton(self.layoutWidget)
        self.pushButton_Stop.setObjectName(_fromUtf8("pushButton_Stop"))
        self.verticalLayout.addWidget(self.pushButton_Stop)
        self.label = QtGui.QLabel(self.layoutWidget)
        self.label.setObjectName(_fromUtf8("label"))
        self.verticalLayout.addWidget(self.label)
        self.horizontalLayout_HC = QtGui.QHBoxLayout()
        self.horizontalLayout_HC.setObjectName(_fromUtf8("horizontalLayout_HC"))
        self.spinBox_HC = QtGui.QSpinBox(self.layoutWidget)
        self.spinBox_HC.setMaximum(255)
        self.spinBox_HC.setObjectName(_fromUtf8("spinBox_HC"))
        self.horizontalLayout_HC.addWidget(self.spinBox_HC)
        self.horizontalSlider_HC = QtGui.QSlider(self.layoutWidget)
        self.horizontalSlider_HC.setMaximum(255)
        self.horizontalSlider_HC.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalSlider_HC.setObjectName(_fromUtf8("horizontalSlider_HC"))
        self.horizontalLayout_HC.addWidget(self.horizontalSlider_HC)
        self.verticalLayout.addLayout(self.horizontalLayout_HC)
        self.label_2 = QtGui.QLabel(self.layoutWidget)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.verticalLayout.addWidget(self.label_2)
        self.horizontalLayout_SC = QtGui.QHBoxLayout()
        self.horizontalLayout_SC.setObjectName(_fromUtf8("horizontalLayout_SC"))
        self.spinBox_SC = QtGui.QSpinBox(self.layoutWidget)
        self.spinBox_SC.setMaximum(255)
        self.spinBox_SC.setObjectName(_fromUtf8("spinBox_SC"))
        self.horizontalLayout_SC.addWidget(self.spinBox_SC)
        self.horizontalSlider_SC = QtGui.QSlider(self.layoutWidget)
        self.horizontalSlider_SC.setMaximum(255)
        self.horizontalSlider_SC.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalSlider_SC.setObjectName(_fromUtf8("horizontalSlider_SC"))
        self.horizontalLayout_SC.addWidget(self.horizontalSlider_SC)
        self.verticalLayout.addLayout(self.horizontalLayout_SC)
        self.label_3 = QtGui.QLabel(self.layoutWidget)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.verticalLayout.addWidget(self.label_3)
        self.horizontalLayout_VC = QtGui.QHBoxLayout()
        self.horizontalLayout_VC.setObjectName(_fromUtf8("horizontalLayout_VC"))
        self.spinBox_VC = QtGui.QSpinBox(self.layoutWidget)
        self.spinBox_VC.setMaximum(255)
        self.spinBox_VC.setObjectName(_fromUtf8("spinBox_VC"))
        self.horizontalLayout_VC.addWidget(self.spinBox_VC)
        self.horizontalSlider_VC = QtGui.QSlider(self.layoutWidget)
        self.horizontalSlider_VC.setMaximum(255)
        self.horizontalSlider_VC.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalSlider_VC.setObjectName(_fromUtf8("horizontalSlider_VC"))
        self.horizontalLayout_VC.addWidget(self.horizontalSlider_VC)
        self.verticalLayout.addLayout(self.horizontalLayout_VC)
        self.label_4 = QtGui.QLabel(self.layoutWidget)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.verticalLayout.addWidget(self.label_4)
        self.horizontalLayout_HS = QtGui.QHBoxLayout()
        self.horizontalLayout_HS.setObjectName(_fromUtf8("horizontalLayout_HS"))
        self.spinBox_HS = QtGui.QSpinBox(self.layoutWidget)
        self.spinBox_HS.setMaximum(255)
        self.spinBox_HS.setObjectName(_fromUtf8("spinBox_HS"))
        self.horizontalLayout_HS.addWidget(self.spinBox_HS)
        self.horizontalSlider_HS = QtGui.QSlider(self.layoutWidget)
        self.horizontalSlider_HS.setMaximum(255)
        self.horizontalSlider_HS.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalSlider_HS.setObjectName(_fromUtf8("horizontalSlider_HS"))
        self.horizontalLayout_HS.addWidget(self.horizontalSlider_HS)
        self.verticalLayout.addLayout(self.horizontalLayout_HS)
        self.label_5 = QtGui.QLabel(self.layoutWidget)
        self.label_5.setObjectName(_fromUtf8("label_5"))
        self.verticalLayout.addWidget(self.label_5)
        self.horizontalLayout_SS = QtGui.QHBoxLayout()
        self.horizontalLayout_SS.setObjectName(_fromUtf8("horizontalLayout_SS"))
        self.spinBox_SS = QtGui.QSpinBox(self.layoutWidget)
        self.spinBox_SS.setMaximum(255)
        self.spinBox_SS.setObjectName(_fromUtf8("spinBox_SS"))
        self.horizontalLayout_SS.addWidget(self.spinBox_SS)
        self.horizontalSlider_SS = QtGui.QSlider(self.layoutWidget)
        self.horizontalSlider_SS.setMaximum(255)
        self.horizontalSlider_SS.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalSlider_SS.setObjectName(_fromUtf8("horizontalSlider_SS"))
        self.horizontalLayout_SS.addWidget(self.horizontalSlider_SS)
        self.verticalLayout.addLayout(self.horizontalLayout_SS)
        self.label_6 = QtGui.QLabel(self.layoutWidget)
        self.label_6.setObjectName(_fromUtf8("label_6"))
        self.verticalLayout.addWidget(self.label_6)
        self.horizontalLayout_VS = QtGui.QHBoxLayout()
        self.horizontalLayout_VS.setObjectName(_fromUtf8("horizontalLayout_VS"))
        self.spinBox_VS = QtGui.QSpinBox(self.layoutWidget)
        self.spinBox_VS.setMaximum(255)
        self.spinBox_VS.setObjectName(_fromUtf8("spinBox_VS"))
        self.horizontalLayout_VS.addWidget(self.spinBox_VS)
        self.horizontalSlider_VS = QtGui.QSlider(self.layoutWidget)
        self.horizontalSlider_VS.setMaximum(255)
        self.horizontalSlider_VS.setOrientation(QtCore.Qt.Horizontal)
        self.horizontalSlider_VS.setObjectName(_fromUtf8("horizontalSlider_VS"))
        self.horizontalLayout_VS.addWidget(self.horizontalSlider_VS)
        self.verticalLayout.addLayout(self.horizontalLayout_VS)
        self.label_7 = QtGui.QLabel(self.layoutWidget)
        self.label_7.setObjectName(_fromUtf8("label_7"))
        self.verticalLayout.addWidget(self.label_7)
        self.horizontalLayout_Targets = QtGui.QHBoxLayout()
        self.horizontalLayout_Targets.setObjectName(_fromUtf8("horizontalLayout_Targets"))
        self.radioButton_T1 = QtGui.QRadioButton(self.layoutWidget)
        self.radioButton_T1.setObjectName(_fromUtf8("radioButton_T1"))
        self.horizontalLayout_Targets.addWidget(self.radioButton_T1)
        self.radioButton_T2 = QtGui.QRadioButton(self.layoutWidget)
        self.radioButton_T2.setObjectName(_fromUtf8("radioButton_T2"))
        self.horizontalLayout_Targets.addWidget(self.radioButton_T2)
        self.verticalLayout.addLayout(self.horizontalLayout_Targets)
        self.horizontalLayout_Masks = QtGui.QHBoxLayout()
        self.horizontalLayout_Masks.setObjectName(_fromUtf8("horizontalLayout_Masks"))
        self.checkBox_Mask = QtGui.QCheckBox(self.layoutWidget)
        self.checkBox_Mask.setObjectName(_fromUtf8("checkBox_Mask"))
        self.horizontalLayout_Masks.addWidget(self.checkBox_Mask)
        self.checkBox_Masked = QtGui.QCheckBox(self.layoutWidget)
        self.checkBox_Masked.setObjectName(_fromUtf8("checkBox_Masked"))
        self.horizontalLayout_Masks.addWidget(self.checkBox_Masked)
        self.verticalLayout.addLayout(self.horizontalLayout_Masks)
        MainWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)
        ######################
        self.show()
        ####################

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(_translate("MainWindow", "Track Colors", None))
        self.pushButton_Start.setText(_translate("MainWindow", "Start", None))
        self.pushButton_Stop.setText(_translate("MainWindow", "Stop", None))
        self.label.setText(_translate("MainWindow", "H Center", None))
        self.label_2.setText(_translate("MainWindow", "S Center", None))
        self.label_3.setText(_translate("MainWindow", "V Center", None))
        self.label_4.setText(_translate("MainWindow", "H Span", None))
        self.label_5.setText(_translate("MainWindow", "S Span", None))
        self.label_6.setText(_translate("MainWindow", "V Span", None))
        self.label_7.setText(_translate("MainWindow", "HSV Settings for:", None))
        self.radioButton_T1.setText(_translate("MainWindow", "Target 1", None))
        self.radioButton_T2.setText(_translate("MainWindow", "Target 2", None))
        self.checkBox_Mask.setText(_translate("MainWindow", "Show Mask", None))
        self.checkBox_Masked.setText(_translate("MainWindow", "Show Masked", None))

########################## ADDED BY ROBERT ##################################
def main():
	app = QApplication(sys.argv)	#create application object
	ex = Ui_MainWindow()
	sys.exit(app.exec_())				#close the app on exit button press
	
if __name__ == '__main__':				#run main if calling this script directly
    main()
