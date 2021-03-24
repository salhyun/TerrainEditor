#pragma once

#include "SecretMath.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Vector4.h"
#include "Matrix.h"
#include "Quaternion.h"

namespace geo
{
	enum SOLID_KIND { SPHERE=0, AACUBE, CUBE, CYLINDER} ;

	enum INTERSECT {INTERSECT_INFINITE=0, INTERSECT_NONE, INTERSECT_POINT} ;

	struct SPixel
	{
		int x, y ;
		DWORD color ;
	} ;

	struct SPixelCircle
	{
		int cx, cy ;
		int nRadius ;
	} ;

	struct SPoint
	{
		float x, y ;

		SPoint() {x=y=0;}
		SPoint(float _x, float _y) {x=_x; y=_y;}
		void set(float _x, float _y) {x=_x; y=_y;}
	} ;

	struct SPolyhedron
	{
		int nKind ;
	} ;

	struct SSphere : public SPolyhedron
	{
		Vector3 vPos ;
		float fRadius ;
		DWORD dwAttr ;

		void set(Vector3 &p, float r) {vPos=p; fRadius=r; nKind=SPHERE; dwAttr=0;}
	} ;

	struct SRect
	{
		float left, right, top, bottom ;

		SRect() {left=right=top=bottom=0;}
		SRect(float _left, float _right, float _top, float _bottom) {left=_left; right=_right; top=_top; bottom=_bottom;}
		void set(float _left, float _right, float _top, float _bottom) {left=_left; right=_right; top=_top; bottom=_bottom;}
	} ;

	struct STriangleWeight
	{
		float afweight[3] ;

		STriangleWeight() {afweight[0]=afweight[1]=afweight[2]=0;}
		STriangleWeight(float w0, float w1, float w2) {afweight[0]=w0; afweight[1]=w1; afweight[2]=w2;}
		void set(float w0, float w1, float w2) {afweight[0]=w0; afweight[1]=w1; afweight[2]=w2;}
	} ;

	struct SLine
	{
		Vector3 s, v ;//s는 출발점, v는 방향
		float d ;//d는 거리

		SLine() ;
		SLine(Vector3 _s, Vector3 _v, float _d) ;
		void set(Vector3 _s, Vector3 _v, float _d) ;
		Vector3 GetEndPos() ;
		Vector3 GetPos(float t) ;//t=[0, 1]
		float GetDist(Vector3 &pos) ;
	} ;

	struct SVertex
	{
		Vector3 vPos ;
		Vector3 vNormal ;
		Vector2 vTex ;
		int nPosDecision ;

		SVertex() ;
		SVertex(Vector3 _vPos, Vector3 _vNormal, Vector2 _vTex) ;
		void set(Vector3 _vPos, Vector3 _vNormal, Vector2 _vTex) ;
	} ;

	struct SPlane
	{
		enum {SPLANE_INFRONT=0, SPLANE_BEHIND, SPLANE_COINCLIDING, SPLANE_INTERSECT} ;

		float a, b, c, d ;

		SPlane() ;
		SPlane(float _a, float _b, float _c, float _d) ;
		SPlane(Vector3 &vPos, Vector3 &vNormal) ;
		void set(float _a, float _b, float _c, float _d) ;
		void set(Vector3 &vPos, Vector3 &vNormal) ;
		bool MakePlane(Vector3 &v0, Vector3 &v1, Vector3 &v2) ;//왼손법칙에 따라 세점을 가지고 평면을 만든다.
		int Classify(Vector3 &vPos) const ;//한점과 평면의 관계를 얻는다.
		float ToPosition(Vector3 &vPos) const ;//한점과 거리를 구한다. 음수:평면의 뒷면, 양수:평면의 앞면
		Vector3 GetNormal() const ;//평면의 노말값을 구한다.
		SPlane GetPlanebyTransform(Matrix4 *pmat) const ;
		Vector3 GetOnPos(Vector3 &vPos) ;
	} ;

