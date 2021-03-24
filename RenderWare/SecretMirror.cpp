#include "SecretMirror.h"
#include "MathOrdinary.h"
#include "Quaternion.h"
#include "GeoLib.h"
#include "SecretTerrain.h"
#include "SecretSky.h"
#include "SecretCloud.h"
#include "SecretRenderingGroup.h"
#include "SecretSkyDome.h"
#include "SecretSceneManager.h"

void SMirrorPlane::BuildMirror(Matrix4 &_matTransform, Vector3 &_vNormal, float width, float height)
{
	typedef struct
	{
		float x, y, z ;
		float nx, ny, nz ;
		float tu, tv ;
	} VERTEX ;

	VERTEX vertex[4] =
	{//      x      y      z      nx     ny     nz    tu    tv
		//{-fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f},
		//{+fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f},
		//{+fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f},
		//{-fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f},

        //미러링을 하니까 좌우를 바꿔준다.
		{-width/2.0f, +0.0f, +height/2.0f, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f},
		{+width/2.0f, +0.0f, +height/2.0f, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f},
		{+width/2.0f, +0.0f, -height/2.0f, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f},
		{-width/2.0f, +0.0f, -height/2.0f, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f},

		//{-fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f},
		//{+fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f},
		//{+fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f},
		//{-fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f},
	} ;

	for(int i=0 ; i<4 ; i++)
	{
		sVertex[i].pos = D3DXVECTOR3(vertex[i].x, vertex[i].y, vertex[i].z) ;
		//sVertex[i].normal = D3DXVECTOR3(vertex[i].nx, vertex[i].ny, vertex[i].nz) ;
		sVertex[i].normal = VectorConvert(_vNormal) ;
		sVertex[i].tex = D3DXVECTOR2(vertex[i].tu, vertex[i].tv) ;
	}

    typedef struct
	{
		WORD index[3] ;
	} INDEX ;

	INDEX index[] =
	{
		{0, 1, 3},
		{1, 2, 3},
	} ;

	sIndex[0].index[0] = index[0].index[0] ;
	sIndex[0].index[1] = index[0].index[1] ;
	sIndex[0].index[2] = index[0].index[2] ;
	sIndex[1].index[0] = index[1].index[0] ;
	sIndex[1].index[1] = index[1].index[1] ;
	sIndex[1].index[2] = index[1].index[2] ;

	vNormal = _vNormal ;
	matTransform = _matTransform ;
	fWidth = width ;
	fHeight = height ;
}
void SMirrorPlane::BuildBehindMirror(Matrix4 &_matTransform, Vector3 &_vNormal, float width, float height)
{
	typedef struct
	{
		float x, y, z ;
		float nx, ny, nz ;
		float tu, tv ;
	} VERTEX ;

	VERTEX vertex[4] =
	{//      x      y      z      nx     ny     nz    tu    tv
		//{-fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f},
		//{+fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f},
		//{+fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f},
		//{-fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f},

        //BehindMirror는 바꾸지 않는다.
		{-width/2.0f, +0.0f, +height/2.0f, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f},
		{+width/2.0f, +0.0f, +height/2.0f, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f},
		{+width/2.0f, +0.0f, -height/2.0f, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f},
		{-width/2.0f, +0.0f, -height/2.0f, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f},

		//{-fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 0.0f, 1.0f},
		//{+fSize, +0.0f, +fSize, +0.0f, +1.0f, +0.0f, 1.0f, 1.0f},
		//{+fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 1.0f, 0.0f},
		//{-fSize, +0.0f, -fSize, +0.0f, +1.0f, +0.0f, 0.0f, 0.0f},
	} ;

	for(int i=0 ; i<4 ; i++)
	{
		sVertex[i].pos = D3DXVECTOR3(vertex[i].x, vertex[i].y, vertex[i].z) ;
		//sVertex[i].normal = D3DXVECTOR3(vertex[i].nx, vertex[i].ny, vertex[i].nz) ;
		sVertex[i].normal = VectorConvert(_vNormal) ;
		sVertex[i].tex = D3DXVECTOR2(vertex[i].tu, vertex[i].tv) ;
	}

    typedef struct
	{
		WORD index[3] ;
	} INDEX ;

	INDEX index[] =
	{
		{0, 1, 3},
		{1, 2, 3},
	} ;

	sIndex[0].index[0] = index[0].index[0] ;
	sIndex[0].index[1] = index[0].index[1] ;
	sIndex[0].index[2] = index[0].index[2] ;
	sIndex[1].index[0] = index[1].index[0] ;
	sIndex[1].index[1] = index[1].index[1] ;
	sIndex[1].index[2] = index[1].index[2] ;

	vNormal = _vNormal ;
	matTransform = _matTransform ;
	fWidth = width ;
	fHeight = height ;
}

Vector3 SMirrorPlane::GetPos()
{
	return Vector3(matTransform.m41, matTransform.m42, matTransform.m43) ;
}

CSecretMirror::CSecretMirror()
{
	m_pTexReflect = NULL ;
	m_pSurfReflect = NULL ;
	m_pSurfZBuffer = NULL ;
	m_pTexBehind = NULL ;
	m_pSurfBehind = NULL ;
	m_nRenderMethod = 0 ;
	m_bDepthWater = false ;
	m_fClipingMoveZ = 0.0f ;
}

CSecretMirror::CSecretMirror(Vector3 vMirrorPos, Vector3 vNormal, float fWidth, float fHeight, Matrix4 &matRotation, LPDIRECT3DDEVICE9 pd3dDevice, bool bDrawFrustum, bool bDrawCoordsys)
{
    Initialize(vMirrorPos, vNormal, fWidth, fHeight, matRotation, pd3dDevice, bDrawFrustum, bDrawCoordsys) ;
}

