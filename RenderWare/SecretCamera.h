#pragma once

#include "def.h"
#include "Matrix.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

class CSecretTerrain ;
class CSecretWaterManager ;

class CSecretCamera
{
public :
	enum CAMERAMOVE { MOVE_FLY=0, MOVE_ONGROUND } ;

private :
    DWORD m_dwFlag, m_dwHowtoMove ;
	Matrix4 m_matCamera ;
    CSecretTerrain *m_pcTerrain ;
	CSecretWaterManager *m_pcWaterManager ;

	Vector3 m_vOriginLookat, m_vOriginUpper ;

	Vector3 m_vPos, m_vLookat, m_vUpper ;
	Quaternion m_OriginQuat, m_ResultQuat ;
	float m_fPitch, m_fYaw, m_fRoll ;

public :
	float m_fStature ;

public :
	CSecretCamera() ;
	~CSecretCamera() ;

	void Initialize(Vector3 &vPos, Vector3 &vLookat, Vector3 &vUpper, CSecretTerrain *pcTerrain=NULL) ;
	Vector3 TransformbyQuat(float fPitch, float fYaw, float fRoll, Vector3 &vPos) ;
	Vector3 TransformbyQuat(Quaternion &quat, Vector3 &vPos) ;
	void BuildViewMatrix(Vector3 &vLookat, Vector3 &vPos) ;
	Matrix4 *GetMatrix() ;
	void SetTerrain(CSecretTerrain *pcTerrain) { m_pcTerrain = pcTerrain; }
	CSecretTerrain *GetTerrain() { return m_pcTerrain; }
	void SetWaterManager(CSecretWaterManager *pcWaterManager) ;

	Vector3 GetPosition() ;
	Vector3 GetLookat() ;
	Vector3 GetUpper() ;

	void SetFlag(DWORD flag, bool enable) ;
	bool IsFlag(DWORD flag) ;

	void SetCameraMove(DWORD move) { m_dwHowtoMove=move; }
	DWORD GetCameraMove() {return m_dwHowtoMove; }

} ;

