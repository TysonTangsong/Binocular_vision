
// Binocular_vision.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CBinocular_visionApp:
// �йش����ʵ�֣������ Binocular_vision.cpp
//

class CBinocular_visionApp : public CWinApp
{
public:
	CBinocular_visionApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CBinocular_visionApp theApp;