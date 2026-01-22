#pragma once
#include "Random/Algorithm/DrunkardWalk/DrunkardWalk.h"

class InputMgr;
class TextureMgr;
class RandomMgr;
class DebugMgr;
class GameMgr;
class Logger;

class Globals
{
public:
	Globals();
	~Globals();

	void Init();
	void Shut();
	void Destroy();

	unsigned int FrameCount;

	GameMgr* GameMgr;
	TextureMgr* TextureMgr;
	DebugMgr* DebugMgr;
	Logger* Console;
	RandomMgr* RandomMgr;
};

extern Globals gData;