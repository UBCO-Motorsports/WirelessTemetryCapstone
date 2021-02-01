import pyqtgraph as pg
import numpy as np
from PyQt5 import QtWidgets, QtCore, QtGui
import sys
from RPM import RPMGauge
from Speedo import splashScreen

from PyQt5.QtWidgets import QSplitter
from Serial import SerialModule     #dont comment or delete > needed for Serial communication


class GraphManager(QtGui.QWidget):

    def __init__(self, parentWidget):
        super(GraphManager, self).__init__()
        self.parentWidget = parentWidget
        self.SerialModule = SerialModule()
        # self.SerialModule.connectSerial()

        self.graph_layout = QtGui.QGridLayout()
        self.setLayout(self.graph_layout)

        self.r = 255
        self.g = 0
        self.b = 0
        self.pen = pg.mkPen(color=(self.r,self.g,self.b),width=2)

        # Generate test data
        self.x = [i for i in range(200)]
        self.y = [i for i in range(200)]
        self.z = [-i for i in range(200)]

        # Generates array of graphs and puts them in a layout
        # [Row][Column] to align with QGridLayout
        self.graph_array = [[],[],[]]
        for i in range(3):
            for j in range(4):
                self.graph_array[i].append(PlotWdgt(self))
                self.graph_layout.addWidget(self.graph_array[i][j], i, j)
                self.graph_array[i][j].showGrid(x=True, y=True)
                self.graph_array[i][j].setBackground('w')

                self.graph_array[i][j].xData = self.x
                self.graph_array[i][j].yData = self.SerialModule.array1

                plotItem = self.graph_array[i][j].getPlotItem()
                plotItem.setLabel('bottom', text=self.graph_array[i][j].xLabel)
                plotItem.setLabel('left', text=self.graph_array[i][j].yLabel)



        # self.graph_array[0][0].xData.append(self.x)
        # self.graph_array[0][0].yData.append(self.SerialModule.array1)
        #
        # self.dial = RPMGauge(self)
        # self.graph_array[0][0].hide()
        # self.graph_array[0][0] = self.dial
        # self.graph_layout.addWidget(self.dial, 0, 0)
        # self.dial.dial_size.setGeometry(-10,-10,320,320)

        # self.speed = splashScreen()
        # self.graph_layout.removeWidget(self.graph_array[0][1])
        # self.graph_array[0][1].close()
        # self.graph_array[0][1] = self.speed
        # self.graph_layout.addWidget(self.speed, 0, 1)
        # self.speed.frame_size.setGeometry(-10,-10,320,320)

    def configMenuCalled(self, plotWidget):
        self.parentWidget.configMenuCalled(plotWidget)

    def update(self):
        #TODO
        # Call update for element in graph_array, if graph_array[i][j] == [polar]: update_polar()
        # Graph -> Update Graph, Dial -> Update Dial, Polar -> Update Polar

        # Update test data
        del self.x[0]  # Remove the first x element.
        self.x.append(self.x[-1] + 1)  # Add a new value 1 higher than the last.
        del self.y[0]
        self.y.append(self.y[-1] + 1)
        del self.z[0]
        self.z.append(self.z[-1] - 1)

        # Read latest data if serial is connected
        if self.parentWidget.serialConnected:
            self.SerialModule.readSerial()

            # Gets a sample of the latest arrays
            self.serialArrays = self.SerialModule.getData()

            # Iterates through each graph/dial and refreshes its data
            for i, row in enumerate(self.graph_array):
                for graph in row:
                    if graph.type == 'time_domain':
                        graph.clear()
                        graph.plot(graph.xData, self.serialArrays[i], pen=self.pen, clear=True)
                        # print('cartesian')
                    elif graph.type == 'dial':
                        pass
                        # print('dial')
                    elif graph.type == 'polar':
                        pass
                        # print('polar')


    def showGraphs(self, num_shown):
        if num_shown == '12':
            for row in range(len(self.graph_array)):
                for column in range(len(self.graph_array[row])):
                    self.graph_array[row][column].show()
        elif num_shown == '8':
            self.showGraphs('12')
            for column in range(len(self.graph_array[2])):
                self.graph_array[2][column].hide()
        elif num_shown == '6':
            self.showGraphs('8')
            for row in range(2):
                self.graph_array[row][3].hide()
        elif num_shown == '4':
            self.showGraphs('6')
            for row in range(2):
                self.graph_array[row][2].hide()
        elif num_shown == '2':
            self.showGraphs('4')
            for row in range(2):
                self.graph_array[row][1].hide()
        elif num_shown == '1':
            self.showGraphs('2')
            self.graph_array[1][0].hide()

class PlotWdgt(pg.PlotWidget):
    def __init__(self, parentWidget, parent=None):
        super(PlotWdgt, self).__init__(parent, viewBox=CustomViewBox(self))
        self.setSizePolicy(QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Ignored, QtWidgets.QSizePolicy.Ignored))
        self.parentWidget = parentWidget
        self.getPlotItem().getAxis('top').setStyle(showValues=False)


        self.type = 'time_domain'
        self.xData = []
        self.yData = []
        self.xLabel = 'X-Axis'
        self.yLabel = 'Y-Axis'
        self.title = ''
        self.yRange = 10
        self.autoRange = True
        #TODO store the current axis labels, legend, and datasets to populate config menu

    def configMenuCalled(self):
        # Calls parent widget to open edit menu
        self.parentWidget.configMenuCalled(self)

class CustomViewBox(pg.ViewBox):
    def __init__(self, parentWidget, parent=None):
        super(CustomViewBox, self).__init__(parent)
        self.menu = pg.ViewBoxMenu.ViewBoxMenu(self)
        self.parentWidget = parentWidget

        # Adds edit option to right click menu
        self.menu.addSeparator()
        self.editData = QtGui.QAction("Edit Data", self.menu)
        self.editData.triggered.connect(self.parentWidget.configMenuCalled)
        self.menu.addAction(self.editData)


# app = QtWidgets.QApplication(sys.argv)
# test = GraphManager()
# test.show()
# app.setAttribute(QtCore.Qt.AA_Use96Dpi)
# sys.exit(app.exec_())