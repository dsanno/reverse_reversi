// reversi_guiView.cpp : CReversi_guiView クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "reversi_gui.h"

#include "reversi_guiDoc.h"
#include "reversi_guiView.h"
#include "board.h"
#include "evaluator.h"
#include "com.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EVALUATOR_FILE "data/eval.dat"
#define OPENING_FILE "data/open.dat"
#define MPC_FILE "data/mpc.dat"
/////////////////////////////////////////////////////////////////////////////
// CReversi_guiView

IMPLEMENT_DYNCREATE(CReversi_guiView, CView)

BEGIN_MESSAGE_MAP(CReversi_guiView, CView)
	//{{AFX_MSG_MAP(CReversi_guiView)
	ON_WM_CREATE()
	ON_COMMAND(ID_PLAY_START, OnPlayStart)
	ON_UPDATE_COMMAND_UI(ID_PLAY_START, OnUpdatePlayStart)
	ON_COMMAND(ID_PLAY_BREAK, OnPlayBreak)
	ON_UPDATE_COMMAND_UI(ID_PLAY_BREAK, OnUpdatePlayBreak)
	ON_COMMAND(ID_PLAY_UNDO, OnPlayUndo)
	ON_UPDATE_COMMAND_UI(ID_PLAY_UNDO, OnUpdatePlayUndo)
	ON_COMMAND(ID_PLAY_BLACK, OnPlayBlack)
	ON_UPDATE_COMMAND_UI(ID_PLAY_BLACK, OnUpdatePlayBlack)
	ON_COMMAND(ID_PLAY_WHITE, OnPlayWhite)
	ON_UPDATE_COMMAND_UI(ID_PLAY_WHITE, OnUpdatePlayWhite)
	ON_COMMAND(ID_PLAY_LEVEL1, OnPlayLevel1)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL1, OnUpdatePlayLevel1)
	ON_COMMAND(ID_PLAY_LEVEL2, OnPlayLevel2)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL2, OnUpdatePlayLevel2)
	ON_COMMAND(ID_PLAY_LEVEL3, OnPlayLevel3)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL3, OnUpdatePlayLevel3)
	ON_COMMAND(ID_PLAY_LEVEL4, OnPlayLevel4)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL4, OnUpdatePlayLevel4)
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_PLAY_LEVEL5, OnPlayLevel5)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL5, OnUpdatePlayLevel5)
	ON_COMMAND(ID_PLAY_LEVEL6, OnPlayLevel6)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL6, OnUpdatePlayLevel6)
	ON_COMMAND(ID_PLAY_LEVEL7, OnPlayLevel7)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL7, OnUpdatePlayLevel7)
	ON_COMMAND(ID_PLAY_LEVEL8, OnPlayLevel8)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LEVEL8, OnUpdatePlayLevel8)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiView クラスの構築/消滅

CReversi_guiView::CReversi_guiView()
: m_State(STATE_BASE), m_Color(BLACK), m_Level(0),
m_Board(NULL), m_Evaluator(NULL), m_Opening(NULL), m_Com(NULL)
{
	// TODO: この場所に構築用のコードを追加してください。
}

CReversi_guiView::~CReversi_guiView()
{
	if (m_Com) {
		Com_Delete(m_Com);
	}
	if (m_Opening) {
		Opening_Delete(m_Opening);
	}
	if (m_Evaluator) {
		Evaluator_Delete(m_Evaluator);
	}
	if (m_Board) {
		Board_Delete(m_Board);
	}
}

BOOL CReversi_guiView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiView クラスの描画