CSecretMirror::~CSecretMirror()
{
	Release() ;
}
void CSecretMirror::Release()
{
	SAFE_RELEASE(m_pTexReflect) ;
	SAFE_RELEASE(m_pSurfReflect) ;
	SAFE_RELEASE(m_pTexBehind) ;
	SAFE_RELEASE(m_pSurfBehind) ;
	SAFE_RELEASE(m_pSurfZBuffer) ;
}
HRESULT CSecretMirror::Initialize(Vector3 vMirrorPos, Vector3 vNormal, float fWidth, float fHeight, Matrix4 &matRotation, LPDIRECT3DDEVICE9 pd3dDevice, bool bDrawFrustum, bool bDrawCoordsys)
{
	m_bDrawFrustum = bDrawFrustum ;
	m_bDrawCoordsys = bDrawCoordsys ;
	_SetMirrorPlane(vMirrorPos, vNormal, fWidth, fHeight, matRotation) ;
	m_pd3dDevice = pd3dDevice ;

	float half_width = fWidth*0.5f ;
	float half_height = fHeight*0.5f ;
	m_fRadius = sqrt( (half_width*half_width) + (half_height*half_height) ) ;

	if(FAILED(m_pd3dDevice->CreateTexture(TEXTURESIZE, TEXTURESIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexReflect, NULL)))
		return E_FAIL ;
	if (FAILED(m_pTexReflect->GetSurfaceLevel(0, &m_pSurfReflect)))
		return E_FAIL;

	if(FAILED(m_pd3dDevice->CreateTexture(TEXTURESIZE, TEXTURESIZE, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexBehind, NULL)))
		return E_FAIL ;
	if(FAILED(m_pTexBehind->GetSurfaceLevel(0, &m_pSurfBehind)))
		return E_FAIL;

	if(FAILED(m_pd3dDevice->CreateDepthStencilSurface(TEXTURESIZE, TEXTURESIZE, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pSurfZBuffer, NULL)))
		return E_FAIL ;

	m_cCoordsys.Initialize(pd3dDevice) ;
	m_nRenderMethod = 0 ;
	m_bDepthWater = false ;
	m_fClipingMoveZ = 0.0f ;

    return S_OK ;
}

void CSecretMirror::_SetMirrorPlane(Vector3 vMirrorPos, Vector3 vNormal, float fWidth, float fHeight, Matrix4 &matRotation)
{
	//Matrix4 mat ;
	//mat.Identity() ;

	//vMirrorPos.set(0, -1.125f, -12) ;

	//mat.SetRotatebyAxisX(90) ;
	//vNormal.set(0, 1, 0) ;
	//vNormal *= mat ;
	//mat.m41 = vMirrorPos.x ;
	//mat.m42 = vMirrorPos.y ;
	//mat.m43 = vMirrorPos.z ;

    matRotation.m41 = vMirrorPos.x ;
	matRotation.m42 = vMirrorPos.y ;
	matRotation.m43 = vMirrorPos.z ;

	m_sMirrorPlane.BuildMirror(matRotation, vNormal, fWidth, fHeight) ;
}