	struct STriangle
	{
		//Vector3 avPos[3] ;
		//Vector3 avNormal[3] ;
		//Vector3 avTex[2] ;
		SVertex avVertex[3] ;
		SPlane sPlane ;
		SSphere sBoundingSphere ;
		bool bSelected ;//어떤 이벤트에 의해서 선택되어졌을경우 (일회성임)
		float dist ;//점과 거리비교할때 사용(일회성)
		DWORD dwAttr ;//각종 속성값을 부여(일회성)

		STriangle() ;
		STriangle(Vector3 &v0, Vector3 &v1, Vector3 &v2) ;
		void set(Vector3 &v0, Vector3 &v1, Vector3 &v2) ;
		void MakeBoundingShpere() ;
		void GetLine(int nNum, SLine *psLine) ;
		Vector3 GetNormalbyWeight(STriangleWeight *psWeight) ;
		Vector3 GetPosbyWeight(STriangleWeight *psWeight) ;
		Vector3 GetMidPos() ;
		Vector2 GetTexbyWeight(STriangleWeight *psWeight) ;
		SSphere *GetBoundingSphere() ;
	} ;

	struct STransform
	{
		Matrix4 matTrans, matRot, matScale, matTransform ;
        Quaternion OriginQuat, ResultQuat ;
		float fPitch, fYaw, fRoll ;

		STransform() { fPitch=fYaw=fRoll=0.0f; }

		void Reset() ;
        void Translate(float x, float y, float z, bool bAdd=true) ;
		void Rotate(float pitch, float yaw, float roll) ;
		void Scale(float x, float y, float z) ;
		Matrix4 *ComputeTransform() ;
		Matrix4 *GetTransform() ;
		void SetTransform(Matrix4 *pmatTransform) ;
	} ;

	struct SAACube : public SPolyhedron //Axis Aligned Cube
	{
		enum {SAACUBE_LLB, SAACUBE_LLT, SAACUBE_LRT, SAACUBE_LRB, SAACUBE_ULB, SAACUBE_ULT, SAACUBE_URT, SAACUBE_URB} ;
		enum {SAACUBE_LEFTFACE, SAACUBE_RIGHTFACE, SAACUBE_TOPFACE, SAACUBE_BOTTOMFACE, SAACUBE_FRONTFACE, SAACUBE_BACKFACE} ;

		Vector3 vCenter ;
		Vector3 vMin, vMax ;
		float fWidth, fHeight, fDepth, fRadius ;
		float fDiagonal ;//밑면에서 면의 중심과 꼭지점의 거리

		SAACube() {} ;
		SAACube(Vector3 &vMinPos, Vector3 &vMaxPos) ;
		SAACube(Vector3 &vCenterPos, float width, float height, float depth) ;

		void set(Vector3 &vMinPos, Vector3 &vMaxPos) ;
		void set(Vector3 &vCenterPos, float width, float height, float depth) ;
		Vector3 GetPos(int nPos) const ;
		SPlane GetFace(int nFace) const ;
	} ;

	struct SCube : public SPolyhedron //Oriented Cube
	{
		enum {SCUBE_LLB, SCUBE_LLT, SCUBE_LRT, SCUBE_LRB, SCUBE_ULB, SCUBE_ULT, SCUBE_URT, SCUBE_URB} ;
		enum {SCUBE_LEFTFACE, SCUBE_RIGHTFACE, SCUBE_TOPFACE, SCUBE_BOTTOMFACE, SCUBE_FRONTFACE, SCUBE_BACKFACE} ;

		Vector3 vCenter ;
		Vector3 vMin, vMax ;
		Vector3 vVertices[8] ;
		float fWidth, fHeight, fDepth, fRadius ;
		float fDiagonal ;//밑면에서 면의 중심과 꼭지점의 거리
		float fSmallestX, fLargestX, fSmallestY, fLargestY, fSmallestZ, fLargestZ ;

