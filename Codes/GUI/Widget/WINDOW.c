#include "WINDOW.h"
#include "GUI.h"

#define WINDOW_DEF_CAPHEIGHT   20
/* Ĭ�ϵĴ���caption����ɫcaption����ɫ. */
/* �Լ����屳��ɫ. */
#define WINDOW_CAPTION_COLOR1       0x0042789B  /* ����CAPTION���ɫ */
#define WINDOW_CAPTION_COLOR2       0x00FFFFFF  /* ���ڷǻ��ɫ */
#define WINDOW_TITLE_COLOR1         0x00FFFFFF  /* ���ڱ�����ɫ */
#define WINDOW_TITLE_COLOR2         0x00909090  /* ���ڱ�����ɫ */
#define WINDOW_BODY_BKC             0x00FFFFFF  /* ���ڵ�ɫ */

/* ��׼����߿���ɫ���� */
#define WINDOW_EDGE_COLOR           0x002A3033  /* ������ɫ */

static GUI_HWIN __GetClient(WINDOW_Obj *pObj)
{
    return pObj->hClient;
}

/* Window�Ի溯�� */
static void __Paint(WM_HWIN hWin)
{
    int xSize, ySize;
    GUI_RECT Rect;
    WINDOW_Obj *pObj = hWin;

    GUI_GetClientRect(&Rect);
    xSize = Rect.x1 + 1;
    ySize = Rect.y1 + 1;
    
    /* ���Ʊ����� */
    if (pObj != WM_GetActiveWindow()) {
        GUI_SetForeground(WINDOW_CAPTION_COLOR2);
        GUI_SetFontColor(WINDOW_TITLE_COLOR2);
    } else {
        GUI_SetForeground(WINDOW_CAPTION_COLOR1);
        GUI_SetFontColor(WINDOW_TITLE_COLOR1);
    }
    GUI_FillRect(1, 1, xSize - 2, pObj->CaptionHeight - 1);
    /* ���Ʊ��� */
    GUI_SetFont(WIDGET_GetFont(pObj));
    GUI_Val2Rect(&Rect, 2, 1, xSize - 4, pObj->CaptionHeight);
    GUI_DispStringInRect(&Rect, pObj->Title, GUI_ALIGN_VCENTER); /* ��ֱ���� */
    /* ���Ʊ߿� */
    if (pObj != WM_GetActiveWindow()) {
        GUI_SetForeground(WINDOW_TITLE_COLOR2);
    } else {
        GUI_SetForeground(WINDOW_CAPTION_COLOR1);
    }
    GUI_DrawRect(0, 0, xSize, ySize);
}

/* WINDOW���ý��㺯�� */
static void WINDOW_SetFocus(GUI_MESSAGE *pMsg)
{
    WINDOW_Obj *pObj = pMsg->hWin;

    if (pMsg->hWinSrc != pObj && pMsg->hWinSrc != pObj->hClient) {
        pObj->hFocus = pMsg->hWinSrc;
        WM_Invalidate(pObj->hFocus); /* ��ǰ����ؼ���Ч�� */
    }
}

/* ����һ���ؼ�����Ϊ����  */
static void WINDOW_ChangeFocus(WINDOW_Obj *pObj)
{
    WM_Obj *pFocus;

    pFocus = pObj->hFocus;
    if (pFocus && pFocus->hNext) {
        pFocus = pFocus->hNext;
    } else {
        pFocus = WM_HandleToPtr(pObj->hClient)->hFirstChild;
    }
    if (pFocus) {
        WM_SetWindowFocus(pFocus);
    }
}

