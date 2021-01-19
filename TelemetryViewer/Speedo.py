import sys
from PyQt5 import QtWidgets, QtCore,QtGui


# GUI File
from PyQt5.QtWidgets import QMenu

from Speedoroot import Ui_Form

Speed = 60


class splashScreen(QtWidgets.QWidget):
    def __init__(self):
        QtWidgets.QWidget.__init__(self)
        self.ui = Ui_Form()
        self.ui.setupUi(self)
        self.animate(0)

        self.frame_size = self.ui.frame

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.accellerate)
        self.timer.start(15)

        self.type = 'dial'

        self.show()
    def accellerate (self):
        global Speed
        value=Speed

        self.animate(value)
        if Speed > 150:
            self.timer.stop()

            self.close()
        Speed += 0

    def contextMenuEvent(self, event):
        menu = QMenu(self)
        newAct = menu.addAction("Test")
        quitAct= menu.addAction("Quit")
        action= menu.popup(self.mapToGlobal(event.pos()))
        if action == quitAct:
            self.close()

    def animate(self,value):
        global Speed
        value=int(Speed)
        htmlText= "{Value} kph"
        newHtml=htmlText.replace("{Value}",str(value))
        self.ui.label_3.setText(newHtml)

        t = QtGui.QTransform()
        # -222 and 45
        if value>120:
            value=120
        if value<0:
            value=0
        value=reMap(Speed,120, 0,239,0)
        t.rotate(value)
        # load your image
        image = QtGui.QImage("QT Images/needle3.png")
        pixmap = QtGui.QPixmap.fromImage(image)
        # rotate the pixmap
        rotated_pixmap = pixmap.transformed(t)
        self.ui.Needle.setPixmap(rotated_pixmap)


def reMap(value, maxInput, minInput, maxOutput, minOutput):
    value = maxInput if value > maxInput else value
    value = minInput if value < minInput else value

    inputSpan = maxInput - minInput
    outputSpan = maxOutput - minOutput

    scaledThrust = float(value - minInput) / float(inputSpan)

    return minOutput + (scaledThrust * outputSpan)


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    window = splashScreen()
    sys.exit(app.exec_())