		SCube() ;
		//void set(Vector3 &vCenterPos, float width, float height, float depth, float pitch, float yaw, float roll) ;
		void set(Vector3 &center, Vector3 basis[3], Vector3 &min, Vector3 &max) ;
		void ComputeBoundary() ;
		SPlane GetFace(int nFace) ;

	private :
		Vector3 _GetPos(int nPos) ;
	} ;

	struct SCylinder : public SPolyhedron
	{
		Vector3 vStart, vEnd ;
		float fRadius ;
		SSphere sBoundingSphere ;
		DWORD dwAttr ;

		SCylinder() {} ;
		SCylinder(Vector3 start, Vector3 end, float radius) ;
		void set(Vector3 &start, Vector3 &end, float radius) ;
		int Intersect(geo::SLine &line, Vector3 &vIntersect, float precision=0.0001f) ;
	} ;

	struct SMovement
	{
		enum ACTION { ACTION_IDLE=0, ACTION_GO, ACTION_STAY, ACTION_BACK, ACTION_FINISH } ;
		enum ATTR { ATTR_LOOP=0x01, ATTR_REVERSE=0x02 } ;

		int nAttr ;
		int anActions[30], nActionNode, nNumAction ;
		int nCurAction ;
		Vector3 vStart, vEnd, vPos ;
		geo::SLine sPath ;
		float fDist, fTimeLimit, fTimeStay ;
		float fElapsedTime ;
		
		float (*pFunction)(float) ;

		float fTimeRatio, fDistRatio, fInvDistRatio ;

		void _NextAction() ;

		void set(Vector3 &start, Vector3 &end, float time_limit, float time_stay) ;
		void setActions(int *pnActions, int nNum) ;
		void setAction(int action) ;
		int getCurAction() ;
		void addAttr(int attr, bool enable=true) ;
		int getAttr() ;
		void process(float time) ;
		Vector3 getPos() ;
		void setFunction(float (*pfunc)(float)) ;
	} ;
}

bool IntesectXZProjTriangleToPoint(geo::STriangle &sTriangle, Vector3 vPos, float &u, float &v) ;//use HeightMap
void SolveLinearSystem(float *pfInputMatrix, int nNumColumn, int nNumRow, int nNumVariable, float *pfResult) ;//using Guass-Jordan Elimination
//bool TriangleIntersectLinearSystem(Vector3 &r, Vector3 &q1, Vector3 &q2, Vector3 &vBaryCenter, float precision=0.0001f) ;
bool TriangleIntersectLinearSystem(Vector3 &r, Vector3 &q1, Vector3 &q2, Vector3 &vBaryCenter, float precision=1.192092896e-07F) ;

int CloseToExp(float fInput, float fExponent, bool bReverse=false) ;

float IntersectLinetoPoint(geo::SLine &line, Vector3 &vPos, Vector3 *pvIntersected=NULL) ;

int IntersectLineToLine(geo::SLine *psLine1, geo::SLine *psLine2, Vector3 *pvPos) ;//일단 사용금지
float DistLinetoLine(geo::SLine *psLine1, geo::SLine *psLine2) ;

bool IntersectLinetoPlane(Vector3 &v1, Vector3 &v2, geo::SPlane &plane, Vector3 &vOut, float &t, float precision=0.0001f) ;
int IntersectLinetoPlane(geo::SLine &line, geo::SPlane &plane, Vector3 &vPos, bool bCullingTest=true, bool bDirectional=true, float precision=0.0001f) ;
int IntersectLinetoPlane(geo::SLine &line, geo::SPlane &plane, Vector3 &vPos, float &t, bool bCullingTest=true, float precision=0.0001f) ;