void CReversi_guiView::OnDraw(CDC* pDC)
{
	CReversi_guiDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: この場所にネイティブ データ用の描画コードを追加します。
	int i, j, x, y, x0, y0, w, h;
	CBrush brush_border, brush_bg, brush_black, brush_white, *brush_old;
	CPen pen_border, *pen_old;
	CString str;
	CRect rect;
	GetClientRect(&rect);

	w = rect.Width() / (BOARD_SIZE + 1);
	h = rect.Height() / (BOARD_SIZE + 2);
	if (w < h) {
		h = w;
	} else {
		w = h;
	}
	x0 = (rect.Width() - w * BOARD_SIZE) / 2;
	y0 = (rect.Height() - h * (BOARD_SIZE + 1)) / 2 * 3;

	brush_border.CreateStockObject(BLACK_BRUSH);
	brush_bg.CreateSolidBrush(RGB(64, 224, 224));
	brush_black.CreateStockObject(BLACK_BRUSH);
	brush_white.CreateStockObject(WHITE_BRUSH);

	pen_border.CreateStockObject(BLACK_PEN);

	brush_old = pDC->SelectObject(&brush_border);
	pen_old = pDC->SelectObject(&pen_border);
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);
	// 盤面の描画
	x = x0, y = y0;
	rect.SetRect(x, y, x + w * BOARD_SIZE, y + h * BOARD_SIZE);
	pDC->FillRect(&rect, &brush_bg);
	x = x0, y = y0;
	pDC->SetTextAlign(TA_CENTER | TA_BASELINE);
	for (i = 0; i <= BOARD_SIZE; i++) {
		pDC->MoveTo(x, y);
		pDC->LineTo(x, y + h * BOARD_SIZE);
		if (i < BOARD_SIZE) {
			pDC->TextOut(x + w / 2, y - 2, "ABCDEFGH" + i, 1);
		}
		x += w;
	}
	x = x0, y = y0;
	pDC->SetTextAlign(TA_CENTER | TA_BASELINE);
	for (i = 0; i <= BOARD_SIZE; i++) {
		pDC->MoveTo(x, y);
		pDC->LineTo(x + w * BOARD_SIZE, y);
		if (i < BOARD_SIZE) {
			pDC->TextOut(x - w / 4, y + h / 2 + 6, "12345678" + i, 1);
		}
		y += h;
	}
	x = x0 + w * 2, y = y0 + h * 2;
	pDC->Ellipse(x - 3, y - 3, x + 4, y + 4);
	x = x0 + w * (BOARD_SIZE - 2), y = y0 + h * 2;
	pDC->Ellipse(x - 3, y - 3, x + 4, y + 4);
	x = x0 + w * 2, y = y0 + h * (BOARD_SIZE - 2);
	pDC->Ellipse(x - 3, y - 3, x + 4, y + 4);
	x = x0 + w * (BOARD_SIZE - 2), y = y0 + h * (BOARD_SIZE - 2);
	pDC->Ellipse(x - 3, y - 3, x + 4, y + 4);

	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			x = x0 + (int)(w * (i + 0.1));
			y = y0 + (int)(h * (j + 0.1));
			switch (Board_Disk(m_Board, Board_Pos(i, j))) {
			case BLACK:
				pDC->SelectObject(&brush_black);
				pDC->Ellipse(x, y, x + (int)(w * 0.8), y + (int)(h * 0.8));
				break;
			case WHITE:
				pDC->SelectObject(&brush_white);
				pDC->Ellipse(x, y, x + (int)(w * 0.8), y + (int)(h * 0.8));
				break;
			}
		}
	}

	// 石数と手番の描画
	pDC->SetTextAlign(TA_LEFT | TA_BASELINE);
	if (m_State == STATE_PLAY) {
		if (m_CurrentColor == m_Color) {
			str.Format("●: %2d  ○: %2d  NEXT : %s", Board_CountDisks(m_Board, BLACK), Board_CountDisks(m_Board, WHITE), m_CurrentColor == BLACK?"●":"○");
		} else {
			str.Format("●: %2d  ○: %2d  NEXT : %s 思考中です", Board_CountDisks(m_Board, BLACK), Board_CountDisks(m_Board, WHITE), m_CurrentColor == BLACK?"●":"○");
		}
	} else {
		str.Format("●: %2d  ○: %2d  中断中です", Board_CountDisks(m_Board, BLACK), Board_CountDisks(m_Board, WHITE));
	}
	x = x0; y = y0 - h / 2 - 4;
	pDC->TextOut(x, y, str);
	
	pDC->SelectObject(brush_old);
	pDC->SelectObject(pen_old);
	brush_border.DeleteObject();
	brush_bg.DeleteObject();
	brush_black.DeleteObject();
	brush_white.DeleteObject();
	pen_border.DeleteObject();
}

bool CReversi_guiView::MenuEnabled(UINT id)
{
	bool result;
	switch (m_State) {
	case STATE_BASE:
		switch (id) {
		case ID_PLAY_BREAK:
		case ID_PLAY_UNDO:
			result = false;
			break;
		default:
			result = true;
			break;
		}
		break;
	case STATE_PLAY:
		switch (id) {
		case ID_PLAY_START:
		case ID_PLAY_BLACK:
		case ID_PLAY_WHITE:
		case ID_PLAY_LEVEL1:
		case ID_PLAY_LEVEL2:
		case ID_PLAY_LEVEL3:
		case ID_PLAY_LEVEL4:
		case ID_PLAY_LEVEL5:
		case ID_PLAY_LEVEL6:
		case ID_PLAY_LEVEL7:
		case ID_PLAY_LEVEL8:
			result = false;
			break;
		default:
			result = true;
			break;
		}
		break;
	default:
		result = true;
		break;
	}
	return result;
}

