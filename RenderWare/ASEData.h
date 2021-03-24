#pragma once

#include <string.h>
#include <stdio.h>
#include <vector>
#include "Vector3.h"
#include "GeoLib.h"

struct SMeshMatrix
{
	float m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44 ;

	SMeshMatrix()
	{
		m11 = m22 = m33 = m44 = 1.f ;

		m12 = m13 = m14 = 0.f ;
		m21 = m23 = m24 = 0.f ;
		m31 = m32 = m34 = 0.f ;
		m41 = m42 = m43 = 0.f ;
	}
	void set(float _m11, float _m12, float _m13, float _m14, float _m21, float _m22, float _m23, float _m24,
		float _m31, float _m32, float _m33, float _m34, float _m41, float _m42, float _m43, float _m44)
	{
		m11 = _m11 ;	m12 = _m12 ;	m13 = _m13 ;	m14 = _m14 ;
		m21 = _m21 ;	m22 = _m22 ;	m23 = _m23 ;	m24 = _m24 ;
		m31 = _m31 ;	m32 = _m32 ;	m33 = _m33 ;	m34 = _m34 ;
		m41 = _m41 ;	m42 = _m42 ;	m43 = _m43 ;	m44 = _m44 ;
	}
	void set(SMeshMatrix &mat)
	{
		m11 = mat.m11 ;	m12 = mat.m12 ;	m13 = mat.m13 ;	m14 = mat.m14 ;
		m21 = mat.m21 ;	m22 = mat.m22 ;	m23 = mat.m23 ;	m24 = mat.m24 ;
		m31 = mat.m31 ;	m32 = mat.m32 ;	m33 = mat.m33 ;	m34 = mat.m34 ;
		m41 = mat.m41 ;	m42 = mat.m42 ;	m43 = mat.m43 ;	m44 = mat.m44 ;
	}
} ;
struct SMeshQuaternion
{
	float x, y, z, w ;

	SMeshQuaternion() {x=y=z=0.0f; w=1.0f;}
	SMeshQuaternion(float _x, float _y, float _z, float _w) {x=_x; y=_y; z=_z; w=_w;}
	void set(float _x, float _y, float _z, float _w) {x=_x; y=_y; z=_z; w=_w;}

	SMeshQuaternion operator +(SMeshQuaternion &q)
	{
		SMeshQuaternion qRet ;
		qRet.set(x+q.x, y+q.y, z+q.z, w+q.w) ;
		return qRet ;
	}
	SMeshQuaternion &operator +=(SMeshQuaternion &q)
	{
		set(x+q.x, y+q.y, z+q.z, w+q.w) ;
		return *this ;
	}

	SMeshQuaternion operator *(SMeshQuaternion &q)
	{
		float fx, fy, fz, fw ;

		fw	= q.w*w - q.x*x - q.y*y - q.z*z;

		fx	= q.w*x + q.x*w + q.y*z - q.z*y;
		fy	= q.w*y + q.y*w + q.z*x - q.x*z;
		fz	= q.w*z + q.z*w + q.x*y - q.y*x;

		return SMeshQuaternion(fx, fy, fz, fw) ;
	}
	SMeshQuaternion &operator *=(SMeshQuaternion &q)
	{
		float fx, fy, fz, fw ;

		fw	= q.w*w - q.x*x - q.y*y - q.z*z;

		fx	= q.w*x + q.x*w + q.y*z - q.z*y;
		fy	= q.w*y + q.y*w + q.z*x - q.x*z;
		fz	= q.w*z + q.z*w + q.x*y - q.y*x;

		x=fx ; y=fy ; z=fz ; w=fw ;

		return *this ;
	}

} ;
struct SMeshVector
{
	float x, y, z ;

	SMeshVector() {x=y=z=0.0f;}
	void set(float _x, float _y, float _z) {x=_x; y=_y; z=_z;}
} ;
struct SMeshTexCoord
{
	float u, v ;
	SMeshTexCoord() {u=v=0.0f;}
	void set(float _u, float _v) {u=_u; v=_v;}
} ;

struct SColor
{
	float r, g, b, a ;

	SColor()
	{
		r = g = b = a = 1.0f ;
	}
} ;

struct SSceneInfo
{
	float fFirstFrame, fLastFrame ;
	float fFrameSpeed, fTicksPerFrame ;

	SSceneInfo()
	{
		fFirstFrame=fLastFrame=0 ;
		fFrameSpeed=fTicksPerFrame=0 ;
	}
} ;

enum MATERIAL_ATTR
{
	MATERIAL_ATTR_TWOSIDED=0x01,
	MATERIAL_ATTR_ALPHABLENDING=0x02,
} ;

