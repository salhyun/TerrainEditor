#include "SecretCamera.h"
#include "SecretTerrain.h"
#include "SecretWater.h"

CSecretCamera::CSecretCamera()
{
	m_fStature = 2.0f ;
	m_dwFlag = m_dwHowtoMove = 0 ;
	m_pcTerrain = NULL ;
	m_pcWaterManager = NULL ;
}

CSecretCamera::~CSecretCamera()
{
}

void CSecretCamera::Initialize(Vector3 &vPos, Vector3 &vLookat, Vector3 &vUpper, CSecretTerrain *pcTerrain)
{
	//Vector3 vRight ;
	//m_vLookat = (vLookat-vPos).Normalize() ;
	//vRight = (vUpper.cross(m_vLookat)).Normalize() ;
	//m_vUpper = m_vLookat.cross(vRight) ;

	//m_vLookat = vLookat ;
	//m_vUpper = vUpper ;
	m_vLookat = m_vOriginLookat = vLookat ;
	m_vUpper = m_vOriginUpper = vUpper ;

	m_vPos = vPos ;
	m_fPitch = m_fYaw = m_fRoll = 0 ;
	m_pcTerrain = pcTerrain ;
}

//void CSecretCamera::TransformbyQuat(float fPitch, float fYaw, float fRoll, Vector3 &vPos)
//{
//	fPitch = 0 ;
//
//	Quaternion qtemp, quat(fPitch*pi/180.0f, fYaw*pi/180.0f, fRoll*pi/180.0f) ;
//	qtemp = m_quat ;
//	m_quat = qtemp*quat ;
//
//	Matrix4 mat ;
//	m_quat.GetMatrix(mat) ;
//	m_matCamera = mat ;
//
//	m_vPos += vPos ;
//}
Vector3 CSecretCamera::TransformbyQuat(float fPitch, float fYaw, float fRoll, Vector3 &vPos)
{
	m_fPitch = fmod(m_fPitch+fPitch, 360.0f) ;
	m_fYaw = fmod(m_fYaw+fYaw, 360.0f) ;
	m_fRoll = fmod(m_fRoll+fRoll, 360.0f) ;

	Quaternion quat(m_fPitch*3.141592f/180.0f, m_fYaw*3.141592f/180.0f, m_fRoll*3.141592f/180.0f) ;
	m_ResultQuat = quat*m_OriginQuat ;

	Vector3 vLookat, vUpper, vRight ;
	m_ResultQuat.RotatebyQuat(vLookat, m_vOriginLookat) ;
	m_ResultQuat.RotatebyQuat(vUpper, m_vOriginUpper) ;
	vRight = vUpper.cross(vLookat) ;

	m_matCamera.Identity() ;

	m_matCamera.m11 = vRight.x ;
	m_matCamera.m12 = vRight.y ;
	m_matCamera.m13 = vRight.z ;

	m_matCamera.m21 = vUpper.x ;
	m_matCamera.m22 = vUpper.y ;
	m_matCamera.m23 = vUpper.z ;

	m_matCamera.m31 = vLookat.x ;
	m_matCamera.m32 = vLookat.y ;
	m_matCamera.m33 = vLookat.z ;

	//vPos ���� ������ �̵��Ÿ� �̰��� ���� ������ȿ��� �̵��� �Ÿ�
	//�׷��� vPos*m_matCamera(������������) �ϸ� ������������� �̵��Ÿ��� ���´�

	vPos = vPos*m_matCamera ;

	m_matCamera.Identity() ;

	//���� ���� ������� �����.
	m_matCamera.m11 = vRight.x ;
	m_matCamera.m21 = vRight.y ;
	m_matCamera.m31 = vRight.z ;

	m_matCamera.m12 = vUpper.x ;
	m_matCamera.m22 = vUpper.y ;
	m_matCamera.m32 = vUpper.z ;

	m_matCamera.m13 = vLookat.x ;
	m_matCamera.m23 = vLookat.y ;
	m_matCamera.m33 = vLookat.z ;

	//vPos = vPos*m_matCamera ;

	g_cDebugMsg.SetDebugMsg(14, "pos(%03.03f, %03.03f, %03.03f)", enumVector(vPos)) ;

	//������������� �̵��Ÿ��� �ȴ�
	m_vPos += vPos ;

	if(m_dwHowtoMove == MOVE_ONGROUND && m_pcTerrain)
	{
		if(m_pcWaterManager)
		{
			m_vPos.y = m_pcTerrain->m_sHeightMap.GetHeightMap(m_vPos.x, m_vPos.z)+m_fStature ;

			geo::SLine sline ;
			Vector3 a = m_vPos+Vector3(0, 500.0f, 0), vIntersect ;
			sline.set(a, Vector3(0, -1, 0), 1000.0f) ;

			if(m_pcWaterManager->Intersect(&sline, vIntersect, true) >= 0)
			{
				if(m_vPos.y < vIntersect.y)
					m_vPos.y = vIntersect.y + m_fStature ;
			}
		}
		else
			m_vPos.y = m_pcTerrain->m_sHeightMap.GetHeightMap(m_vPos.x, m_vPos.z)+m_fStature ;
	}

	m_matCamera.m41 = -vRight.dot(m_vPos) ;
	m_matCamera.m42 = -vUpper.dot(m_vPos) ;
	m_matCamera.m43 = -vLookat.dot(m_vPos) ;
	m_matCamera.m44 = 1.0f ;

	//���Ͱ��� ��ġ���� �Ǵ� ������
	//���� ȸ���� �ϰ� AxisX(m11, m21, m31), AxisY(m12, m22, m32), AxisZ(m13, m23, m33) �ϰ� ���ڿ�
	//��ġ���� ����ϱ� ������ -vRight.dot(m_vPos), -vUpper.dot(m_vPos), -vLookat.dot(m_vPos) �� �ؾ� �ȴ�.
	//����� ����? ȸ���ϰ� �̵��ϰ� ������?

	g_cDebugMsg.SetDebugMsg(15, "Lookat(%03.03f, %03.03f, %03.03f)", enumVector(vLookat)) ;
	g_cDebugMsg.SetDebugMsg(16, "Upper (%03.03f, %03.03f, %03.03f)", enumVector(vUpper)) ;
	g_cDebugMsg.SetDebugMsg(17, "Right (%03.03f, %03.03f, %03.03f)", enumVector(vRight)) ;
	g_cDebugMsg.SetDebugMsg(18, "Pos   (%03.03f, %03.03f, %03.03f)", enumVector(m_vPos)) ;
	g_cDebugMsg.SetDebugMsg(19, "Pitch=%03.03f Yaw=%03.03f Roll=%03.03f\r\n", m_fPitch, m_fYaw, m_fRoll) ;

	return m_vPos ;
}

