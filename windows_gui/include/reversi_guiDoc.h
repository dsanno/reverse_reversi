// reversi_guiDoc.h : CReversi_guiDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_REVERSI_GUIDOC_H__0C035A48_30B5_44E4_8400_E1796CC9AE46__INCLUDED_)
#define AFX_REVERSI_GUIDOC_H__0C035A48_30B5_44E4_8400_E1796CC9AE46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CReversi_guiDoc : public CDocument
{
protected: // シリアライズ機能のみから作成します。
	CReversi_guiDoc();
	DECLARE_DYNCREATE(CReversi_guiDoc)

// アトリビュート
public:

// オペレーション
public:

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CReversi_guiDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CReversi_guiDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CReversi_guiDoc)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_REVERSI_GUIDOC_H__0C035A48_30B5_44E4_8400_E1796CC9AE46__INCLUDED_)