bool CReversi_guiView::MenuChecked(UINT id)
{
	bool result;
	switch (id) {
	case ID_PLAY_BLACK:
		result = (m_Color == BLACK);
		break;
	case ID_PLAY_WHITE:
		result = (m_Color == WHITE);
		break;
	case ID_PLAY_LEVEL1:
		result = (m_Level == 0);
		break;
	case ID_PLAY_LEVEL2:
		result = (m_Level == 1);
		break;
	case ID_PLAY_LEVEL3:
		result = (m_Level == 2);
		break;
	case ID_PLAY_LEVEL4:
		result = (m_Level == 3);
		break;
	case ID_PLAY_LEVEL5:
		result = (m_Level == 4);
		break;
	case ID_PLAY_LEVEL6:
		result = (m_Level == 5);
		break;
	case ID_PLAY_LEVEL7:
		result = (m_Level == 6);
		break;
	case ID_PLAY_LEVEL8:
		result = (m_Level == 7);
		break;
	default:
		result = false;
	}
	return result;
}

void CReversi_guiView::Flip(int x, int y)
{
	int move;
	int value;
	CString str;
	int score;

	if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
		return;
	}
	if (!Board_Flip(m_Board, m_CurrentColor, Board_Pos(x, y))) {
		return;
	}
	m_ColorHistory[m_Turn] = m_CurrentColor;
	m_CurrentColor = Board_OpponentColor(m_CurrentColor);
	m_Turn++;
	while (1) {
		RedrawWindow();
		Sleep(200);
		move = Com_NextMove(m_Com, m_Board, m_CurrentColor, &value);
		if (move != PASS && move != NOMOVE) {
			Board_Flip(m_Board, m_CurrentColor, move);
		}
		m_ColorHistory[m_Turn] = m_CurrentColor;
		m_CurrentColor = Board_OpponentColor(m_CurrentColor);
		m_Turn++;
		if (Board_CanPlay(m_Board, m_CurrentColor)) {
			break;
		}
		m_CurrentColor = Board_OpponentColor(m_CurrentColor);
		if (!Board_CanPlay(m_Board, BLACK) && !Board_CanPlay(m_Board, WHITE)) {
			break;
		}
	}
	RedrawWindow();
	if (!Board_CanPlay(m_Board, BLACK) && !Board_CanPlay(m_Board, WHITE)) {
		if (Board_CountDisks(m_Board, m_Color) == 0) {
			score = -BOARD_SIZE * BOARD_SIZE;
		} else if (Board_CountDisks(m_Board, Board_OpponentColor(m_Color)) == 0) {
			score = BOARD_SIZE * BOARD_SIZE;
		} else {
			score = Board_CountDisks(m_Board, m_Color) - Board_CountDisks(m_Board, Board_OpponentColor(m_Color));
		}
		if (score > 0) {
			str.Format("コンピュータの%d石負けです", score);
		} else if (score < 0 ) {
			str.Format("あなたの%d石負けです", -score);
		} else {
			str.Format("引き分けです");
		}
		MessageBox(str, "reversi", MB_OK);
		m_State = STATE_BASE;
		RedrawWindow();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiView クラスの診断

#ifdef _DEBUG
void CReversi_guiView::AssertValid() const
{
	CView::AssertValid();
}

void CReversi_guiView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CReversi_guiDoc* CReversi_guiView::GetDocument() // 非デバッグ バージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReversi_guiDoc)));
	return (CReversi_guiDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReversi_guiView クラスのメッセージ ハンドラ

int CReversi_guiView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	srand((unsigned)time(NULL));
	m_Board = Board_New();
	m_Evaluator = Evaluator_New();
	m_Opening = Opening_New();
	if (m_Evaluator) {
		Evaluator_Load(m_Evaluator, EVALUATOR_FILE);
	}
	if (m_Opening) {
		Opening_Load(m_Opening, OPENING_FILE);
	}
	if (m_Evaluator && m_Opening) {
		m_Com = Com_New(m_Evaluator, m_Opening);
		Com_SetOpening(m_Com, 1);
		Com_LoadMPCInfo(m_Com, MPC_FILE);
	}
	if (!m_Board || !m_Evaluator || !m_Com) {
		MessageBox("初期化に失敗しました", "ｴﾗｰ", MB_OK);
		return -1;
	}
	return 0;
}

void CReversi_guiView::OnPlayStart() 
{
	int move, value;

	if (MessageBox("新規対局を開始しますか？", "reversi", MB_YESNO) == IDYES) {
		Board_Clear(m_Board);
		m_Turn = 0;
		m_CurrentColor = BLACK;
		switch (m_Level) {
		case 0:
			Com_SetLevel(m_Com, 2, 8, 10);
			break;
		case 1:
			Com_SetLevel(m_Com, 4, 10, 12);
			break;
		case 2:
			Com_SetLevel(m_Com, 6, 12, 14);
			break;
		case 3:
			Com_SetLevel(m_Com, 8, 14, 16);
			break;
		case 4:
			Com_SetLevel(m_Com, 10, 16, 18);
			break;
		case 5:
			Com_SetLevel(m_Com, 12, 16, 18);
			break;
		case 6:
			Com_SetLevel(m_Com, 14, 18, 20);
			break;
		case 7:
			Com_SetLevel(m_Com, 16, 20, 22);
			break;
		default:
			break;
		}
		m_State = STATE_PLAY;
		RedrawWindow();
		if (m_Color == WHITE) {
			move = Com_NextMove(m_Com, m_Board, m_CurrentColor, &value);
			Board_Flip(m_Board, m_CurrentColor, move);
			m_ColorHistory[m_Turn] = m_CurrentColor;
			m_CurrentColor = Board_OpponentColor(m_CurrentColor);
			m_Turn++;
			RedrawWindow();
		}
	}
}

void CReversi_guiView::OnUpdatePlayStart(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_START));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_START));
}