/* WINDOW�ؼ��Զ��ص����� */
static void __Callback(WM_MESSAGE *pMsg)
{
    WM_CALLBACK *UserCb = ((WINDOW_Obj*)pMsg->hWin)->UserCb;

    switch (pMsg->MsgId) {
    case WM_PAINT:
        WIDGET_Paint(pMsg->hWin);
        break;
    case WM_GET_CLIENT:
        pMsg->Param = (GUI_PARAM)__GetClient(pMsg->hWin);
        return;
    case WM_SET_FOCUS:
        WINDOW_SetFocus(pMsg); /* ���ý��� */
        return;
    case WM_GET_FOCUS:
        pMsg->hWin = ((WINDOW_Obj*)pMsg->hWin)->hFocus;
        return;
    case WM_KILL_FOCUS: /* ע��Window�ؼ��Ľ��㲻�����Լ� */
        WM_Invalidate(pMsg->hWin);
        WM_SendMessage(((WINDOW_Obj*)pMsg->hWin)->hFocus,
            WM_KILL_FOCUS, pMsg->Param);
        break;
    case WM_KEYDOWN:
        if (pMsg->Param == KEY_TAB) {
            WINDOW_ChangeFocus(pMsg->hWin);
       }
        WM_SendMessage(((WINDOW_Obj*)pMsg->hWin)->hFocus,
            WM_KEYDOWN, pMsg->Param);
        break;
    case WM_KEYUP:
        WM_SendMessage(((WINDOW_Obj*)pMsg->hWin)->hFocus,
            WM_KEYUP, pMsg->Param);
        break;
    case WM_BUTTON_RELEASED:
        if (pMsg->hWinSrc == ((WINDOW_Obj*)pMsg->hWin)->hBtn) {
            WM_DeleteWindow(pMsg->hWin);
            return;
        }
        break;
    default:
        WM_DefaultProc(pMsg);
    }
    UserCb = ((WINDOW_Obj*)pMsg->hWin)->UserCb;
    if (UserCb) {
        UserCb(pMsg);
    }
}

/* �ͻ����Ի溯�� */
static void __PaintClient(WM_HWIN hWin)
{
    GUI_RECT Rect;

    GUI_GetClientRect(&Rect);
    /* ���Ʊ��� */
    GUI_SetForeground(WINDOW_BODY_BKC);
    GUI_FillRect(0, 0, Rect.x1 + 1, Rect.y1 + 1);
}

/* �ͻ����ص����� */
static void __ClientCallback(WM_MESSAGE *pMsg)
{
    WM_HWIN hParent;
    WM_CALLBACK *UserCb;

    hParent = WM_GetParentHandle(pMsg->hWin);
    switch (pMsg->MsgId) {
    case WM_PAINT:
        __PaintClient(pMsg->hWin);
        break;
    case WM_SET_FOCUS:
        pMsg->hWin = hParent;
        WINDOW_SetFocus(pMsg); /* ���ý��� */
        return;
    case WM_TP_PRESS: /* ���ƶ����� */
        break;
    case WM_TP_LEAVE: /* ���ƶ����� */
        break;
    default:
        WM_DefaultProc(pMsg);
    }
    UserCb = ((WINDOW_Obj*)hParent)->UserCb;
    if (UserCb) {
        // WM_PAINT��Ϣ�´���Դ���ھ��
        if (pMsg->MsgId == WM_PAINT) {
            pMsg->hWinSrc = pMsg->hWin;
        }
        pMsg->hWin = hParent;
        UserCb(pMsg);
    }
}

/* �رհ�ť�Ի溯�� */
static void __BtnPaint(GUI_HWIN hWin)
{
    GUI_RECT Rect;
	int x0, y0;
	GUI_COLOR lColor = WINDOW_TITLE_COLOR1;

    GUI_GetClientRect(&Rect);
    if (WM_GetActiveWindow() != WM_GetDsektopWindow(hWin)) {
		lColor = WINDOW_TITLE_COLOR2;
    } else if (BUTTON_GetStatus(hWin)) { /* ���� */
		GUI_SetForeground(0x00D04040);
        GUI_FillRect(0, 0, Rect.x1 + 1, Rect.y1 + 1);
    }
    /* ����'X' */
	GUI_SetForeground(lColor);
	x0 = Rect.x0 + (Rect.x1 - Rect.x0 - 9) / 2;
	y0 = Rect.y0 + (Rect.y1 - Rect.y0 - 9) / 2;
	GUI_DrawLine(x0, y0, x0 + 9, y0 + 9);
	GUI_DrawLine(x0, y0 + 9, x0 + 9, y0);
}

