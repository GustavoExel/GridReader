import numpy as np
from PyEFVLib.geometry.Point import Point

class InnerFace:
	def __init__(self, element, handle, local):
		self.handle = handle
		self.local = local
		self.element = element
		self.area = 0.0
		self.evalCentroid()
		self.calculateGlobalDerivatives()

	def evalCentroid(self):
		shapeFunctionValues = self.element.shape.innerFaceShapeFunctionValues[self.local]
		coords = np.array([0.0,0.0,0.0])
		for vertex, weight in zip(self.element.vertices, shapeFunctionValues):
			coords += weight * vertex.getCoordinates()
		self.centroid = Point(*coords)

	def calculateGlobalDerivatives(self):
		derivatives = self.element.shape.innerFaceShapeFunctionDerivatives[self.local]
		self.globalDerivatives = np.matmul(np.linalg.inv(self.element.getTransposeJacobian(derivatives)) , np.transpose(derivatives))