void CSecretMirror::Process(Vector3 &vCameraPos)
{
	if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
		_ProcessMirror(vCameraPos) ;
	else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)
		_ProcessBehindMirror(vCameraPos) ;
}
void CSecretMirror::_ProcessMirror(Vector3 &vCameraPos)
{
	Matrix4 matReflectView ;
	Vector3 vUpper, vRight, vLookat, vPos, vMirrorPos, vMirrorNormal ;

	//vCameraPos.set(0, 4.15f, -2) ;
	//vCameraPos.set(0, 2, 0) ;

    Matrix4 matMirror = m_sMirrorPlane.matTransform ;
	matMirror.m41 = matMirror.m42 = matMirror.m43 = 0.0f ;

	vMirrorNormal = m_sMirrorPlane.vNormal*matMirror ;
	vMirrorPos = m_sMirrorPlane.GetPos() ;

	if(((vMirrorPos-vCameraPos).Magnitude() <= 0.01f) || ((vMirrorPos-vCameraPos).Normalize().dot(vMirrorNormal) > 0))
	{
		m_bRender = false ;
		return ;
	}
	else
		m_bRender = true ;

	//##################//
	//make ReflectMatrix//
    //##################//
	vPos = Vector3(vCameraPos.x, vMirrorPos.y-vCameraPos.y, vCameraPos.z) ;
	vLookat = vMirrorNormal ;
	Vector3 vLookatMirror(vCameraPos.x-vMirrorPos.x, vCameraPos.y-vMirrorPos.y, vCameraPos.z-vMirrorPos.z) ;
	Vector3 vProj = ProjectionNormal(vLookatMirror, vLookat) ;
    vUpper = vLookatMirror-vProj ;
	//vUpper = Vector3(vCameraPos.x-vMirrorPos.x, 0, vCameraPos.z-vMirrorPos.z) ;

	vPos = vMirrorPos+((-vProj)+vUpper) ;

	vUpper.set(0, 0, 1) ;

	vUpper = vUpper.Normalize() ;
	vRight = vUpper.cross(vLookat) ;

	matReflectView.m11 = vRight.x ;
	matReflectView.m21 = vRight.y ;
	matReflectView.m31 = vRight.z ;

	matReflectView.m12 = vUpper.x ;
	matReflectView.m22 = vUpper.y ;
	matReflectView.m32 = vUpper.z ;

	matReflectView.m13 = vLookat.x ;
	matReflectView.m23 = vLookat.y ;
	matReflectView.m33 = vLookat.z ;

	matReflectView.m41 = -vRight.dot(vPos) ;
	matReflectView.m42 = -vUpper.dot(vPos) ;
	matReflectView.m43 = -vLookat.dot(vPos) ;

	matReflectView.m14 = matReflectView.m24 = matReflectView.m34 = 0.0f ;
	matReflectView.m44 = 1.0f ;

	m_matReflectView = MatrixConvert(matReflectView) ;
	if(m_bDrawCoordsys)
	{
		D3DXMATRIX matsys ;
		D3DXMatrixInverse(&matsys, NULL, &m_matReflectView) ;
		m_cCoordsys.SetMatrix(matsys) ;
	}
    
    //카메라가 거울을 중심으로 회전한 만큼 버텍스에 계산해준다.
	//float ftheta = IncludedAngle(Vector3(0, 0, 1), Vector3(vCameraPos.x-vMirrorPos.x, 0, vCameraPos.z-vMirrorPos.z)) ;

	float ftheta = IncludedAngle(Vector3(0, 0, 1)*matMirror, vUpper) ;
	if((vCameraPos.x-vMirrorPos.x) < 0)
		ftheta = 2.0f*3.141592f-ftheta ;

	Matrix4 matTransform, matInvRot ;
	Quaternion q(vMirrorNormal, ftheta) ;
	q.GetMatrix(matTransform) ;

	matTransform = matMirror*matTransform ;

	//TRACE_Matrix(matTransform) ;

	_BuildReflectProjection(matTransform, vMirrorPos, matReflectView) ;
	_BuildFrustum() ;

	Vector3 vVertex, vNormal ;

	matTransform.SetTranslation(vMirrorPos) ;
	m_matMirrorTransform = MatrixConvert(matTransform) ;

	vNormal.x = vMirrorNormal.x*matTransform.m11 + vMirrorNormal.y*matTransform.m21 + vMirrorNormal.z*matTransform.m31 ;
	vNormal.y = vMirrorNormal.x*matTransform.m12 + vMirrorNormal.y*matTransform.m22 + vMirrorNormal.z*matTransform.m32 ;
	vNormal.z = vMirrorNormal.x*matTransform.m13 + vMirrorNormal.y*matTransform.m23 + vMirrorNormal.z*matTransform.m33 ;

	for(int i=0 ; i<4 ; i++)
	{
		vVertex = Vector3(m_sMirrorPlane.sVertex[i].pos.x, m_sMirrorPlane.sVertex[i].pos.y, m_sMirrorPlane.sVertex[i].pos.z) ;
		vVertex *= matTransform ;
		m_sVertex[i].pos = VectorConvert(vVertex) ;
		m_sVertex[i].normal = VectorConvert(vNormal) ;
		m_sVertex[i].tex = m_sMirrorPlane.sVertex[i].tex ;
	}

	m_sIndex[0] = m_sMirrorPlane.sIndex[0] ;
	m_sIndex[1] = m_sMirrorPlane.sIndex[1] ;
} ;
void CSecretMirror::_ProcessBehindMirror(Vector3 &vCameraPos)
{
	Matrix4 matReflectView ;
	Vector3 vUpper, vRight, vLookat, vPos, vMirrorPos, vMirrorNormal ;

	//vCameraPos.set(0, 4.15f, -2) ;
	//vCameraPos.set(0, 2, 0) ;

    Matrix4 matMirror = m_sMirrorPlane.matTransform ;
	matMirror.m41 = matMirror.m42 = matMirror.m43 = 0.0f ;

	//vMirrorNormal = m_sMirrorPlane.vNormal*matMirror ;
	vMirrorNormal = -m_sMirrorPlane.vNormal*matMirror ;
	vMirrorPos = m_sMirrorPlane.GetPos() ;

	if(((vMirrorPos-vCameraPos).Magnitude() <= 0.01f) || ((vCameraPos-vMirrorPos).Normalize().dot(vMirrorNormal) > 0))
	{
		m_bRender = false ;
		return ;
	}
	else
		m_bRender = true ;

	//##################//
	//make ReflectMatrix//
    //##################//
	vPos = Vector3(vCameraPos.x, vCameraPos.y, vCameraPos.z) ;
	vLookat = vMirrorNormal ;
	Vector3 vLookatMirror(vMirrorPos.x-vCameraPos.x, vMirrorPos.y-vCameraPos.y, vMirrorPos.z-vCameraPos.z) ;
	Vector3 vProj = ProjectionNormal(vLookatMirror, vLookat) ;
    vUpper = vProj-vLookatMirror ;
	//vUpper = Vector3(vCameraPos.x-vMirrorPos.x, 0, vCameraPos.z-vMirrorPos.z) ;

	vUpper.set(0, 0, 1) ;
    
	vUpper = vUpper.Normalize() ;
	vRight = vUpper.cross(vLookat) ;

	matReflectView.m11 = vRight.x ;
	matReflectView.m21 = vRight.y ;
	matReflectView.m31 = vRight.z ;

	matReflectView.m12 = vUpper.x ;
	matReflectView.m22 = vUpper.y ;
	matReflectView.m32 = vUpper.z ;

	matReflectView.m13 = vLookat.x ;
	matReflectView.m23 = vLookat.y ;
	matReflectView.m33 = vLookat.z ;

	matReflectView.m41 = -vRight.dot(vPos) ;
	matReflectView.m42 = -vUpper.dot(vPos) ;
	matReflectView.m43 = -vLookat.dot(vPos) ;

	matReflectView.m14 = matReflectView.m24 = matReflectView.m34 = 0.0f ;
	matReflectView.m44 = 1.0f ;

	m_matReflectView = MatrixConvert(matReflectView) ;
	if(m_bDrawCoordsys)
	{
		D3DXMATRIX matsys ;
		D3DXMatrixInverse(&matsys, NULL, &m_matReflectView) ;
		m_cCoordsys.SetMatrix(matsys) ;
	}
    
    //카메라가 거울을 중심으로 회전한 만큼 버텍스에 계산해준다.
	//float ftheta = IncludedAngle(Vector3(0, 0, 1), Vector3(vCameraPos.x-vMirrorPos.x, 0, vCameraPos.z-vMirrorPos.z)) ;

	float ftheta = IncludedAngle(Vector3(0, 0, 1)*matMirror, vUpper) ;
	//if((vCameraPos.x-vMirrorPos.x) < 0)
	if((vMirrorPos.x-vCameraPos.x) < 0)
		ftheta = 2.0f*3.141592f-ftheta ;

	Matrix4 matTransform ;
	//Quaternion q(Vector3(0, 1, 0), ftheta) ;
	Quaternion q(vMirrorNormal, ftheta) ;
	q.GetMatrix(matTransform) ;

	matTransform = matMirror*matTransform ;

	//TRACE_Matrix(matTransform) ;

	_BuildReflectProjection(matTransform, vMirrorPos, matReflectView) ;
	_BuildFrustum() ;

	Vector3 vVertex, vNormal ;

	matTransform.SetTranslation(vMirrorPos) ;
	m_matBehindTransform = MatrixConvert(matTransform) ;

	vNormal.x = vMirrorNormal.x*matTransform.m11 + vMirrorNormal.y*matTransform.m21 + vMirrorNormal.z*matTransform.m31 ;
	vNormal.y = vMirrorNormal.x*matTransform.m12 + vMirrorNormal.y*matTransform.m22 + vMirrorNormal.z*matTransform.m32 ;
	vNormal.z = vMirrorNormal.x*matTransform.m13 + vMirrorNormal.y*matTransform.m23 + vMirrorNormal.z*matTransform.m33 ;

	for(int i=0 ; i<4 ; i++)
	{
		vVertex = Vector3(m_sMirrorPlane.sVertex[i].pos.x, m_sMirrorPlane.sVertex[i].pos.y, m_sMirrorPlane.sVertex[i].pos.z) ;
		vVertex *= matTransform ;
		m_sVertex[i].pos = VectorConvert(vVertex) ;
		m_sVertex[i].normal = VectorConvert(vNormal) ;
		m_sVertex[i].tex = m_sMirrorPlane.sVertex[i].tex ;
	}

	m_sIndex[0] = m_sMirrorPlane.sIndex[0] ;
	m_sIndex[1] = m_sMirrorPlane.sIndex[1] ;
}
void CSecretMirror::Render()
{
	if(!m_bRender)
		return ;

	//D3DXMATRIX matOldWord, mat ;
	//m_pd3dDevice->GetTransform(D3DTS_WORLD, &matOldWord) ;
	//mat = MatrixConvert(m_sMirrorPlane.matTransform) ;
	//m_pd3dDevice->SetTransform(D3DTS_WORLD, &mat) ;

	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SMirror_Vertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;

	if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
		m_pd3dDevice->SetTexture(0, m_pTexReflect) ;
	else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)
		m_pd3dDevice->SetTexture(0, m_pTexBehind) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, m_sIndex, D3DFMT_INDEX16, m_sVertex, sizeof(m_sVertex[0])) ;

	if(m_bDrawFrustum)
		m_cFrustum.DrawFrustum(m_pd3dDevice) ;
	if(m_bDrawCoordsys)
		m_cCoordsys.Render() ;

	//m_pd3dDevice->SetTransform(D3DTS_WORLD, &matOldWord) ;
}

