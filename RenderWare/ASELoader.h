#pragma once

#include "ASEData.h"

//3DMax 에서 추출한 데이타(*.ASE, *.AMD)를 로드해서 class CASEData에 파싱한다.
class CASELoader
{
private :
	int m_nLineCount ;//읽은 만큼 카운트 된다.

public :
	CASELoader() ;
	~CASELoader() ;

	bool ReadASEFile(char *pszFileName, CASEData *pcASEData) ;

	bool ReadSCENE(FILE *pf, SSceneInfo *psSceneInfo) ;

	bool ReadMATERIAL_LIST(FILE *pf, CASEData *pcASEData) ;//Read MaterialList
	bool ReadMATERIAL(FILE *pf, SMaterial *psMaterial) ;
	bool ReadMAP_DIFFUSE(FILE *pf, SMaterial *psMaterial) ;
	bool ReadMAP_BUMP(FILE *pf, SMaterial *psMaterial) ;
	bool ReadMAP_OPACITY(FILE *pf) ;

	bool ReadGEOMOBJECT(FILE *pf, CASEData *pcASEData, int nAttr=0) ;//Read *GEOMOBJECT
	bool ReadNODE_TM(FILE *pf, SMesh *pMesh) ;// Read *GEOMOBJECT{ *NODE_TM
	bool ReadMESH(FILE *pf, SMesh *pMesh) ;// Read *GEOMOBJECT{ *MESH

	bool ReadMESH_VERTEX_LIST(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *MESH { *MESH_VERTEX_LIST
	bool ReadMESH_FACE_LIST(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *MESH { *MESH_FACE_LIST

	bool ReadMESH_TVERTLIST(FILE *pf, SMeshTexCoord *psTexCoord, int nNumTexCoord) ;//Read *GEOMOBJECT{ *MESH { *MESH_TVERTLIST
	bool ReadMESH_TFACELIST(FILE *pf, SMesh *pMesh, SMeshTexCoord *psTexCoord, int nNumTexCoord) ;//Read *GEOMOBJECT{ *MESH { *MESH_TFACELIST

	bool ReadMESH_NORMALS(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *MESH { *MESH_NORMALS

	bool ReadTM_ANIMATION(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *TM_ANIMATION
	bool ReadCONTROL_POS_TRACK(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *TM_ANIMATION { *CONTROL_POS_TRACK
	bool ReadCONTROL_ROT_TRACK(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *TM_ANIMATION { *CONTROL_ROT_TRACK
	bool ReadCONTROL_ROT_TCB(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *TM_ANIMATION { *CONTROL_ROT_TCB

	bool ReadMESH_MAPPINGCHANNEL(FILE *pf) ;

    bool ReadSHAPEOBJECT(FILE *pf, CASEData *pcASEData) ;
	bool ReadSHAPE_LINE(FILE *pf, SShapeLine *psShapeLine) ;
	bool ReadNODE_TM(FILE *pf, SShapeObject *psShapeObject) ;

	bool ReadSKIN(FILE *pf, SMesh *pMesh) ;//Read *GEOMOBJECT{ *MESH { *SKIN

	bool ReadMESH_CVERTLIST(FILE *pf, SMesh *pMesh) ;
	bool ReadMESH_CFACELIST(FILE *pf, SMesh *pMesh) ;

	bool ReadGROUP(FILE *pf, CASEData *pcASEData) ;


} ;