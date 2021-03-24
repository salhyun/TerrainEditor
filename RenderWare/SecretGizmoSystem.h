#include "def.h"
#include "D3DDef.h"
#include "GeoLib.h"
#include "Vector3.h"

enum GIZMOSUBSET
{
	GIZMOSUBSET_AXISX=0x01,
	GIZMOSUBSET_AXISY=0x02,
	GIZMOSUBSET_AXISZ=0x04,
	GIZMOSUBSET_PLANEXY=0x08,
	GIZMOSUBSET_PLANEXZ=0x10,
	GIZMOSUBSET_PLANEYZ=0x20,
	GIZMOSUBSET_ROTATEBYAXISX=0x40,
	GIZMOSUBSET_ROTATEBYAXISY=0x80,
	GIZMOSUBSET_ROTATEBYAXISZ=0x100,
} ;

struct SGizmoObject
{
	enum KIND {KIND_LINE=0x01, KIND_MESH} ;
	enum ATTR {ATTR_SELECTED=0x01, ATTR_BASEAXIS=0x02, ATTR_MOVE=0x04, ATTR_ROTATE=0x08, ATTR_SCALE=0x10} ;

	int nKind, nAttr ;
	int nNumVertex ;
	SCVertex *psOriginVertices, *psVertices ;

	int nNumIndex ;
	SCIndex *psOriginIndices, *psIndices ;

	D3DXVECTOR3 vCenter ;
	int nNumPosition ;
	D3DXVECTOR3 *pvOriginPositions, *pvPositions ;

	float fLineWidth ;

    int nNumSubmitVertex, nNumSubmitIndex ;

	SGizmoObject() ;
	~SGizmoObject() ;
	void SetVertex(int nNum, SCVertex *_psVertices=NULL) ;
	void SetIndex(int nNum, SCIndex *_psIndices=NULL) ;
	void SetPositions(int nNum, D3DXVECTOR3 *_pvPositions=NULL) ;
    void Process() ;

} ;
struct SGizmoTrueRenderingObject
{
	float fDistfromCamera ;
	D3DXMATRIX *pmatWorld ;
	SGizmoObject *psObject ;

	void Process(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, geo::SPlane *psViewPlane, int nGizmoType) ;
} ;

struct SGizmoMoveGeometry
{
    DWORD dwSelectedSubset ;
	geo::SLine sAxisX, sAxisY, sAxisZ ;
	geo::SPlane sPlaneXY, sPlaneXZ,  sPlaneYZ ;
	Vector3 vFirstPos ;

	SGizmoMoveGeometry() ;
	DWORD IntersectSubset(geo::SLine *psLine, Matrix4 *pmatWorld) ;
	int GetIntersectedPoswithSubset(geo::SLine *psLine, Matrix4 *pmatWorld, Vector3 *pvPos) ;
} ;

struct SGizmoRotateGeometry
{
	DWORD dwSelectedSubset ;
	geo::SLine sAxisX, sAxisY, sAxisZ ;
    geo::SPlane sPlaneXY, sPlaneXZ,  sPlaneYZ ;
	geo::SLine sTangentLine ;
	geo::SPlane sRotatePlane ;
	D3DXMATRIX matRotate ;

	SGizmoRotateGeometry() ;
	DWORD IntersectSubset(geo::SLine *psLine, geo::SPlane *psViewPlane, Matrix4 *pmatWorld) ;
	int GetIntersectedPoswithSubset(geo::SLine *psLine, Matrix4 *pmatWorld, Vector3 *pvPos) ;
} ;
struct SGizmoScaleGeometry
{
	geo::SLine sAxisX, sAxisY, sAxisZ ;
} ;

struct SGizmoScrape
{
	enum SCRAPE_STATUS {SCRAPE_IDLE=0, SCRAPE_START, SCRAPE_SCRAPING, SCRAPE_END} ;

	int nStatus ;
	float fSpeed ;
	Vector3 vPrePos, vCurPos, vAmount ;

	SGizmoScrape() ;
	void ProcessStatus(bool bPressed) ;
	void Process(Vector3 vPos, int nIntersect) ;
	int GetStatus() {return nStatus;}
} ;

class CSecretGizmoSystem
{
public :
	enum GIZMOTYPE
	{MOVE=1, ROTATE, SCALE} ;

private :
	bool m_bEnable ;
	int m_nType ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	D3DXMATRIX m_matWorld, m_matOrigin ;

	SCVertex *m_psOriginVertices, *m_psVertices ;
	unsigned short *m_pwLineIndices, *m_pwTriangleIndices ;

	int m_nNumVertex ;
    int m_nLineMinVertex, m_nLineNumVertex, m_nLineCount ;
	int m_nTriangleMinVertex, m_nTriangleNumVertex, m_nTriangleCount ;

	float m_fPreScale, m_fCurScale ;

	int nNumObject ;
	SGizmoObject *m_psGizmoObjects ;
	SGizmoTrueRenderingObject *m_psGizmoTRObjects ;


public :
    SGizmoScrape m_sScrape ;
	SGizmoMoveGeometry m_sGizmoMoveGeometry ;
	SGizmoRotateGeometry m_sGizmoRotateGeometry ;
	geo::SPlane m_sViewPlane ;

public :
	CSecretGizmoSystem() ;
	~CSecretGizmoSystem() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice) ;
	void Process(Vector3 *pvCamera, geo::SLine *psPickingLine, D3DXMATRIX *pmatV, D3DXMATRIX *pmatP, bool bPressed) ;
	void Render() ;

	void SetType(int nType) ;
	int GetType() ;
	void SetmatWorld(D3DXMATRIX *pmatWorld) ;
	D3DXMATRIX *GetmatWorld() {return &m_matWorld;}
	void SetEnable(bool bEnable) {m_bEnable=bEnable;}
	bool IsEnable() {return m_bEnable;}

	bool IsSelectedSubset() ;

} ;