struct SMaterial
{
	int nAttr ;
	bool bUseTexDecal ;
	bool bUseOpacity ;
	bool bUseNormalMap ;
	bool bUseAmbientMap ;
	char szDecaleName[256] ;
	char szBumpName[256] ;
	char szAmbientName[256] ;

	int nNumSubMaterial ;
	SMaterial *psSubMaterial ;

	SColor sDiffuse, sAmbient, sSpecular ;
	SMeshTexCoord sTexTiling ;

	SMaterial()
	{
		nAttr=0 ;
		bUseTexDecal=false ;
		bUseOpacity=false ;
		bUseNormalMap=false ;
		bUseAmbientMap=false ;
		memset(szDecaleName, 0, 256) ;
		memset(szBumpName, 0, 256) ;
		memset(szAmbientName, 0, 256) ;

		nNumSubMaterial=0 ;
		psSubMaterial=NULL ;
	}
	~SMaterial()
	{
		if(psSubMaterial)
		{
			delete []psSubMaterial ;
			psSubMaterial = NULL ;
		}
	}
} ;

struct SMeshKeyPos
{
	int nFrame ;
	SMeshVector pos ;
} ;

struct SMeshKeyRot
{
	int nFrame ;
	SMeshQuaternion q ;
} ;

struct SMeshKeyScale
{
	int nFrame ;
	SMeshVector scale ;
} ;

#define MAXNUM_SKIN 12

struct SMeshSkin
{
	bool bEnable ;
	int nBoneIndex ;
	float fWeight ;
	char szBoneName[256] ;

	SMeshSkin()
	{
		bEnable = false ;
		fWeight = 1.0f ;
	}
} ;

struct SMeshVertex
{
	bool bEnable ;
	SMeshVector pos ;
	SMeshVector normal ;
	SMeshVector tangent ;
	SMeshTexCoord tex ;
	int nNumSkin ;
	float fAmbientDegree ;
	SMeshSkin *apsSkin[MAXNUM_SKIN] ;

	SMeshVertex()
	{
		bEnable = false ;
		fAmbientDegree = 1 ;
		nNumSkin = 0 ;
		for(int i=0 ; i<MAXNUM_SKIN ; i++)
			apsSkin[i] = NULL ;
	}

	~SMeshVertex()
	{
		for(int i=0 ; i<nNumSkin ; i++)
		{
			if(apsSkin[i])
			{
				delete apsSkin[i] ;
				apsSkin[i] = NULL ;
			}
		}
	}
} ;

struct SVertexDisk
{
	Vector3 vPos ;
	Vector3 vNormal ;
	float fRadius ;
	float fAmbientDegree ;
	int nIndexOfVertex ;
	int nAccumulatedPos ;

	SVertexDisk()
	{
		nIndexOfVertex = -1 ;
		nAccumulatedPos = 0 ;
		fRadius = fAmbientDegree = 0.0f ;
	} ;
	SVertexDisk(Vector3 _vPos, Vector3 _vNormal, float _fRadius)
	{
		set(_vPos, _vNormal, _fRadius) ;		
	} ;

	void set(Vector3 _vPos, Vector3 _vNormal, float _fRadius)
	{
		vPos = _vPos ;
		vNormal = _vNormal ;
		fRadius = _fRadius ;
	} ;
} ;

struct SMeshTriangle
{
	int nMaterialID ;
	int anVertexIndex[3] ;
	unsigned long dwVertexColor[3] ;
	SMeshVector asVertexNormal[3] ;
	SMeshVector sFaceNormal ;
	SMeshTexCoord asTexCoord[3] ;
	
	SMeshTriangle()
	{
		nMaterialID = 0 ;

		for(int i=0 ; i<3 ; i++)
		{
			anVertexIndex[i] = 0 ;
			dwVertexColor[i] = 0 ;
			asVertexNormal[i].set(0.0f, 0.0f, 0.0f) ;
			asTexCoord[i].set(0.0f, 0.0f) ;
		}

		sFaceNormal.set(0.0f, 0.0f, 0.0f) ;
	}

} ;

enum SMESHATTR
{
	SMESHATTR_ANIMATION=0x01,
	SMESHATTR_USESUBMATERIAL=0x02,
	SMESHATTR_USENORMALMAP=0x04,
	SMESHATTR_USEAMBIENTDEGREE=0x08,
	SMESHATTR_GROUPED=0x10,
	SMESHATTR_ALPHABLENDING=0x20,
	SMESHATTR_TWOSIDED=0x40,
	SMESHATTR_NOTEXTURE=0x80,
} ;

enum SMESHKIND
{
	SMESHKIND_NONE=0,
	SMESHKIND_BONE,
	SMESHKIND_RIGID,
	SMESHKIND_DUMMY,
	SMESHKIND_SKINNING,
	SMESHKIND_BILLBOARD,
	SMESHKIND_XBILLBOARD,
} ;

