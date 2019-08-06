// reversi_guiDoc.cpp : CReversi_guiDoc クラスの動作の定義を行います。
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
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc クラスの構築/消滅

CReversi_guiDoc::CReversi_guiDoc()
{
	// TODO: この位置に１度だけ呼ばれる構築用のコードを追加してください。

}

CReversi_guiDoc::~CReversi_guiDoc()
{
}

BOOL CReversi_guiDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc シリアライゼーション

void CReversi_guiDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: この位置に保存用のコードを追加してください。
	}
	else
	{
		// TODO: この位置に読み込み用のコードを追加してください。
	}
}

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiDoc クラスの診断

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
// CReversi_guiDoc コマンド
