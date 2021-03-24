#pragma once

#include "Vector3.h"

//-------------------------------------------------------------------------------
//-- Function declarations ------------------------------------------------------
//-------------------------------------------------------------------------------

void ComputeBoundingBox( const Vector3* points, int nPoints, 
		Vector3& centroid, Vector3 basis[3],
		Vector3& min, Vector3& max );

void ComputeBoundingCylinder( const Vector3* points, int nPoints, 
		Vector3& centroid, Vector3& axis,
		float& min, float& max, float& radius );