void CSecretCamera::BuildViewMatrix(Vector3 &vLookat, Vector3 &vPos)
{
	Vector3 up(0, 1, 0) ;

	Vector3 right = up.cross(vLookat) ;
	up = vLookat.cross(right) ;

	m_matCamera.m11 = right.x ;
	m_matCamera.m21 = right.y ;
	m_matCamera.m31 = right.z ;

	m_matCamera.m12 = up.x ;
	m_matCamera.m22 = up.y ;
	m_matCamera.m32 = up.z ;

	m_matCamera.m13 = vLookat.x ;
	m_matCamera.m23 = vLookat.y ;
	m_matCamera.m33 = vLookat.z ;

	m_vPos = vPos ;

	m_matCamera.m41 = -right.dot(m_vPos) ;
	m_matCamera.m42 = -up.dot(m_vPos) ;
	m_matCamera.m43 = -vLookat.dot(m_vPos) ;
	m_matCamera.m44 = 1.0f ;

	m_matCamera.m14 = m_matCamera.m24 = m_matCamera.m34 = 0.0f ;
}

Vector3 CSecretCamera::GetLookat()
{
	//Vector3 vLookat ;
	//m_ResultQuat.RotatebyQuat(vLookat, m_vOriginLookat) ;
	//return vLookat ;

	return Vector3(m_matCamera.m13, m_matCamera.m23, m_matCamera.m33) ;
}

Vector3 CSecretCamera::GetPosition()
{
	return m_vPos ;
}

Vector3 CSecretCamera::GetUpper()
{
	//Vector3 vUpper ;
	//m_ResultQuat.RotatebyQuat(vUpper, m_vOriginUpper) ;
	//return vUpper ;

	return Vector3(m_matCamera.m12, m_matCamera.m22, m_matCamera.m32) ;
}

Matrix4 *CSecretCamera::GetMatrix()
{
	return &m_matCamera ;
}
void CSecretCamera::SetFlag(DWORD flag, bool enable)
{
	if(enable)
		m_dwFlag |= flag ;
	else
		m_dwFlag &= (~flag) ;
}
bool CSecretCamera::IsFlag(DWORD flag)
{
	if(m_dwFlag & flag)
		return true ;
	else
		return false ;
}
void CSecretCamera::SetWaterManager(CSecretWaterManager *pcWaterManager)
{
	m_pcWaterManager = pcWaterManager ;
}

//void CSecretCamera::Transform(float fPitch, float fYaw, float fRoll, Vector3 &vPos)
//{
//	fPitch = fPitch*pi/180.0f ;
//	fYaw = fYaw*pi/180.0f ;
//	fRoll = fRoll*pi/180.0f ;
//
//	_SubTransform(m_vLookat, fPitch, fYaw, fRoll) ;
//	_SubTransform(m_vUpper, fPitch, fYaw, fRoll) ;
//
//	m_vPos += vPos ;
//
//}
//
//void CSecretCamera::_SubTransform(Vector3 &vOut, float fPitch, float fYaw, float fRoll)
//{
//	Quaternion q, qtemp ;
//
//	q.EulerToQuat(fPitch, fYaw, fRoll) ;
//	qtemp = m_quat ;
//	m_quat = q*qtemp ;
//
//	Vector3 vtemp ;
//	m_quat.RotatebyQuat(vtemp, m_vOrgLookat) ;
//
//	vOut = vtemp ;
//}