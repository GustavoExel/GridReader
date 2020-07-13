import numpy as np
import subprocess, os, sys
from PyEFVLib.simulation.Saver import Saver

class CsvSaver(Saver):
	def __init__(self, grid, outputPath, basePath): 
		Saver.__init__(self, grid, outputPath, basePath, 'csv')

	def finalize(self):
		"""
		This saves the results organized in columns, being the columns respectively:
		Index, X, Y, Z, TimeStep1 - Field1, TimeStep1 - Field2, ... , TimeStep2 - Field1, TimeStep2 - Field2, ...
		The first line is reserved to the titles of the columns, as shown above.
		The second line, below the fields is reserved to the TimeStep data
		"""
		# indexes = np.arange( 1, self.grid.vertices.size + 1 )
		X,Y,Z = zip(*[v.getCoordinates() for v in self.grid.vertices])
		X,Y,Z = np.array(X), np.array(Y), np.array(Z) 
		labels = ["X", "Y", "Z"]
		timeData = ["", "", ""]

		fieldData = np.array([ self.fields[fieldName][tsIdx] for tsIdx in range( self.timeSteps.size ) for fieldName in self.fields.keys() ])
		labels += [ f"TimeStep{tsIdx} - {fieldName}" for tsIdx in range( self.timeSteps.size ) for fieldName in self.fields.keys() ]
		timeData += [ timeStepSize for timeStepSize in self.timeSteps for fieldIdx in range( len(self.fields) ) ]

		data = np.array([X, Y, Z, *fieldData]).T

		with open(self.outputPath, "w") as f:
			f.write("\"{}\"\n".format( "\", \"".join(labels) ))
			# f.write("{}\n".format( ", ".join([str(n) for n in timeData]) ))
			for line in data:
				f.write("{}\n".format( ", ".join([str(n) for n in line]) ))

		self.finalize = True