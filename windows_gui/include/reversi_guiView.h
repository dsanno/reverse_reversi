// reversi_guiView.h : CReversi_guiView �N���X�̐錾����уC���^�[�t�F�C�X�̒�`�����܂��B
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_REVERSI_GUIVIEW_H__FDCFF0B8_52FA_4B9A_AFEF_E598B286166F__INCLUDED_)
#define AFX_REVERSI_GUIVIEW_H__FDCFF0B8_52FA_4B9A_AFEF_E598B286166F__INCLUDED_
#include "board.h"
#include "evaluator.h"
#include "opening.h"
#include "com.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CReversi_guiView : public CView
{
protected: // �V���A���C�Y�@�\�݂̂���쐬���܂��B
	CReversi_guiView();
	DECLARE_DYNCREATE(CReversi_guiView)

// �A�g���r���[�g
public:
	CReversi_guiDoc* GetDocument();

// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CReversi_guiView)
	public:
	virtual void OnDraw(CDC* pDC);  // ���̃r���[��`�悷��ۂɃI�[�o�[���C�h����܂��B
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CReversi_guiView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool MenuEnabled(UINT id);
	bool MenuChecked(UINT id);
	void Flip(int x, int y);
	enum {
		STATE_BASE,
		STATE_PLAY,
		STATE_N
	};
	int m_State;
	int m_Color;
	int m_CurrentColor;
	int m_ColorHistory[BOARD_SIZE * BOARD_SIZE];
	int m_Turn;
	int m_Level;
	Board *m_Board;
	Evaluator *m_Evaluator;
	Opening *m_Opening;
	Com *m_Com;

// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CReversi_guiView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPlayStart();
	afx_msg void OnUpdatePlayStart(CCmdUI* pCmdUI);
	afx_msg void OnPlayBreak();
	afx_msg void OnUpdatePlayBreak(CCmdUI* pCmdUI);
	afx_msg void OnPlayUndo();
	afx_msg void OnUpdatePlayUndo(CCmdUI* pCmdUI);
	afx_msg void OnPlayBlack();
	afx_msg void OnUpdatePlayBlack(CCmdUI* pCmdUI);
	afx_msg void OnPlayWhite();
	afx_msg void OnUpdatePlayWhite(CCmdUI* pCmdUI);
	afx_msg void OnPlayLevel1();
	afx_msg void OnUpdatePlayLevel1(CCmdUI* pCmdUI);
	afx_msg void OnPlayLevel2();
	afx_msg void OnUpdatePlayLevel2(CCmdUI* pCmdUI);
	afx_msg void OnPlayLevel3();
	afx_msg void OnUpdatePlayLevel3(CCmdUI* pCmdUI);
	afx_msg void OnPlayLevel4();
	afx_msg void OnUpdatePlayLevel4(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPlayLevel5();
	afx_msg void OnUpdatePlayLevel5(CCmdUI* pCmdUI);
	afx_msg void OnPlayLevel6();
	afx_msg void OnUpdatePlayLevel6(CCmdUI* pCmdUI);
	afx_msg void OnPlayLevel7();
	afx_msg void OnUpdatePlayLevel7(CCmdUI* pCmdUI);
	afx_msg void OnPlayLevel8();
	afx_msg void OnUpdatePlayLevel8(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // reversi_guiView.cpp �t�@�C�����f�o�b�O���̎��g�p����܂��B
inline CReversi_guiDoc* CReversi_guiView::GetDocument()
   { return (CReversi_guiDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_REVERSI_GUIVIEW_H__FDCFF0B8_52FA_4B9A_AFEF_E598B286166F__INCLUDED_)