void CSecretMirror::_BuildReflectProjection(Matrix4 matTransform, const Vector3 &vMirrorPos, const Matrix4 &matReflectView)
{
    Vector3 vVertex ;
	float fMinX, fMaxX, fMinY, fMaxY, fMinZ, fMaxZ ;

	matTransform.SetTranslation(vMirrorPos) ;
	matTransform *= matReflectView ;
	vVertex = Vector3(m_sMirrorPlane.sVertex[0].pos.x, m_sMirrorPlane.sVertex[0].pos.y, m_sMirrorPlane.sVertex[0].pos.z) ;
	vVertex *= matTransform ;

	fMinX = fMaxX = vVertex.x ;
	fMinY = fMaxY = vVertex.y ;
	fMinZ = fMaxZ = vVertex.z ;

	for(int i=1 ; i<4 ; i++)
	{
		vVertex = Vector3(m_sMirrorPlane.sVertex[i].pos.x, m_sMirrorPlane.sVertex[i].pos.y, m_sMirrorPlane.sVertex[i].pos.z) ;
		vVertex *= matTransform ;

		if(fMinX > vVertex.x)
			fMinX = vVertex.x ;
		if(fMaxX <= vVertex.x)
			fMaxX = vVertex.x ;

		if(fMinY > vVertex.y)
			fMinY = vVertex.y ;
		if(fMaxY <= vVertex.y)
			fMaxY = vVertex.y ;

		if(fMinZ > vVertex.z)
			fMinZ = vVertex.z ;
		if(fMaxZ <= vVertex.z)
			fMaxZ = vVertex.z ;
	}

	float fMoveZ=0.0f ;
	if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
		fMoveZ = m_fClipingMoveZ ;
	else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)
		fMoveZ = -m_fClipingMoveZ ;

	D3DXMATRIX matReflectProj ;
    D3DXMatrixPerspectiveOffCenterLH(&m_matReflectProj, fMinX, fMaxX, fMinY, fMaxY, fMinZ+fMoveZ, 1500) ;
}

