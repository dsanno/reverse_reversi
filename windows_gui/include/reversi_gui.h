// reversi_gui.h : REVERSI_GUI �A�v���P�[�V�����̃��C�� �w�b�_�[ �t�@�C��
//

#if !defined(AFX_REVERSI_GUI_H__5C324D7A_468B_4F07_97B3_59E73F01D9C2__INCLUDED_)
#define AFX_REVERSI_GUI_H__5C324D7A_468B_4F07_97B3_59E73F01D9C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // ���C�� �V���{��

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiApp:
// ���̃N���X�̓���̒�`�Ɋւ��Ă� reversi_gui.cpp �t�@�C�����Q�Ƃ��Ă��������B
//

class CReversi_guiApp : public CWinApp
{
public:
	CReversi_guiApp();

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CReversi_guiApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
	//{{AFX_MSG(CReversi_guiApp)
	afx_msg void OnAppAbout();
		// ���� - ClassWizard �͂��̈ʒu�Ƀ����o�֐���ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_REVERSI_GUI_H__5C324D7A_468B_4F07_97B3_59E73F01D9C2__INCLUDED_)
