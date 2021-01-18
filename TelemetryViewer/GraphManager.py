import pyqtgraph as pg
import numpy as np
from PyQt5 import QtWidgets, QtCore, QtGui
import sys
from RPM import SplashScreen
from Speedo import splashScreen

from PyQt5.QtWidgets import QSplitter


class GraphManager(QtGui.QWidget):

    def __init__(self):
        super(GraphManager, self).__init__()

        self.graph_layout = QtGui.QGridLayout()
        self.setLayout(self.graph_layout)

        self.pen = pg.mkPen(color=(255,0,0),width=2)

        # Generate test data
        self.x = [i for i in range(200)]
        self.y = [i for i in range(200)]
        self.z = [-i for i in range(200)]

        # Generates array of graphs and puts them in a layout
        # [Row][Column] to align with QGridLayout
        self.graph_array = [[],[],[]]
        for i in range(3):
            for j in range(4):
                self.graph_array[i].append(PlotWdgt())
                self.graph_layout.addWidget(self.graph_array[i][j], i, j)
                self.graph_array[i][j].showGrid(x=True, y=True)
                self.graph_array[i][j].setBackground('w')

                self.graph_array[i][j].data["Y"] = [self.x, self.y]

        # self.dial = SplashScreen()
        # self.graph_array[0][0].close()
        # self.graph_array[0][0] = self.dial
        # self.graph_layout.addWidget(self.dial, 0, 0)
        # self.dial.dial_size.setGeometry(-10,-10,320,320)

        # self.speed = splashScreen()
        # self.graph_layout.removeWidget(self.graph_array[0][1])
        # self.graph_array[0][1].close()
        # self.graph_array[0][1] = self.speed
        # self.graph_layout.addWidget(self.speed, 0, 1)
        # self.speed.frame_size.setGeometry(-10,-10,320,320)



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


        for row in self.graph_array:
            for graph in row:
                if graph.type == 'xy_graph':
                    for data_type in graph.data:
                        graph.plot(graph.data[data_type][0], graph.data[data_type][1], pen=self.pen, clear=True)
                        # print('cartesian')
                elif graph.type == 'dial':
                    return
                    # print('dial')
                elif graph.type == 'polar':
                    return
                    # print('polar')


    #TODO
    # Rewrite this function in a better way
    #
    def showGraphs(self, num_shown):
        if num_shown == '12':
            for row in range(len(self.graph_array)):
                for column in range(len(self.graph_array[row])):
                    self.graph_array[row][column].show()
                    print(str(row) + ' ' + str(column))

        elif num_shown == '8':
            for row in range(2):
                for column in range(4):
                    self.graph_array[row][column].show()
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


    #     # Reset formats to better align widgets
    #     for i in range(4):
    #         self.graph_layout.setColumnStretch(i, 0)
    #         self.graph_layout.setRowStretch(i, 0)
    #
    #     if num_shown == '1':
    #         for i in range(1, 3):
    #             self.graph_array[0][i].hide()
    #         for i in range(1, 3):
    #             for j in range(3):
    #                 self.graph_array[i][j].hide()
    #
    #     elif num_shown == '2':
    #         self.graph_array[1][0].show()
    #         for i in range(2):
    #             for j in range(1, 3):
    #                 self.graph_array[i][j].hide()
    #         for i in range(2, 3):
    #             for j in range(3):
    #                 self.graph_array[i][j].hide()
    #         self.graph_array[0][0].resize(self.width(), self.height()/2)
    #         self.graph_array[1][0].resize(self.width(), self.height()/2)
    #
    #     elif num_shown == '4':
    #         for i in range(2):
    #             for j in range(2):
    #                 self.graph_array[i][j].show()
    #         for i in range(2):
    #             for j in range(2, 3):
    #                 self.graph_array[i][j].hide()
    #         for i in range(2, 3):
    #             for j in range(3):
    #                 self.graph_array[i][j].hide()
    #
    #         for i in range(2):
    #             for j in range(2):
    #                 self.graph_array[i][j].setGeometry(j * self.width() / 2, i * self.height() / 2, self.width() / 2, self.height() / 2)
    #                 print(self.graph_array[i][j].frameGeometry())
    #
    #
    #     elif num_shown == '8':
    #         for i in range(2,3):
    #             for j in range(3):
    #                 self.graph_array[i][j].hide()
    #         for i in range(2):
    #             for j in range(3):
    #                 self.graph_array[i][j].show()
    #
    #         for i in range(2):
    #             for j in range(3):
    #                 self.graph_array[i][j].setGeometry(j * self.width()/4, i * self.height()/2, self.width()/4, self.height()/2)
    #
    #     elif num_shown == '12':
    #         for i in range(3):
    #             # Reformats layout of widgets
    #             self.graph_layout.setColumnStretch(i, 1)
    #             self.graph_layout.setRowStretch(i, 1)
    #             for j in range(4):
    #                 self.graph_array[i][j].show()

#TODO
class PlotWdgt(pg.PlotWidget):
    def __init__(self, parent=None):
        super(PlotWdgt, self).__init__(parent, viewBox=CustomViewBox(self))

        self.type = 'xy_graph'
        self.data = {}


class CustomViewBox(pg.ViewBox):
    def __init__(self, parentWidget, parent=None):
        super(CustomViewBox, self).__init__(parent)
        self.menu = pg.ViewBoxMenu.ViewBoxMenu(self)
        self.parentWidget = parentWidget

        # self.menuUpdate = True #Don't think this is needed

        self.menu.addSeparator()
        self.editData = QtGui.QAction("Edit Data", self.menu)
        self.editData.triggered.connect(lambda: self.parentWidget.hide()) #TODO Maybe return 'self' to link to data selection panel
        self.menu.addAction(self.editData)

        # self.menuUpdate = False #Don't think this is needed


# app = QtWidgets.QApplication(sys.argv)
# test = GraphManager()
# test.show()
# app.setAttribute(QtCore.Qt.AA_Use96Dpi)
# sys.exit(app.exec_())