void CSecretMirror::_BuildFrustum()
{
	Matrix4 mat4V, mat4P ;

	mat4V = MatrixConvert(m_matReflectView) ;
	mat4P = MatrixConvert(m_matReflectProj) ;
	//mat4VP = mat4V*mat4P ;
	m_cFrustum.BuildFrustum(mat4V, mat4P) ;
}
void CSecretMirror::SetRenderMethod(int nMethod, bool bDepthWater)
{
	m_nRenderMethod = nMethod ;
	if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
	{
		m_sMirrorPlane.BuildMirror(m_sMirrorPlane.matTransform, m_sMirrorPlane.vNormal, m_sMirrorPlane.fWidth, m_sMirrorPlane.fHeight) ;
	}
	else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)
	{
		m_sMirrorPlane.BuildBehindMirror(m_sMirrorPlane.matTransform, m_sMirrorPlane.vNormal, m_sMirrorPlane.fWidth, m_sMirrorPlane.fHeight) ;
		m_bDepthWater = bDepthWater ;
	}
}
HRESULT CSecretMirror::RenderVirtualCamera(
	D3DXMATRIX *pmatView,
	D3DXMATRIX *pmatProj,
	LPDIRECT3DTEXTURE9 pTex,
	SD3DEffect *psEffect,
	CSecretD3DTerrainEffect *pcTerrainEffect,
	CSecretTerrain *pcTerrain,
	//CSecretSkyEffect *pcSkyEffect,
	//CSecretSky *pcSky,
	//CSecretCloudEffect *pcCloudEffect,
	//CSecretCloud *pcCloud,
	CSecretSkyDomeEffect *pcSkyDomeEffect,
	CSecretSkyDome *pcSkyTop,
	CSecretSkyDome *pcSkyLower,
	CSecretRenderingGroup *pcRenderingGroup,
	STrueRenderingObject ***pppsTRObjects,
	int nNumTRObject)
{
	D3DXVECTOR4 vPlane, vprojPlane ;
	D3DXMATRIX inv, wvp, trans ;
    D3DXMATRIX mat, matOldView, matOldProj ;
	D3DVIEWPORT9 OldViewport, Viewport = {0, 0, TEXTURESIZE, TEXTURESIZE, 0.1f, 1.0f} ;
	m_pd3dDevice->GetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matOldProj) ;

	//store the current back buffer and z-buffer
	LPDIRECT3DSURFACE9 pSurfBack, pSurfZBuffer ;
	m_pd3dDevice->GetRenderTarget(0, &pSurfBack) ;
	m_pd3dDevice->GetDepthStencilSurface(&pSurfZBuffer) ;
	m_pd3dDevice->GetViewport(&OldViewport) ;

	//m_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView) ;
	//m_pd3dDevice->SetTransform(D3DTS_PROJECTION, pmatProj) ;

	m_pd3dDevice->SetViewport(&Viewport) ;

	if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
		m_pd3dDevice->SetRenderTarget(0, m_pSurfReflect) ;
	else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)
		m_pd3dDevice->SetRenderTarget(0, m_pSurfBehind) ;

	m_pd3dDevice->SetDepthStencilSurface(m_pSurfZBuffer) ;

	//Render
	//m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L) ;
	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );
	if(SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		//바꿔야 할것들
		//vLookat, vCameraPos, matWorld, matView, matProj
		D3DXVECTOR4 vPrevLookat, vPrevCameraPos, vPrevSunDir ;
		D3DXMATRIX matPrevWorld, matPrevView, matPrevProj, matMirror ;

		vPrevLookat = pcTerrainEffect->m_psEssentialElements->vLookAt ;
		vPrevCameraPos = pcTerrainEffect->m_psEssentialElements->vCameraPos ;
		vPrevSunDir = pcTerrainEffect->m_psEssentialElements->vSunDir ;
		matPrevWorld = pcTerrainEffect->m_psEssentialElements->matWorld ;
		matPrevView = pcTerrainEffect->m_psEssentialElements->matView ;
		matPrevProj = pcTerrainEffect->m_psEssentialElements->matProj ;

		if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
		{
			//Set Mirror
			D3DXMatrixScaling(&matMirror, 1, -1, 1) ;

			//D3DXMATRIX a ;
			//D3DXMatrixIdentity(&a) ;
			//a._42 = -GetCenterPos().y ;
			//matMirror = a * matMirror ;
			//a._42 = GetCenterPos().y ;
			//matMirror *= a ;
			matMirror._42 = GetCenterPos().y * 2.0f;

			pcTerrainEffect->m_psEssentialElements->matWorld *= matMirror ;
			pcTerrainEffect->m_psEssentialElements->vSunDir.x = -pcTerrainEffect->m_psEssentialElements->vSunDir.x ;

			//Set Clip Plane
			vPlane.x = 0.0f ;
			vPlane.y = -1.0f ;
			vPlane.z = 0.0f ;
			vPlane.w = GetCenterPos().y+0.25f ;

			//이걸 노말라이즈라고 하는 이유가 이평면벡터는 방향(0, 1, 0)이 포함되어 있어서 이것을 클리핑공간으로 가져갈려면
			//스케일이 적용되면 안되기때문에 연산을 역행렬하고 전치행렬하면 역행렬로 방향이 반대가 되고 스케일도 반대가 되고
			//전치행렬을 하면 그 상태에서 방향만 원래대로 돌아오게 된다.
			wvp = pcTerrainEffect->m_psEssentialElements->matView*pcTerrainEffect->m_psEssentialElements->matProj ;
			D3DXMatrixInverse(&inv, NULL, &wvp) ;
			D3DXMatrixTranspose(&trans, &inv) ;
			D3DXVec4Transform(&vprojPlane, &vPlane, &trans) ;
		}
		else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)
		{
			vPlane.x = 0.0f ;
			vPlane.y = -1.0f ;
			vPlane.z = 0.0f ;
			vPlane.w = GetCenterPos().y+3.0f ;

			wvp = pcTerrainEffect->m_psEssentialElements->matView*pcTerrainEffect->m_psEssentialElements->matProj ;
			D3DXMatrixInverse(&inv, NULL, &wvp) ;
			D3DXMatrixTranspose(&trans, &inv) ;
			D3DXVec4Transform(&vprojPlane, &vPlane, &trans) ;
		}
		//pcTerrainEffect->m_psEssentialElements->matView = *pmatView ;
		//pcTerrainEffect->m_psEssentialElements->matProj = *pmatProj ;

		if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
		{

			//if(pcSkyEffect->m_pEffect != NULL)//Sky
			//{
			//	pcSkyEffect->m_pEffect->SetTechnique(pcSkyEffect->m_hTechnique) ;
			//	pcSkyEffect->m_pEffect->Begin(NULL, 0) ;

			//	pcSky->SetWorldScale(1024.0f) ;
			//	pcSky->Render(pcSkyEffect) ;
			//	pcSky->SetWorldScale(1.0f) ;

			//	pcSkyEffect->m_pEffect->End() ;
			//}
			//if(pcCloudEffect->m_pEffect != NULL)//Cloud
			//{
			//	pcCloudEffect->m_pEffect->SetTechnique(pcCloudEffect->m_hTechnique) ;
			//	pcCloudEffect->m_pEffect->Begin(NULL, 0) ;

			//	pcCloud->SetWorldScale(1024.0f) ;
			//	pcCloud->Render(pcCloudEffect) ;
			//	pcCloud->SetWorldScale(1.0f) ;

			//	pcCloudEffect->m_pEffect->End() ;
			//}

			if(pcSkyDomeEffect->m_pEffect != NULL)
			{
				pcSkyDomeEffect->m_pEffect->SetTechnique(pcSkyDomeEffect->m_hTechnique) ;
				pcSkyDomeEffect->m_pEffect->Begin(NULL, 0) ;

				pcSkyTop->Render(pcSkyDomeEffect) ;

				pcSkyDomeEffect->m_pEffect->End() ;
			}
			if(pcSkyDomeEffect->m_pEffect != NULL)
			{
				pcSkyDomeEffect->m_pEffect->SetTechnique(pcSkyDomeEffect->m_hTechnique) ;
				pcSkyDomeEffect->m_pEffect->Begin(NULL, 0) ;

				pcSkyLower->Render(pcSkyDomeEffect) ;

				pcSkyDomeEffect->m_pEffect->End() ;
			}

		}

		pcTerrainEffect->m_psEssentialElements->vLookAt = vPrevLookat ;
		pcTerrainEffect->m_psEssentialElements->vCameraPos = D3DXVECTOR4(pmatView->_41, pmatView->_42, pmatView->_43, 1.0f) ;

		if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)
		{
			//D3DXVECTOR4 vPlane(0.0f, -1.0f, 0.0f, GetCenterPos().y+0.25f), vprojPlane ;

			//D3DXMATRIX inv, wvp, trans ;
			////이걸 노말라이즈라고 하는 이유가 이평면벡터는 방향(0, 1, 0)이 포함되어 있어서 이것을 클리핑공간으로 가져갈려면
			////스케일이 적용되면 안되기때문에 연산을 역행렬하고 전치행렬하면 역행렬로 방향이 반대가 되고 스케일도 반대가 되고
			////전치행렬을 하면 그 상태에서 방향만 원래대로 돌아오게 된다.
			//wvp = pcTerrainEffect->m_psEssentialElements->matView*pcTerrainEffect->m_psEssentialElements->matProj ;
			//D3DXMatrixInverse(&inv, NULL, &wvp) ;
			//D3DXMatrixTranspose(&trans, &inv) ;

			//D3DXVec4Transform(&vprojPlane, &vPlane, &trans) ;

			m_pd3dDevice->SetClipPlane(0, vprojPlane) ;
			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0) ;

			DWORD cullMode;
			m_pd3dDevice->GetRenderState(D3DRS_CULLMODE, &cullMode);
			m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW) ;

			if(pcTerrainEffect->m_pEffect != NULL)//Terrain
			{
				pcTerrainEffect->m_pEffect->SetTechnique(pcTerrainEffect->m_hTechnique) ;
				pcTerrainEffect->m_pEffect->Begin(NULL, 0) ;

				pcTerrain->RenderQuadTree(pcTerrainEffect, NULL, this) ;
				pcTerrainEffect->m_pEffect->End() ;
			}
			m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, cullMode) ;
			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0) ;
		}
		else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)
		{
			//D3DXVECTOR4 vPlane(0.0f, -1.0f, 0.0f, GetCenterPos().y+3.0f), vprojPlane ;

			//D3DXMATRIX inv, wvp, trans ;
			////이걸 노말라이즈라고 하는 이유가 이평면벡터는 방향(0, 1, 0)이 포함되어 있어서 이것을 클리핑공간으로 가져갈려면
			////스케일이 적용되면 안되기때문에 연산을 역행렬하고 전치행렬하면 역행렬로 방향이 반대가 되고 스케일도 반대가 되고
			////전치행렬을 하면 그 상태에서 방향만 원래대로 돌아오게 된다.
			//wvp = pcTerrainEffect->m_psEssentialElements->matView*pcTerrainEffect->m_psEssentialElements->matProj ;
			//D3DXMatrixInverse(&inv, NULL, &wvp) ;
			//D3DXMatrixTranspose(&trans, &inv) ;

			//D3DXVec4Transform(&vprojPlane, &vPlane, &trans) ;

			m_pd3dDevice->SetClipPlane(0, vprojPlane) ;
			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0) ;

			if(pcTerrainEffect->m_pEffect != NULL)//Terrain
			{
				pcTerrainEffect->m_pEffect->SetTechnique(pcTerrainEffect->m_hTechnique) ;
				pcTerrainEffect->m_pEffect->Begin(NULL, 0) ;

                D3DXMATRIX mat ;
				D3DXMatrixIdentity(&mat) ;

                //pass 1 : terrain
				//pcTerrain->RenderQuadTree(pcTerrainEffect, &mat, this) ;

				if(m_bDepthWater)
				{
					pcTerrain->AddAttr(CSecretTerrain::ATTR_DEPTHWATER) ;

					//pass 2 : depth of the terrain
					pcTerrain->RenderQuadTree(pcTerrainEffect, NULL, this) ;

					pcTerrain->AddAttr(CSecretTerrain::ATTR_DEPTHWATER, false) ;
				}

				pcTerrainEffect->m_pEffect->End() ;
			}

			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0) ;
		}

        //오브젝트렌더링
		if(m_nRenderMethod == CSecretMirror::RENDER_MIRROR)//반사
		{
			D3DXVECTOR4 a = psEffect->vCameraPos ;
			D3DXMATRIX A = psEffect->matView ;
			D3DXMATRIX B = psEffect->matProj ;
			D3DXMATRIX C = psEffect->matWorld ;

			psEffect->vCameraPos = pcTerrainEffect->m_psEssentialElements->vCameraPos ;
			psEffect->matView = pcTerrainEffect->m_psEssentialElements->matView ;
			psEffect->matProj = pcTerrainEffect->m_psEssentialElements->matProj ;

			//psEffect->matWorld *= matMirror ;
			psEffect->vLightDir.x = -psEffect->vLightDir.x ;

			m_pd3dDevice->SetClipPlane(0, vprojPlane) ;
			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0) ;

			//m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW) ;

			if(nNumTRObject)
			{
				if(psEffect->pEffect != NULL)//Objects
				{
					DWORD dwAlphaTest, dwAlphaFunc, dwAlphaRef ;

					m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
					m_pd3dDevice->GetRenderState(D3DRS_ALPHAFUNC, &dwAlphaFunc) ;
					m_pd3dDevice->GetRenderState(D3DRS_ALPHAREF, &dwAlphaRef) ;

					m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

					//Set Shader
					psEffect->pEffect->SetTechnique(psEffect->hTechnique) ;
					psEffect->pEffect->Begin(NULL, 0) ;

					int nNumLOD ;
					STrueRenderingObject *psTRObject, **ppsTRObjects = *pppsTRObjects ;
					CSecretMeshObject *psMeshObject ;
					D3DXMATRIX matReflectedWorld ;
					Vector3 p, vCamera(a.x, 0, a.z) ;
					for(int numObj=0 ; numObj<nNumTRObject ; numObj++)
					{
						psTRObject = ppsTRObjects[numObj] ;
						nNumLOD = ppsTRObjects[numObj]->nNumLOD-1 ;
						p.set(psTRObject->matWorld._41, 0, psTRObject->matWorld._43) ;

						if(nNumLOD == 0)//빌보드가 아님
						{
							int nMeshCount ;
							psMeshObject = psTRObject->apcObject[nNumLOD] ;

							if((psTRObject->apcObject[0]->GetAttr() & CSecretMeshObject::ATTR_REFLECTED)//빌보드모델이 아닌 실제폴리곤모델로 비교
								&& ((vCamera-p).Magnitude() < 100.0f))
							{
								psMeshObject->SetWireFrame(false) ;
								matReflectedWorld = ppsTRObjects[numObj]->matWorld * matMirror ;
								for(nMeshCount=0 ; nMeshCount<psMeshObject->GetNumMesh() ; nMeshCount++)
									psMeshObject->Render(psEffect, psMeshObject->GetMesh(nMeshCount), &matReflectedWorld) ;
							}
						}
						else if(nNumLOD == 1)//빌보드
						{
							psMeshObject = psTRObject->apcObject[nNumLOD] ;//빌보드모델 사용

							if((psTRObject->apcObject[0]->GetAttr() & CSecretMeshObject::ATTR_REFLECTED)//빌보드모델이 아닌 실제폴리곤모델로 비교
								&& ((vCamera-p).Magnitude() < 100.0f))
							{
								psMeshObject->SetWireFrame(false) ;
								matReflectedWorld = ppsTRObjects[numObj]->matWorld * matMirror ;
								m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, psMeshObject->m_dwAddAlpha) ;
								psMeshObject->Render(psEffect, psMeshObject->GetMesh(0), &matReflectedWorld) ;
							}
						}
					}
					psEffect->pEffect->End() ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, dwAlphaFunc) ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, dwAlphaRef) ;
				}
			}

			/*
			pcRenderingGroup->m_matMirror = matMirror ;
			//pcRenderingGroup->SetSpecificRenderingArea(Vector3(m_sMirrorPlane.matTransform.m41, 0, m_sMirrorPlane.matTransform.m43), m_fRadius*1.5f) ;
			pcRenderingGroup->SetSpecificRenderingArea(Vector3(m_sMirrorPlane.matTransform.m41, 0, m_sMirrorPlane.matTransform.m43), m_fRadius) ;

			if(psEffect->pEffect != NULL)//Objects
			{
				//Set Shader
				psEffect->pEffect->SetTechnique(psEffect->hTechnique) ;
				psEffect->pEffect->Begin(NULL, 0) ;
				pcRenderingGroup->Render(psEffect, false, (CSecretRenderingRoll::RROLL_ATTR_INSTANTBLENDTOTEST | CSecretRenderingRoll::RROLL_ATTR_MIRRORRENDER)) ;
				psEffect->pEffect->End() ;
			}
			*/

			//m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW) ;
			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0) ;

            psEffect->vCameraPos = a ;
			psEffect->matView = A ;
			psEffect->matProj = B ;
			psEffect->matWorld = C ;
		}
		else if(m_nRenderMethod == CSecretMirror::RENDER_BEHINDMIRROR)//물속 오브젝트 굴절
		{
			vPlane.x = 0.0f ;
			vPlane.y = -1.0f ;
			vPlane.z = 0.0f ;
			vPlane.w = GetCenterPos().y ;
			wvp = pcTerrainEffect->m_psEssentialElements->matView*pcTerrainEffect->m_psEssentialElements->matProj ;
			D3DXMatrixInverse(&inv, NULL, &wvp) ;
			D3DXMatrixTranspose(&trans, &inv) ;
			D3DXVec4Transform(&vprojPlane, &vPlane, &trans) ;

			m_pd3dDevice->SetClipPlane(0, vprojPlane) ;
			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0) ;

			if(nNumTRObject)
			{
				if(psEffect->pEffect != NULL)//Objects
				{
					DWORD dwAlphaTest, dwAlphaFunc, dwAlphaRef ;

					m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
					m_pd3dDevice->GetRenderState(D3DRS_ALPHAFUNC, &dwAlphaFunc) ;
					m_pd3dDevice->GetRenderState(D3DRS_ALPHAREF, &dwAlphaRef) ;

					m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

					//Set Shader
					psEffect->pEffect->SetTechnique(psEffect->hTechnique) ;
					psEffect->pEffect->Begin(NULL, 0) ;

					STrueRenderingObject *psTRObject, **ppsTRObjects = *pppsTRObjects ;
					CSecretMeshObject *psMeshObject ;
					Vector3 p, vCamera(psEffect->vCameraPos.x, 0, psEffect->vCameraPos.z) ;
					for(int numObj=0 ; numObj<nNumTRObject ; numObj++)
					{
						psTRObject = ppsTRObjects[numObj] ;
						psMeshObject = psTRObject->apcObject[ppsTRObjects[numObj]->nNumLOD-1] ;//빌보드모델 사용
						p.set(psTRObject->matWorld._41, 0, psTRObject->matWorld._43) ;

						if((vCamera-p).Magnitude() < 100.0f)
						{
							psMeshObject->SetWireFrame(false) ;
							m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, psMeshObject->m_dwAddAlpha) ;
							psMeshObject->Render(psEffect, psMeshObject->GetMesh(0), &ppsTRObjects[numObj]->matWorld) ;
						}
					}
					psEffect->pEffect->End() ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, dwAlphaFunc) ;
					m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, dwAlphaRef) ;
				}
			}

			/*
			pcRenderingGroup->SetSpecificRenderingArea(Vector3(m_sMirrorPlane.matTransform.m41, 0, m_sMirrorPlane.matTransform.m43), m_fRadius*1.5f) ;

			if(psEffect->pEffect != NULL)//Objects
			{
				//Set Shader
				psEffect->pEffect->SetTechnique(psEffect->hTechnique) ;
				psEffect->pEffect->Begin(NULL, 0) ;
				pcRenderingGroup->Render(psEffect, false, CSecretRenderingRoll::RROLL_ATTR_BEHINDMIRRORRENDER) ;
				psEffect->pEffect->End() ;
			}
			*/

			m_pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0) ;
		}

		pcTerrainEffect->m_psEssentialElements->vLookAt = vPrevLookat ;
		pcTerrainEffect->m_psEssentialElements->vCameraPos = vPrevCameraPos ;
		pcTerrainEffect->m_psEssentialElements->vSunDir = vPrevSunDir ;
		pcTerrainEffect->m_psEssentialElements->matWorld = matPrevWorld ;
		pcTerrainEffect->m_psEssentialElements->matView = matPrevView ;
		pcTerrainEffect->m_psEssentialElements->matProj = matPrevProj ;
	}
	m_pd3dDevice->EndScene() ;

	//Restore RenderTarget
	m_pd3dDevice->SetRenderTarget(0, pSurfBack) ;
	m_pd3dDevice->SetDepthStencilSurface(pSurfZBuffer) ;
	m_pd3dDevice->SetViewport(&OldViewport) ;

	//m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00808080, 1.0f, 0L ) ;

	SAFE_RELEASE(pSurfBack) ;
	SAFE_RELEASE(pSurfZBuffer) ;

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matOldProj) ;

	return S_OK ;
}

