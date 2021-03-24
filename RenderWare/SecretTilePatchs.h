#pragma once

#include "SecretQuadTree.h"

#define MAXNUM_TILEPATCH 16

int anPatchs[] =
{
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT | CSecretTile::NEIGHBOR_RIGHT),//0
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT),//1
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_RIGHT),//2
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT | CSecretTile::NEIGHBOR_RIGHT),//3
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_LEFT | CSecretTile::NEIGHBOR_RIGHT),//4
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_RIGHT),//5
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_RIGHT),//6
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT),//7
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_LEFT),//8
	(CSecretTile::NEIGHBOR_LEFT | CSecretTile::NEIGHBOR_RIGHT),//9
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_DOWN),//10
	(CSecretTile::NEIGHBOR_RIGHT),//11
	(CSecretTile::NEIGHBOR_LEFT),//12
	(CSecretTile::NEIGHBOR_DOWN),//13
	(CSecretTile::NEIGHBOR_UP),//14
	0,//15
} ;

//이 순서대로 해야된다.
//TILE_POS {TILEPOS_LEFTTOP=0, TILEPOS_RIGHTTOP, TILEPOS_LEFTBOTTOM, TILEPOS_RIGHTBOTTOM, TILEPOS_TOP, TILEPOS_BOTTOM, TILEPOS_LEFT, TILEPOS_RIGHT, TILEPOS_INSIDE} ;

int anPatch01[] =//up, down, left, right
{
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch02[] =//up, down, left
{
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	0,
} ;

int anPatch03[] =//up, down, right
{
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_DOWN),
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch04[] =//down, left, right
{
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_RIGHT),
	0,
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch05[] =//up, left, right
{
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_UP),
	0,
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch06[] =//up, right
{
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_RIGHT),
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_UP),
	0,
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch07[] =//down, right
{
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_RIGHT),
	0,
	(CSecretTile::NEIGHBOR_DOWN),
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch08[] =//down, left
{
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	(CSecretTile::NEIGHBOR_DOWN | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_DOWN),
	0,
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	0,
} ;

int anPatch09[] =//up, left
{
	(CSecretTile::NEIGHBOR_UP | CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	(CSecretTile::NEIGHBOR_UP),
	0,
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	0,
} ;

int anPatch10[] =//left, right
{
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
	0,
	(CSecretTile::NEIGHBOR_LEFT),
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch11[] =//up, down
{
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_DOWN),
	0,
	0,
	0,
} ;

int anPatch12[] =//right
{
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
	0,
	0,
	(CSecretTile::NEIGHBOR_RIGHT),
	0,
} ;

int anPatch13[] =//left
{
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	0,
	0,
	(CSecretTile::NEIGHBOR_LEFT),
	0,
	0,
} ;

int anPatch14[] =//down
{
	0,
	0,
	(CSecretTile::NEIGHBOR_DOWN),
	(CSecretTile::NEIGHBOR_DOWN),
	0,
	(CSecretTile::NEIGHBOR_DOWN),
	0,
	0,
	0,
} ;

int anPatch15[] =//up
{
	(CSecretTile::NEIGHBOR_UP),
	(CSecretTile::NEIGHBOR_UP),
	0,
	0,
	(CSecretTile::NEIGHBOR_UP),
	0,
	0,
	0,
	0,
} ;

int anPatch16[] =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
} ;

int *apnSubPatchs[] =
{
	anPatch01,
	anPatch02,
	anPatch03,
	anPatch04,
	anPatch05,
	anPatch06,
	anPatch07,
	anPatch08,
	anPatch09,
	anPatch10,
	anPatch11,
	anPatch12,
	anPatch13,
	anPatch14,
	anPatch15,
	anPatch16,
} ;
