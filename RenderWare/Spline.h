#pragma once

#include "def.h"
#include "Vector3.h"
#include "Matrix.h"

#define MAXNUM_SPLINEDATA 100

struct SPLINEDATA
{
	ULONG nIndex ;
    Vector3 vPos ;
	Vector3 vNor ;
	float fDist ;
} ;

class CSpline
{
//variable
private :
	bool m_bEnable ;
	Matrix4 m_matHermite ;
	SPLINEDATA m_SplineData[MAXNUM_SPLINEDATA] ;

	float m_fMaxDist ;
	int m_nNodeCount, m_nNumNode ;
	int m_nIndexCount ;

public :
	

//function
private :
	void SetSpline(SPLINEDATA *pSpline, Vector3 vPos, Vector3 vNor) ;

public :
	CSpline() ;
	~CSpline() ;

	void Initialize(Vector3 vStart, Vector3 vNorStart, Vector3 vEnd, Vector3 vNorEnd) ;
	void Initialize(Vector3 vStart, Vector3 vNorStart) ;
    void AddNode(Vector3 vPos, Vector3 vNor) ;

	int GetCurrentNode() ;//현재 노드를 가리킨다. 그러니까 루프를 돌리려면 '<=' 이렇게 해주어야 함.
	SPLINEDATA *GetSplineData(int nNode) ;

	Vector3 GetPositionOnCubic(int nNode, float t) ;
	Vector3 GetTangentAtPosition(int nNode, float t, bool bNormalize=true) ;
	Vector3 GetAccelVectorAtPosition(int nNode, float t, bool bNormalize=true) ;//Acceleration Vector 가속도 벡터 r"(t)

	bool IsEnable() {return m_bEnable;}
	void SetEnable(bool enable=true) {m_bEnable=enable;}
	void ImportSplineData(SPLINEDATA *psSplineData, int nNum) ;
	int GetNumNode() {return m_nNumNode;}
} ;