HRESULT CSecretMirror::RenderVirtualCamera(D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj, LPDIRECT3DTEXTURE9 pTex, SD3DEffect *psEffect, CSecretTerrain *pcTerrain)
{
    D3DXMATRIX mat, matOldView, matOldProj ;
	D3DVIEWPORT9 OldViewport, Viewport = {0, 0, TEXTURESIZE, TEXTURESIZE, 0.1f, 1.0f} ;
	m_pd3dDevice->GetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->GetTransform(D3DTS_PROJECTION, &matOldProj) ;

	//store the current back buffer and z-buffer
	LPDIRECT3DSURFACE9 pSurfBack, pSurfZBuffer ;
	m_pd3dDevice->GetRenderTarget(0, &pSurfBack) ;
	m_pd3dDevice->GetDepthStencilSurface(&pSurfZBuffer) ;
	m_pd3dDevice->GetViewport(&OldViewport) ;

	//m_pd3dDevice->SetTransform(D3DTS_VIEW, pmatView) ;
	//m_pd3dDevice->SetTransform(D3DTS_PROJECTION, pmatProj) ;

	D3DXMatrixInverse(&mat, NULL, &psEffect->matWorld) ;
    D3DXVECTOR4 vLocalLightPos ;
	D3DXVec4Transform(&vLocalLightPos, &psEffect->vLightPos, &mat) ;
	psEffect->pEffect->SetVector(psEffect->hvLightPos, &vLocalLightPos) ;

	D3DXVECTOR4 vLocalCameraPos(pmatView->_41, pmatView->_42, pmatView->_43, 1.0f) ;
	psEffect->pEffect->SetVector(psEffect->hvCameraPos, &vLocalCameraPos) ;

	mat = psEffect->matWorld * (*pmatView) * (*pmatProj) ;
	psEffect->pEffect->SetMatrix(psEffect->hmatWVP, &mat) ;
	psEffect->pEffect->SetMatrix(psEffect->hmatWIT, &psEffect->matWorld) ;

	m_pd3dDevice->SetViewport(&Viewport) ;

	m_pd3dDevice->SetRenderTarget(0, m_pSurfReflect) ;
	m_pd3dDevice->SetDepthStencilSurface(m_pSurfZBuffer) ;

	//Render
	//m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0L) ;
	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L );

	pcTerrain->SetWireFrame(false) ;
	pcTerrain->RenderAllOctTree(psEffect) ;

	//Restore RenderTarget
	m_pd3dDevice->SetRenderTarget(0, pSurfBack) ;
	m_pd3dDevice->SetDepthStencilSurface(pSurfZBuffer) ;
	m_pd3dDevice->SetViewport(&OldViewport) ;

	m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00808080, 1.0f, 0L ) ;

	SAFE_RELEASE(pSurfBack) ;
	SAFE_RELEASE(pSurfZBuffer) ;

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matOldView) ;
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matOldProj) ;

	return S_OK ;
}

