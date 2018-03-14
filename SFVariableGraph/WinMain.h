#pragma once

#include "MainWndProc.h"
#include "Graph.h"
#include "IdleHandler.h"
#include "SliderPanel.h"
#include "SFVE.h"
#include "VersionID.h"
#include "versionbuildinc.h"
#include <string>
#include <sstream>
#include <Windows.h>

extern SFVG::Graph *ptrgraph;

int CALLBACK WinMain (_In_ HINSTANCE hinst, _In_ HINSTANCE hprevinst, _In_ LPSTR cmdline, _In_ int cmdshow);