// reversi_guiDoc.cpp : CReversi_guiDoc �N���X�̓���̒�`���s���܂��B
//

#include "stdafx.h"
#include "reversi_gui.h"

#include "reversi_guiDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc

IMPLEMENT_DYNCREATE(CReversi_guiDoc, CDocument)

BEGIN_MESSAGE_MAP(CReversi_guiDoc, CDocument)
	//{{AFX_MSG_MAP(CReversi_guiDoc)
		// ���� - ClassWizard �͂��̈ʒu�Ƀ}�b�s���O�p�̃}�N����ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc �N���X�̍\�z/����

CReversi_guiDoc::CReversi_guiDoc()
{
	// TODO: ���̈ʒu�ɂP�x�����Ă΂��\�z�p�̃R�[�h��ǉ����Ă��������B

}

CReversi_guiDoc::~CReversi_guiDoc()
{
}

BOOL CReversi_guiDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: ���̈ʒu�ɍď�����������ǉ����Ă��������B
	// (SDI �h�L�������g�͂��̃h�L�������g���ė��p���܂��B)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc �V���A���C�[�[�V����

void CReversi_guiDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: ���̈ʒu�ɕۑ��p�̃R�[�h��ǉ����Ă��������B
	}
	else
	{
		// TODO: ���̈ʒu�ɓǂݍ��ݗp�̃R�[�h��ǉ����Ă��������B
	}
}

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc �N���X�̐f�f

#ifdef _DEBUG
void CReversi_guiDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CReversi_guiDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc �R�}���h