void CSecretMirror::SetMirrorSurface(UINT lNumVertex, SMirror_Vertex *psVertex, UINT lNumIndex, SMirror_Index *psIndex)
{
	UINT i ;

	//MirrorPlane 두번째 버텍스가 tex(0, 0)이다.
	Vector3 vPos(m_sMirrorPlane.sVertex[1].pos.x, m_sMirrorPlane.sVertex[1].pos.y, m_sMirrorPlane.sVertex[1].pos.z) ;

	m_sMirrorSurface.lNumVertex = lNumVertex ;
	m_sMirrorSurface.psVertex = new SMirror_Vertex[lNumVertex] ;
	for(i=0 ; i<m_sMirrorSurface.lNumVertex ; i++)
		m_sMirrorSurface.psVertex[i] = psVertex[i] ;

	m_sMirrorSurface.lNumIndex = lNumIndex ;
	m_sMirrorSurface.psIndex = new SMirror_Index[lNumIndex] ;
	for(i=0 ; i<m_sMirrorSurface.lNumIndex ; i++)
		m_sMirrorSurface.psIndex[i] = psIndex[i] ;
}

Vector3 CSecretMirror::GetPos(UINT lPos)
{
	//return Vector3(m_sVertex[lPos].pos.x, m_sVertex[lPos].pos.y, m_sVertex[lPos].pos.z) ;
	Vector3 pos(m_sMirrorPlane.sVertex[lPos].pos.x, m_sMirrorPlane.sVertex[lPos].pos.y, m_sMirrorPlane.sVertex[lPos].pos.z) ;
	pos *= m_sMirrorPlane.matTransform ;
	return pos ;

	//return Vector3(m_sMirrorPlane.sVertex[lPos].pos.x, m_sMirrorPlane.sVertex[lPos].pos.y, m_sMirrorPlane.sVertex[lPos].pos.z) ;
}