void CReversi_guiView::OnPlayBreak() 
{
	m_State = STATE_BASE;
	RedrawWindow();
}

void CReversi_guiView::OnUpdatePlayBreak(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_BREAK));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_BREAK));
}

void CReversi_guiView::OnPlayUndo() 
{
	// 対局中でなければUndoしない
	if (m_State != STATE_PLAY) {
		return;
	}
	// ２手目以降でなければUndoしない
	if (m_Turn < 2) {
		return;
	}
	// 自分の手番で、かつ着手可能な局面まで戻す
	do {
		m_Turn--;
		Board_Unflip(m_Board);
	} while (m_ColorHistory[m_Turn] != m_Color || !Board_CanPlay(m_Board, m_Color));
	RedrawWindow();
}

void CReversi_guiView::OnUpdatePlayUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_UNDO));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_UNDO));
}

void CReversi_guiView::OnPlayBlack() 
{
	m_Color = BLACK;
}

void CReversi_guiView::OnUpdatePlayBlack(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_BLACK));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_BLACK));
}

void CReversi_guiView::OnPlayWhite() 
{
	m_Color = WHITE;
}

void CReversi_guiView::OnUpdatePlayWhite(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_WHITE));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_WHITE));
}

void CReversi_guiView::OnPlayLevel1() 
{
	m_Level = 0;
}

void CReversi_guiView::OnUpdatePlayLevel1(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL1));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL1));
}

void CReversi_guiView::OnPlayLevel2() 
{
	m_Level = 1;
}

void CReversi_guiView::OnUpdatePlayLevel2(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL2));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL2));
}

void CReversi_guiView::OnPlayLevel3() 
{
	m_Level = 2;
}

void CReversi_guiView::OnUpdatePlayLevel3(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL3));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL3));
}

void CReversi_guiView::OnPlayLevel4() 
{
	m_Level = 3;
}

void CReversi_guiView::OnUpdatePlayLevel4(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL4));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL4));
}

void CReversi_guiView::OnPlayLevel5() 
{
	m_Level = 4;
}

void CReversi_guiView::OnUpdatePlayLevel5(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL5));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL5));
}

void CReversi_guiView::OnPlayLevel6() 
{
	m_Level = 5;
}

void CReversi_guiView::OnUpdatePlayLevel6(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL6));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL6));
}

void CReversi_guiView::OnPlayLevel7() 
{
	m_Level = 6;
}

void CReversi_guiView::OnUpdatePlayLevel7(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL7));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL7));
}

void CReversi_guiView::OnPlayLevel8()
{
	m_Level = 7;
}

void CReversi_guiView::OnUpdatePlayLevel8(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(MenuEnabled(ID_PLAY_LEVEL8));
	pCmdUI->SetCheck(MenuChecked(ID_PLAY_LEVEL8));
}

void CReversi_guiView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	int i, j, x0, y0, w, h;

	if (m_State == STATE_PLAY && m_CurrentColor == m_Color) {
		GetClientRect(&rect);
		w = rect.Width() / (BOARD_SIZE + 1);
		h = rect.Height() / (BOARD_SIZE + 2);
		if (w < h) {
			h = w;
		} else {
			w = h;
		}
		x0 = (rect.Width() - w * BOARD_SIZE) / 2;
		y0 = (rect.Height() - h * (BOARD_SIZE + 1)) / 2 * 3;
		i = (point.x - x0) / w;
		j = (point.y - y0) / h;
		Flip(i, j);
	}
	
	CView::OnLButtonDown(nFlags, point);
}