#define MAXNUM_MESHKEY 1024

struct SMesh
{
	int nAttr ;
    SMESHKIND MeshKind ;
	unsigned long lVertexType ;
	char szMeshName[256] ;
	bool bParent ;
	char szParentName[256] ;

	int nNumVertex ;
	SMeshVertex *psVertex ;
	
	int nNumTriangle ;
	SMeshTriangle *psTriangle ;

	int nNumTexCoord ;
	SMeshTexCoord *psTexCoord ;

    SMeshMatrix smatLocal, smatChild ;
	SMeshVector svPos ;
	SMeshVector svRotAxis ;
	float fRotAngle ;
	SMeshVector svScale ;

	int nNumMeshKeyPos ;
	SMeshKeyPos *apMeshKeyPos[MAXNUM_MESHKEY] ;

	int nNumMeshKeyRot ;
	SMeshKeyRot *apMeshKeyRot[MAXNUM_MESHKEY] ;

    int nNumMeshKeyScale ;
	SMeshKeyScale *apMeshKeyScale[MAXNUM_MESHKEY] ;

	int nMeshParentIndex ;
	SMesh *psMeshParent ;

	//SMesh *psMeshNext ;

	int nMaterialIndex ;
	bool bAmbientDegree ;
	//SColor *psColor ;
	//SMaterial *psMaterial ;

	SMeshVector vMin, vMax ;

	SMesh()
	{
		nAttr = 0 ;
		lVertexType = 0 ;
		bParent = false ;
		memset(szParentName, 0, 256) ;
		memset(szMeshName, 0, 256) ;
		nNumVertex = nNumTriangle = nNumTexCoord = 0 ;
		psVertex = NULL ;
		nNumTriangle = 0 ;
		psTriangle = NULL ;
		psTexCoord = NULL ;
		smatLocal.set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f) ;
		//psMeshNext = NULL ;
		psMeshParent = NULL ;
		nMaterialIndex = 0 ;
		//psColor = NULL ;
		//psMaterial = NULL ;
		nNumMeshKeyPos = nNumMeshKeyRot = nNumMeshKeyScale = 0 ;
		bAmbientDegree = false ;
	}
	~SMesh()
	{
		if(psVertex)
		{
			delete []psVertex ;
			psVertex = NULL ;
		}
		if(psTriangle)
		{
			delete []psTriangle ;
			psTriangle = NULL ;
		}
		if(psTexCoord)
		{
			delete []psTexCoord ;
			psTexCoord = NULL ;
		}
		//if(psColor)
		//{
		//	delete []psColor ;
		//	psColor = NULL ;
		//}
		//if(psMaterial)
		//{
		//	delete []psMaterial ;
		//	psMaterial = NULL ;
		//}

		int i ;
		if(nNumMeshKeyPos > 0)
		{
			for(i=0 ; i<nNumMeshKeyPos ; i++)
			{
				if(apMeshKeyPos[i])
				{
					delete apMeshKeyPos[i] ;
					apMeshKeyPos[i] = NULL ;
				}
			}
		}
		if(nNumMeshKeyRot > 0)
		{
			for(i=0 ; i<nNumMeshKeyRot ; i++)
			{
				if(apMeshKeyRot[i])
				{
					delete apMeshKeyRot[i] ;
					apMeshKeyRot[i] = NULL ;
				}
			}
		}
		if(nNumMeshKeyScale > 0)
		{
			for(i=0 ; i<nNumMeshKeyScale ; i++)
			{
				if(apMeshKeyRot[i])
				{
					delete apMeshKeyScale[i] ;
					apMeshKeyScale[i] = NULL ;
				}
			}
		}
	}
} ;

struct SShapeLine
{
	bool bClosed ;
	int nNumPos ;
    SMeshVector *psPos ;

	SShapeLine()
	{
		bClosed = false ;
		nNumPos=0 ;
		psPos=NULL ;
	}
	~SShapeLine()
	{
		if(psPos) {delete []psPos; psPos=NULL; }
	}
} ;

struct SShapeObject
{
	char szName[256] ;

	SMeshMatrix smatLocal ;
	SMeshVector svPos ;
	SMeshVector svRotAxis ;
	float fRotAngle ;
	SMeshVector svScale ;

	int nNumLine ;
	SShapeLine *psShapeLine ;

    SShapeObject()
	{
		nNumLine=0 ;
		psShapeLine=NULL ;
	}
    ~SShapeObject()
	{
		if(psShapeLine) { delete []psShapeLine; psShapeLine=NULL; }
	}
} ;

#define MAXNUM_ASEDATAMESH 1024
#define MAXNUM_MATERIAL 512
#define MAXNUM_BONE 512
#define MAXNUM_SHAPEOBJECT 128