// �����ͻ���
static void __CreateClient(WINDOW_Obj *pObj)
{
    int xSize, ySize;
    GUI_RECT *pr = &pObj->Widget.Win.rect;

    xSize = pr->x1 - pr->x0 - 1;
    ySize = pr->y1 - pr->y0 - pObj->CaptionHeight;
    pObj->hClient = WM_CreateWindowAsChild(1, pObj->CaptionHeight,
        xSize, ySize, pObj, 0, WM_NULL_ID, __ClientCallback, 0);
}

/* �رհ�ť */
void __CreateBtn(WINDOW_Obj *pObj)
{
	int xSize, ySize;
	GUI_RECT *r = &pObj->Widget.Win.rect;

	xSize = r->x1 - r->x0 + 1;
	ySize = pObj->CaptionHeight - 2;
	pObj->hBtn = BUTTON_Create(xSize - ySize * 4 / 3 - 1,
		1, ySize * 4 / 3, ySize, pObj, 0, 0);
	WIDGET_SetPaintFunction(pObj->hBtn, __BtnPaint);
    WM_SetTransWindow(pObj->hBtn, 1);
}

/*
 @ �������ڿؼ�
 @ x0:WINDOW�ؼ�����������(����ڸ�����)
 @ y0:WINDOW�ؼ�����������(����ڸ�����)
 @ xSize:WINDOW�ؼ���ˮƽ����
 @ ySize:WINDOW�ؼ�����ֱ�߶�
 @ hParent:�����ھ��
 @ Id:����ID
 @ Flag:����״̬
 @ cb:�û��ص�����ָ��
 **/
WM_HWIN WINDOW_Create(int x0,
    int y0,
    int xSize,
    int ySize,
    WM_HWIN hParent,
    int Id,
    u_8 Style,
    WM_CALLBACK *cb)
{
    WINDOW_Obj *pObj;
    
    GUI_LOCK();
    pObj = WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent,
        Style, Id, __Callback, sizeof(WINDOW_Obj) - sizeof(WM_Obj));
    if (pObj == NULL) {
        GUI_UNLOCK();
        return NULL;
    }
    pObj->hFocus = NULL; /* ��ֹhFocus��Ұָ�� */
    pObj->CaptionHeight = WINDOW_DEF_CAPHEIGHT;  /* �������߶� */
    pObj->UserCb = cb;
    WIDGET_SetPaintFunction(pObj, __Paint);
    WINDOW_SetTitle(pObj, ""); /* ���ó�ʼ�ַ��� */
    WINDOW_SetFont(pObj, &GUI_DEF_FONT);
    __CreateClient(pObj); /* �����ͻ��� */
    __CreateBtn(pObj);
    WM_SendMessage(pObj, WM_CREATED, (GUI_PARAM)NULL);
    /* �������뽹�� */
    pObj->hFocus = WM_HandleToPtr(pObj->hClient)->hFirstChild;
    GUI_UNLOCK();
    return pObj;
}

/* WINDOW���ñ��� */
GUI_RESULT WINDOW_SetTitle(WM_HWIN hWin, const char *str)
{
    if (hWin) {
        ((WINDOW_Obj*)hWin)->Title = (char*)str;
        return GUI_OK;
    }
    return GUI_ERR;
}

/* WINDOW�������� */
GUI_RESULT WINDOW_SetFont(WM_HWIN hWin, GUI_FONT *Font)
{
    WIDGET_SetFont(hWin, Font);
    return GUI_OK;
}