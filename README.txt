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