void CSecretMirror::CalculateTexCoordOnMirrorSurface(Matrix4 &matTransform)
{
	//MirrorPlane은 이미 Transform되어져 있다.
}

Vector3 CSecretMirror::ProjectionNormal(Vector3 &vLookat, Vector3 &vNormal)
{
	float fProjLength = vLookat.dot(vNormal)/(vNormal.Magnitude()*vNormal.Magnitude()) ;
	return vNormal*fProjLength ;
}
Vector3 CSecretMirror::GetCenterPos()
{
	return m_sMirrorPlane.GetPos() ;
}
void CSecretMirror::SetMirrorPos(Vector3 vPos)
{
	m_sMirrorPlane.matTransform.m41 = vPos.x ;
	m_sMirrorPlane.matTransform.m42 = vPos.y ;
	m_sMirrorPlane.matTransform.m43 = vPos.z ;
}
void CSecretMirror::SetMirrorSize(int nWidth, int nHeight)
{
	Matrix4 matRot ;
	Vector3 vMirrorPos(m_sMirrorPlane.matTransform.m41, m_sMirrorPlane.matTransform.m42, m_sMirrorPlane.matTransform.m43) ;
	_SetMirrorPlane(vMirrorPos, m_sMirrorPlane.vNormal, (float)nWidth, (float)nHeight, matRot) ;
}
Matrix4 *CSecretMirror::GetmatTransform()
{
	return &m_sMirrorPlane.matTransform ;
}