// reversi_gui.h : REVERSI_GUI アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_REVERSI_GUI_H__5C324D7A_468B_4F07_97B3_59E73F01D9C2__INCLUDED_)
#define AFX_REVERSI_GUI_H__5C324D7A_468B_4F07_97B3_59E73F01D9C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiApp:
// このクラスの動作の定義に関しては reversi_gui.cpp ファイルを参照してください。
//

class CReversi_guiApp : public CWinApp
{
public:
	CReversi_guiApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CReversi_guiApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション
	//{{AFX_MSG(CReversi_guiApp)
	afx_msg void OnAppAbout();
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_REVERSI_GUI_H__5C324D7A_468B_4F07_97B3_59E73F01D9C2__INCLUDED_)