bool IsPointOnTriangle(Vector3 &vPos, geo::STriangle &sTriangle, Vector3 &vResult, float precision=0.0001f) ;
//bool IsPointOnTriangle(Vector3 &vPos, geo::STriangle &sTriangle, Vector3 &vResult, float precision=1.192092896e-07F) ;
int IntersectLinetoTriangle(geo::SLine &line, geo::STriangle &triangle, Vector3 &vBaryCenter, bool bDirectlyUse=false, bool bCullingTest=true, float precision=0.0001f) ;
//int IntersectLinetoTriangle(geo::SLine &line, geo::STriangle &triangle, Vector3 &vBaryCenter, bool bDirectlyUse=false, bool bCullingTest=true, float precision=1.192092896e-07F) ;
int TriangleIntersect(const geo::SLine *psLine, const geo::STriangle *psTriangle, Vector3 *pvResult, bool bUsable=false) ;

bool isInCube(Vector3 &pos, geo::SAACube *psCube) ;
bool IntersectLineToCube(geo::SLine *psLine, geo::SAACube *psCube) ;//Axis Aligned Cube to Line	
bool IntersectLineToCube(geo::SLine *psLine, geo::SCube *psCube) ;
int IntersectPlaneToCube(geo::SPlane *psPlane, geo::SAACube *psCube) ;
bool IntersectDiskToRay(Vector3 &v1, Vector3 &v2, Vector3 &vDiskCenter, Vector3 &vDiskNormal, float fRadius) ;
bool IntersectLineToSphere(geo::SLine *psLine, Vector3 vPos, float fRadius, float precision=0.0001f) ;
bool IntersectLineToSphere(geo::SLine *psLine, geo::SSphere *psSphere, Vector3 &vIntersect, float precision=0.0001f) ;
bool IntersectSphereToBox(geo::SSphere *psSphere, geo::SAACube *psCube) ;

int ComputeBigO(double length, double minsize, double treesystem) ;

Vector3 ReflectVector(Vector3 vIncident, Vector3 vNormal) ;
float IncludedAngle(Vector3 &v1, Vector3 &v2) ;
Vector3 GetMidPoint(Vector3 &v1, Vector3 &v2, Vector3 &v3) ;
float GetAreaOfTriangle(Vector3 v1, Vector3 v2, Vector3 v3) ;
Vector3 ProjectionVector(Vector3 &v1, Vector3 &v2, bool bRange=true) ;

Vector3 ComputeTangent(Vector3 &vt0, Vector3 &vt1, Vector3 &vt2) ;

Vector3 GetHalfVector(Vector3 &vNormal0, Vector3 &vNormal1) ;

Vector3 GetNewVectorfromTwoVector(Vector3 &v1, Vector3 &v2, float theta) ;//두벡터의 크로스벡터를 축으로 원하는 방향을 얻어낸다. 기준은 v1
Vector3 GetNewVectorfromPerpendicularVector(Vector3 &vAxis, Vector3 &vFrom, float theta) ;

Vector3 GetPosOnPlane(Vector3 &vPos, geo::SPlane &sPlane) ;
Vector3 SlidingPlane(geo::SLine *psLine, geo::SPlane *psPlane) ;//평면과 직선이 교차된경우 사용
Vector3 LayingPlane(geo::SLine *psLine, geo::SPlane *psPlane, geo::SLine *psResultLine=NULL) ;

Vector3 GetPosInXZPlane(Vector3 &p) ;

int Intersect2DLines(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float *x, float *y) ;//2D선분교차검사

void DrawCircle(geo::SPixel *psPixels) ;
int BresenhamLine(int x1, int y1, int x2, int y2, geo::SPixel *psPixelBuffer) ;//draw 2D line
float GaussianFunc(float x, float z, float area, float height) ;//원점을 중심으로 한 가우스분포함수 area : 표준편차(보통 0.6065) height : 최대값

float LinerInterpolation(float a, float b, float x) ;
float CosinInterpolation(float a, float b, float x) ;
float C2SCurveFunction(float t) ;
float SinDampingFunction(float t) ;

float WeightInterpolation(float a, float b, float interpolation) ;
void AxisAngleToEuler(Vector3 &axis, float theta, float &pitch, float &yaw, float &roll) ;
float calculateAngle(float x, float y) ;
void calculateVector(float theta, float &x, float &y) ;