class CSimpleQuadTree ;

//ASE 파일에는 GEOMOBJECT{ *NODE_TM{}, *MESH{}, *TM_ANIMATION{} } 이렇게 구성이 되어 있지만
//CASEData 에서는 GEOMOBJECT를 하나의 MESH로 취급한다.
class CASEData
{
public :
	enum KIND
	{
		KIND_MODEL=0, KIND_SHAPE
	} ;
	enum ATTR
	{
		ATTR_SOLID=0x01, ATTR_ANI=0x02, ATTR_LINE=0x04
	} ;
	enum SUBATTR
	{
		SUBATTR_ANI_KEYFRAME=0x01, SUBATTR_ANI_SKINNED=0x02
	} ;
	enum RESULT
	{
		RET_NOTRMDFILE, RET_WRONGRMDVER
	} ;

private :

public :
	SSceneInfo m_sSceneInfo ;

    UINT m_lKind ;
	UINT m_lAttr, m_lSubAttr ;
	bool m_bUseDummyObject ;

	int m_nNumMaterial ;
	SMaterial *m_psMaterial ;

	int m_nNumMesh ;
	int m_nMeshNode ;
    SMesh *m_apMesh[MAXNUM_ASEDATAMESH] ;

	SMeshVector m_vMin, m_vMax ;
	int m_nWidth, m_nHeight ;

	std::vector<SVertexDisk> m_VertexDisks ;
	int m_nProgress ;

	//에니메이션이 스키닝일경우
	//본의 계층구조 순서와 본이름을 매칭시키기위해 사용된다.
	int m_nNumBone ;
	char *m_apszBoneName[MAXNUM_BONE] ;

	char m_szFileName[256] ;
	geo::STriangle *m_psTriangles ;

	int m_nNumShapeObject ;
	SShapeObject *m_apsShapeObject[MAXNUM_SHAPEOBJECT] ;

private :
	void _WriteString(char *psz, FILE *pf) ;
	void _ReadString(char *psz, FILE *pf) ;

	bool _WriteMaterial(SMaterial *psMaterial, FILE *pf) ;
	bool _ReadMaterial(SMaterial *psMaterial, FILE *pf) ;

	bool _WriteMesh(SMesh *pMesh, FILE *pf) ;
	bool _ReadMesh(SMesh *pMesh, FILE *pf) ;

	void _WriteMeshVertex(SMeshVertex *psVertex, FILE *pf) ;
	void _ReadMeshVertex(SMeshVertex *psVertex, FILE *pf) ;

	void _WriteMeshSkin(SMeshSkin *psSkin, FILE *pf) ;
	void _ReadMeshSkin(SMeshSkin *psSkin, FILE *pf) ;

public :
	CASEData() ;
	~CASEData() ;

	void WeldVertex(HWND hwnd=NULL) ;//중복된 버텍스 데이타를 제거하고 텍스쳐 좌표와 연관지어서 버텍스데이타를 재생성 시킨다.
	bool FindMesh(char *pszNodeName, SMesh **ppMesh, int &nIndex) ;
	void Release() ;

	bool FindBoneIndex(char *pszNodeName, int &nIndex) ;

	//Renderable Model Data 최적화된 모델데이타(*.RMD) Export, Import 시킨다.
	bool ExportRMD(char *pszFileName) ;
	bool ImportRMD(char *pszFileName) ;

	//void ComputeTangent(SMeshTriangle *psTriangle) ;
	void MakeTangent() ;//높이맵을 기반으로 탄젠트공간의 노말벡터들을 구해낸다.

    //AmbientOcclusion
	float Corresponded(SMeshVertex *psVertex, Vector3 *pv) ;
	float ComputeDiskAreaPerVertex(SMeshVertex *psVertex, SMesh *psMesh) ;
	void PreComputeAmbientOcclusionPerVertex(int &nTotalVertex, int &nCurVertexCount, int &nCurRatio) ;
	void SetVertexDisk() ;
	void SubSetVertexDisk(std::vector<SVertexDisk> *pVertexDisks, SMesh *pMesh) ;
	bool AnotherDiskTest(Vector3 &v1, Vector3 &v2, SVertexDisk *psCurDisk) ;

	bool TestOcclusion(geo::SLine *psLine, SMesh *psCurMesh) ;

	//Build QuadTree
	CSimpleQuadTree *BuildSimpleQuadTree() ;
	void BuildTerrainFile(char *pszFileName, CSimpleQuadTree *pcQuadTree) ;
} ;

void MakeTangent(SMeshVertex *psVertices, int nNumTriangle, SMeshTriangle *psTriangles) ;
bool IdentityVertex(void *pv1, void *pv2) ;