TODO:
	- performance improvement
		- based on the ideas below we could add Rendering Parameter to the Shape object for different processing
		- the scanline algorithm without depth buffer could be used to realize HUD or UI Interfaces
	    - For 2D / 3D Wireframe there could be used the Bresham algorithm
			- 2D we are ignoring depth buffer only examining rasterpixel
			- 3D we are using bresham to identify pixel for processing and 
			  then going through regular rendering
		- instead of min max calculation use scanline algorithm.
			- on lines only took intersection point +- 1 with min max of raster plate
				- exclide n*m - (ca.)sqrt(n*n + m*m) - O(intersection func)
			- on triangles calc two intersection point and sort them begin on minx - 1 up to maxx + 1
				- exclude half of pixel*vpixel - O(intersection func)
	- create diagramm API
		- create axis a scene
		- create label vector as scene for easier extraction
	- simple polygon triangulation, based on self made algorithm
		- testing if this could work with simple font polygons
	- adding font rendering:
		- build in basic selfmade font
		- loading true type fonts (it would be nice not using freetype lib)

ISSUES:

- background color rendering is broken.



Rendering Strategy:

1. This API uses COLUMN MAJOR Matrix:
	
		V(Vector) = |x|
					|y|
					|z|
		
		The colums of the matrix represent the axis of a coordinate system.
		Column 1 = XA(X-Axis)
		Column 2 = YA(Y-Axis)
		Column 3 = ZA(Z-Axis)
		
		Coordinate Matrix
		M(Matrix) = | XAx YAx ZAx 0 |
					| XAy YAy ZAy 0 |
					| XAz YAz ZAz 0 |
					| 0   0   0   1 |
		
		Translation Matrix
		T(Matrix) = | 1   0   0   Tx |
					| 0   1   0   Ty |
					| 0   0   1   Tz |
					| 0   0   0   1  |
		
		Scaling Matrix
		S(Matrix) = | Sx  0   0   0 |
					| 0   Sy  0   0 |
					| 0   0   Sz  0 |
					| 0   0   0   1 |
		
		Combined TM Matrix
		TM(Matrix) = | XAx YAx ZAx Tx |
					 | XAy YAy ZAy Ty |
					 | XAz YAz ZAz Tz |
					 | 0   0   0   1  |
	
		Transforming (P)Point => Vector Matrix Multiplication intp => (P')TransformedPoint
		
2. Transforming a Point or Vector must be applied in this case:
	Fact: S=Scaling, R=Rotation(xyz), T=Translation, v=Vector/Point, v'=transformed point.
	You NEED to do transformation in following sequence:
		v' = S * R * T * v
		v' = (Rx)*Ry*Rz*T*v   (Rotation Order should be X then Y then Z)
