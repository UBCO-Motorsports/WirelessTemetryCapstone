import sys
from PyQt5 import QtWidgets, QtCore,QtGui


# GUI File
from PyQt5.QtWidgets import QMenu
from PyQt5.QtCore import Qt
from Speedoroot import Ui_Form

newneedle=0


class SpeedoGauge(QtWidgets.QWidget):
    raisesignal = QtCore.pyqtSignal()
    def __init__(self, parent):
        # QtWidgets.QWidget.__init__(self)
        super(SpeedoGauge, self).__init__(parent)
        self.parentwidget = parent
        self.ui = Ui_Form()
        self.ui.setupUi(self)
        self.Speed = 0
        self.animate(0)
        self.highlighted = False

        self.frame_size = self.ui.frame

        self.type = 'Speedo Gauge'
        self.yData = []

        self.show()

    # Don't delete! needed to run GUITest
    def plot(self, x, y):
        # Needed to run GUITest
        pass

    def accellerate (self):
        value=self.Speed

        self.animate(value)
        if self.Speed > 150:
            # self.timer.stop()
            #
            self.close()
        self.Speed += 0

    def contextMenuEvent(self, event):
        menu = QMenu(self)
        menu.setStyleSheet('QMenu {background-color: rgb(240, 240, 240); color: rgb(0, 0, 0);} QMenu:selected{background-color: rgb(144, 200 , 246)}')
        newAct = menu.addAction("Edit Data")
        newAct.triggered.connect(lambda: self.parentwidget.configMenuCalled(self))
        action= menu.popup(self.mapToGlobal(event.pos()))

    def animate(self,value):
        global newneedle
        value=int(value)
        self.Speed=value
        htmlText= "{Value} kph"
        newHtml=htmlText.replace("{Value}",str(value))
        self.ui.label_3.setText(newHtml)

        t = QtGui.QTransform()
        # -222 and 45
        if value>120:
            value=120
        if value<0:
            value=0
        value=reMap(self.Speed,120, 0,239,0)
        t.rotate(value)
        if newneedle is not 0:
            pixmap=newneedle
        else:

            # load your image
            image=QtGui.QImage("QTImages/needle3.png")
            pixmap = QtGui.QPixmap.fromImage(image)
        # rotate the pixmap
        rotated_pixmap = pixmap.transformed(t)
        self.ui.Needle.setPixmap(rotated_pixmap)
    def resizeEvent(self, a0: QtGui.QResizeEvent):
        global newneedle
        height=self.height()
        width=self.width()
        if height>width:
            length=width
        else:
            length=height
        self.ui.frame.setGeometry(QtCore.QRect(0, 0, length, length))
        self.ui.frame_2.setGeometry(QtCore.QRect(0, 0, length, length))
        self.ui.frame_3.setGeometry(QtCore.QRect(0,0,length,length))
        bcircle= """QFrame{
                        border-radius: {Value};
                        background-color: rgb(247,247,247);
                        }"""
        bcircle=bcircle.replace("{Value}", (str(int(int(length)/2))))
        self.ui.frame_3.setStyleSheet(bcircle)
        self.ui.frame_4.setGeometry(QtCore.QRect(5,5,length-10,length-10))
        scircle = """QFrame{
                                border-radius: {Value};
                                background-color: rgb(0,0,0);
                                }"""
        scircle = scircle.replace("{Value}", (str(int(int(length-10) / 2))))
        self.ui.frame_4.setStyleSheet(scircle)

        lines=QtGui.QPixmap("QTImages/speedolines2.png")
        lines=lines.scaled(length+30, length+30, Qt.KeepAspectRatio,Qt.FastTransformation)
        self.ui.label.setGeometry(-15,-15,length+30,length+30)
        self.ui.label.setPixmap(lines)

        numbers = QtGui.QPixmap("QTImages/Speedometer2.png")
        numbers = numbers.scaled(length, length, Qt.KeepAspectRatio, Qt.FastTransformation)
        self.ui.label_2.setGeometry(0, -length/10, length, length)
        self.ui.label_2.setPixmap(numbers)

        pointer = QtGui.QPixmap("QTImages/Pointer2.png")
        pointer = pointer.scaled(length, length, Qt.KeepAspectRatio, Qt.FastTransformation)
        self.ui.label_4.setGeometry(0, 5, length, length)
        self.ui.label_4.setPixmap(pointer)

        self.ui.label_3.setGeometry(0,length/3.25,length,length)
        self.ui.label_3.setAlignment(QtCore.Qt.AlignCenter)
        self.ui.label_3.setFont(QtGui.QFont('Bahnschrift SemiCondensed',length/15))

        self.ui.Needle.setGeometry(0,0,length,length)
        needle= QtGui.QPixmap("QTImages/needle3.png")
        needle=needle.scaled(length,length,Qt.KeepAspectRatio, Qt.FastTransformation)
        newneedle=needle

        if self.highlighted:
            self.ui.frame_3.setStyleSheet('border-radius: 150px;' 'border: 3px solid #00ff00;')

        self.ui.frame_2.raise_()
        self.ui.frame_3.raise_()
        self.ui.frame_4.raise_()
        self.ui.label.raise_()
        self.ui.label_2.raise_()
        self.ui.Needle.raise_()
        self.ui.label_4.raise_()
        self.ui.label_3.raise_()

        self.raisesignal.emit()


def reMap(value, maxInput, minInput, maxOutput, minOutput):
    value = maxInput if value > maxInput else value
    value = minInput if value < minInput else value

    inputSpan = maxInput - minInput
    outputSpan = maxOutput - minOutput

    scaledThrust = float(value - minInput) / float(inputSpan)

    return minOutput + (scaledThrust * outputSpan)


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    mainwindow = QtWidgets.QMainWindow()
    window = splashScreen()
    mainwindow.setCentralWidget(window)
    mainwindow.show()
    sys.exit(app.exec_())
