// OrderDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "POSClient.h"
#include "OrderDlg.h"
#include "NumberInputDlg.h"
#include "VoidReasonDlg.h"
#include "SplitCheckDlg.h"
#include "PayDlg.h"
#include "FunctionDlg.h"
#include "ModifyDlg.h"
#include "StringInputDlg.h"
#include "NotifyKitchenDlg.h"
#include "RemindDishDlg.h"
#include "FloorChooseDlg.h"
#include "FloorDlg.h"
#include "FloorViewDlg.h"
#include "PickCheckDlg.h"
#include "ComboMealDlg.h"
#include "MSGBoxDlg.h"
#include "RemindClassDlg.h"
#include "MenuButton.h"
#include <afxinet.h>

extern UINT CODE_PAGE;
// OrderDlg 对话框
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(OrderDlg, COrderPage)

std::vector<MenuItem> OrderDlg::m_Items;
int OrderDlg::m_nChkNum;
//long OrderDlg::m_lHeadId;
int OrderDlg::m_nQuantity;
int OrderDlg::m_nSeat;
int OrderDlg::ITEM_TEXTSIZE=20;
CString OrderDlg::m_strTblRemark;
#define TOPHEIGHT 120


OrderDlg::OrderDlg(CWnd* pParent /*=NULL*/)
: COrderPage(OrderDlg::IDD)
{
	m_bCloseSearch=FALSE;
	m_nCurrentPage=0;
	m_nPageCount=1;
	m_nQuantity=1;
	m_nSeat=0;
	m_nCurrentClass=0;
	m_nLastActive=0;
	m_bUpdateItems=FALSE;
	m_nVoidState=0;
	m_CheckedClassButton=NULL;
	m_pOrderList=NULL;
	//读取菜品按钮个数的配置
	ITEM_COLUMNSIZE=::GetPrivateProfileInt(_T("POS"),_T("ITEM_COLUMNSIZE"),4,_T(".\\config.ini"));
	if (ITEM_COLUMNSIZE<=0)
		ITEM_COLUMNSIZE=4;
	ITEM_LINESIZE=::GetPrivateProfileInt(_T("POS"),_T("ITEM_LINESIZE"),7,_T(".\\config.ini"));
	if (ITEM_LINESIZE<=0)
		ITEM_LINESIZE=7;
	ITEM_TEXTSIZE=::GetPrivateProfileInt(_T("POS"),_T("ITEM_TEXTSIZE"),20,_T(".\\config.ini"));
	CLASS_TEXTSIZE=::GetPrivateProfileInt(_T("POS"),_T("CLASS_TEXTSIZE"),24,_T(".\\config.ini"));
	m_nPageSize=ITEM_LINESIZE*ITEM_COLUMNSIZE-2;//每页的大小，需扣除翻页按钮
}

OrderDlg::~OrderDlg()
{
	::DeleteObject(m_bpBackgrd);
	//删除已点菜品
	if(m_pOrderList!=NULL)
	{
		while(!m_pOrderList->IsEmpty())
		{
			OrderDetail* item=m_pOrderList->GetTail();
			m_pOrderList->RemoveTail();
			delete item;
		}
		m_searchDlg->DestroyWindow();
		delete m_searchDlg;
	}

	for (std::vector<CButton*>::iterator iter = m_itemButtons.begin(); iter!= m_itemButtons.end();iter++)
	{
		CButton *b = (*iter);
		b->DestroyWindow();
		delete b;
	}
	m_itemButtons.clear();


}
void OrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CPosPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_HINT, m_ctrlHint);
	DDX_Control(pDX, IDC_STATIC_LOG, m_logCtrl);
	DDX_Control(pDX, IDC_STATIC_TIME, m_timeCtrl);
	DDX_Control(pDX, IDC_STATIC_GUEST, m_guestCtrl);
	DDX_Control(pDX, IDC_STATIC_TABLE, m_tableCtrl);
	DDX_Control(pDX, IDC_STATIC_CHECK, m_checkCtrl);
}


BEGIN_MESSAGE_MAP(OrderDlg, CPosPage)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_COMMAND_RANGE(IDC_SLU_BUTTON,IDC_SLU_BUTTON+999,OnSluBnClicked)//SLU
	ON_COMMAND_RANGE(IDC_CLASS_BUTTON,IDC_CLASS_BUTTON+99,OnClassBnClicked)//菜品大的分类
	ON_COMMAND_RANGE(IDC_DYNAMIC_CTRL,IDC_DYNAMIC_CTRL+MAX_ITEMS,OnItemBnClicked)
	ON_COMMAND_RANGE(IDC_BUTTON0,IDC_BUTTON9,OnNumBnClicked)
	ON_BN_CLICKED(IDC_BUTTON_PAY, &OrderDlg::OnBnClickedButtonPay)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &OrderDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &OrderDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &OrderDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_UP, &OrderDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &OrderDlg::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_VOID, &OrderDlg::OnBnClickedButtonVoid)
	ON_BN_CLICKED(IDC_BUTTON_VIEW, &OrderDlg::OnBnClickedButtonView)
	ON_BN_CLICKED(IDC_BUTTON_FUNCTION, &OrderDlg::OnBnClickedButtonFunction)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &OrderDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &OrderDlg::OnBnClickedSendOrder)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER, &OrderDlg::OnBnClickedQuantity)
	ON_BN_CLICKED(IDC_BUTTON_TRANSFER, &OrderDlg::OnBnClickedTransfer)
	ON_BN_CLICKED(IDC_BUTTON_COMBINE, &OrderDlg::OnBnClickedCombine)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &OrderDlg::OnBnClickedAddCheck)
	ON_BN_CLICKED(IDC_BUTTON_PRE, &OrderDlg::OnBnClickedPrepage)
	ON_BN_CLICKED(IDC_BUTTON_MODIFY,&OrderDlg::OnBnClickedModify)
	ON_BN_CLICKED(IDC_BUTTON_TAKEOUT,&OrderDlg::OnBnClickedTakeOut)
	ON_COMMAND_RANGE(IDC_BUTTON_NEXTPAGE,IDC_BUTTON_NEXTPAGE+50,&CPosPage::OnNextPage)
	ON_BN_CLICKED(IDC_BUTTON_OPENITEM, &OrderDlg::OnBnClickedOpenItem)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &OrderDlg::OnBnClickedSearch)
	ON_BN_CLICKED(IDC_BUTTON_REPRICE, &OrderDlg::OnBnClickedRePrice)
	ON_BN_CLICKED(IDC_BUTTON_REMINDDISH, &OrderDlg::OnBnClickedRemindDish)
	ON_BN_CLICKED(IDC_BUTTON_REMINDDISH_CLASS, &OrderDlg::OnBnClickedRemindDishByClass)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_PRINT_CLASS,&OrderDlg::OnBnClickedChangFamilyGroup)
	ON_BN_CLICKED(IDC_BUTTON_ALTERITEM, &OrderDlg::OnBnClickedAlterItem)
	ON_BN_CLICKED(IDC_BUTTON_FREEDISH, &OrderDlg::OnBnClickedFreeDish)
	ON_BN_CLICKED(IDC_BUTTON_NOTIFYKIT, &OrderDlg::OnBnClickedNotifyKitchen)
	ON_BN_CLICKED(IDC_BUTTON_ADDREQUEST, &OrderDlg::OnBnClickedAddRequest)
	ON_BN_CLICKED(IDC_BUTTON_SWAPCOURSE, &OrderDlg::OnBnClickedSwapCourse)
	ON_BN_CLICKED(IDC_BUTTON_PRINTROUND, &OrderDlg::OnBnClickedPrintRound)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, &OrderDlg::OnBnClickedPrePrint)
	ON_BN_CLICKED(IDC_BUTTON_PARTY,&OrderDlg::OnBnClickedButtonParty)
	ON_BN_CLICKED(IDC_BUTTON_TRANSITEM,&OrderDlg::OnBnClickedButtonTransItem)
	ON_BN_CLICKED(IDC_BUTTON_SEATNUM,&OrderDlg::OnBnClickedSeatNum)
	ON_BN_CLICKED(IDC_BUTTON_SELECTALL,&OrderDlg::OnBnClickedSelectAll)
	ON_BN_CLICKED(IDC_BUTTON_TABLE_REMARK,&OrderDlg::OnBnClickedTableRemark)
	ON_BN_CLICKED(IDC_BUTTON_NEXTSEAT,&OrderDlg::OnBnClickedNextSeat)
	ON_MESSAGE(WM_MESSAGE_SEARCH,OnMsgSearch)

	//快餐模式的按钮
	ON_BN_CLICKED(IDC_BUTTON_INCREASE,&OrderDlg::OnBnClickedIncrease)
	ON_BN_CLICKED(IDC_BUTTON_DECREASE,&OrderDlg::OnBnClickedDecrease)
	ON_BN_CLICKED(IDC_BUTTON_REPEAT, &OrderDlg::OnBnClickedRepeat)
	ON_BN_CLICKED(IDC_BUTTON_TACHK, &OrderDlg::OnBnClickedTakeOutChk)
	//ON_BN_CLICKED(IDC_BUTTON_LOGOUT, &OrderDlg::OnBnClickedLogout)
	ON_BN_CLICKED(IDC_BUTTON_DELETEALL, &OrderDlg::OnBnClickedDeleteAll)
	ON_BN_CLICKED(IDC_BUTTON_PENDING, &OrderDlg::OnBnClickedSuspend)
	ON_BN_CLICKED(IDC_BUTTON_PICKCHK, &OrderDlg::OnBnClickedPickCheck)
	ON_BN_CLICKED(IDC_BUTTON_CHKNAME, &PayDlg::OnBnClickedCheckName)
	ON_BN_CLICKED(IDC_BUTTON_TAINFO,&PayDlg::OnBnClickedTakeOutInfo)
	ON_BN_CLICKED(IDC_BUTTON_REFUND,&OrderDlg::OnBnClickedRefund)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_WEBCHK,&FloorViewDlg::OnBnClickedViewWebcheck)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// OrderDlg 消息处理程序
void OrderDlg::OnSetActive()
{
	try
	{
		int bUpdate=FALSE;
		if(theApp.m_bQuickService)
			bUpdate=theLang.UpdatePage(this,_T("IDD_3_QUICK_ORDER"));
		else
			bUpdate=theLang.UpdatePage(this,_T("IDD_3_ORDER"));
		theLang.UpdatePage(m_searchDlg,_T("IDD_STRING"));
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		UpdateInfo(this);
		CPosPage::OnSetActive();
		if (m_bUpdateItems)
		{
			m_bUpdateItems=FALSE;
			m_Items.clear();
			m_nCurrentPage=0;
			m_nPageCount=(int)ceil((float)m_Items.size()/m_nPageSize);
		}
		ShowCurrentPage();
		if (pApp->m_bDirty!=0)
		{//重设状态
			UpdateHint(_T(""));
			m_strTblRemark=_T("");
			m_nVoidState=0;
			m_nSeat=0;
			m_nQuantity=1;
			if(theApp.m_bQuickService&&pApp->m_bDirty!=3)
				OrderDlg::EmptyCheck();
		}
		if (pApp->m_bDirty!=1)
		{//不需要更新内存信息
			if (pApp->m_bDirty==2)
			{
				for (int i=0;i<MAX_CHECKS;i++)
				{
					m_checkDlg[i].m_nStatus=0;
					m_TabCtrl.DisablePage(i,FALSE);
					m_checkDlg[i].m_fPayed=0;
				}
			}
			UpdateCheckData();
			UpdateSencondScreen(0,NULL);
			pApp->m_bDirty=FALSE;
			return;
		}
		else
			pApp->m_bDirty=FALSE;//置为FALSE，并更新内存
		LoadCheckData();
		if (m_pSecond)
		{
			UpdateSencondScreen(m_TabCtrl.GetActivePageIndex(),NULL);
		}
		if(!m_strTAInfo.IsEmpty())
		{
			int pos=m_strTAInfo.Find('\n');
			if(pos<=0)
				pos=m_strTAInfo.GetLength();
			CString showTAInfo=m_strTAInfo.Left(pos);
			UpdateHint(showTAInfo);
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：刷新开单信息
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::UpdateInfo(CWnd* parent)
{
	try
	{
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		//更新信息
		CString tmpMsg,str2;
		CWnd* pCtrl=NULL;
		pCtrl=parent->GetDlgItem(IDC_STATIC_TIME);
		if (pCtrl)
		{
			tmpMsg.Format(_T("%s"),pApp->m_strBeginTime);
			pCtrl->SetWindowText(tmpMsg);
		}
		pCtrl=parent->GetDlgItem(IDC_STATIC_LOG);
		if (pCtrl)
		{
			tmpMsg.Format(_T("%s"),pApp->m_strUserName);
			pCtrl->SetWindowText(tmpMsg);
		}
		pCtrl=parent->GetDlgItem(IDC_STATIC_HINT);
		if(pCtrl)
		{
			if(!m_strTAInfo.IsEmpty())
			{

				int pos=m_strTAInfo.Find('\n');
				if(pos<=0)
					pos=m_strTAInfo.GetLength();
				CString showTAInfo=m_strTAInfo.Left(pos);
				pCtrl->SetWindowText(showTAInfo);
			}
			else
			{
				pCtrl->SetWindowText(_T(""));
			}
		}

		if(theApp.m_bQuickService)
		{
			
			pCtrl=parent->GetDlgItem(IDC_STATIC_GUEST);
			if (pCtrl)
			{
				if (pApp->m_strChkName.GetLength()==0)
				{
					theLang.LoadString(str2,IDS_CHKNUM);
					tmpMsg.Format(_T("%s%d"),str2,pApp->m_nCheckNum);
				}
				else
				{
					theLang.LoadString(str2,IDS_CHKNAME2);
					tmpMsg.Format(_T("%s:%s"),str2,pApp->m_strChkName);
				}
				pCtrl->SetWindowText(tmpMsg);
			}
			pCtrl=parent->GetDlgItem(IDC_STATIC_CHECK);
			if (pCtrl)
			{
				theLang.LoadString(str2,IDS_SERIAL);
				if(pApp->m_nOrderHeadid==0)
					tmpMsg.Format(_T("%s:-/-"),str2);
				else
					tmpMsg.Format(_T("%s:%d"),str2,pApp->m_nOrderHeadid+1);
				pCtrl->SetWindowText(tmpMsg);
			}
			
		}
		else
		{
			pCtrl=parent->GetDlgItem(IDC_STATIC_TABLE);
			if (pCtrl)
			{
				int len = WideCharToMultiByte(CODE_PAGE, 0, pApp->m_strTblName, -1, NULL, 0, NULL, NULL );
				if(len>4)
					pCtrl->SetWindowText(pApp->m_strTblName);
				else
				{
					theLang.LoadString(str2,IDS_TBLNAME);
					tmpMsg.Format(_T("%s:%s"),str2,pApp->m_strTblName);
					pCtrl->SetWindowText(tmpMsg);
				}
			}
			pCtrl=parent->GetDlgItem(IDC_STATIC_CHECK);
			if (pCtrl)
			{
				if (pApp->m_strChkName.GetLength()==0)
				{
					theLang.LoadString(str2,IDS_CHKNUM);
					tmpMsg.Format(_T("%s%d"),str2,pApp->m_nCheckNum);
				}
				else
				{
					theLang.LoadString(str2,IDS_CHKNAME);
					tmpMsg.Format(_T("%s:%s"),str2,pApp->m_strChkName);
				}
				pCtrl->SetWindowText(tmpMsg);
			}

			pCtrl=parent->GetDlgItem(IDC_STATIC_GUEST);
			if (pCtrl)
			{
				theLang.LoadString(str2,IDS_GUESTNUM);
				tmpMsg.Format(_T("%s:%d"),str2,pApp->m_nGuests);
				pCtrl->SetWindowText(tmpMsg);
			}
		}
		
		CRect rc=CRect(0,0,CreatButton::m_nFullWidth,130*CreatButton::m_nFullHeight/768);
		parent->InvalidateRect(rc); 
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
void OrderDlg::OnSize(UINT nType, int cx, int cy)
{
	CPosPage::OnSize(nType, cx, cy);
	cx=CreatButton::m_nFullWidth;
	cy=CreatButton::m_nFullHeight;
	int height=30*cy/768;
	if (m_logCtrl.m_hWnd)
	{
		m_ctrlHint.MoveWindow(60*cx/1024,35,250*cx/1024,height);
		m_ctrlHint.GetWindowRect(&mRectHint);    
		ScreenToClient(&mRectHint);
		int line_y1=65*cy/768;
		height=25*cy/768;
		int line_y2=line_y1+height;
		m_logCtrl.MoveWindow(25*cx/1024,line_y1,95,height);
		m_checkCtrl.MoveWindow(140*cx/1024,line_y1,220*cx/1024,height);
		m_tableCtrl.MoveWindow(25*cx/1024,line_y2,115*cx/1024,height);
		m_guestCtrl.MoveWindow(140*cx/1024,line_y2,115*cx/1024,height);
		m_timeCtrl.MoveWindow(245*cx/1024,line_y2,95*cx/1024,height);

		//小计区域
		m_TabCtrl.MoveWindow(21*cx/1024,120*cy/768,330*cx/1024,(int)(cy*0.73));
	}
	//菜品区域
	CWnd* pCtrl=GetDlgItem(IDC_STATIC);
	if (pCtrl)
	{
		pCtrl->MoveWindow(495*cx/1024,TOPHEIGHT*CreatButton::m_nFullHeight/768,(int)(cx*0.515),(int)(cy*0.6));
	}
	
}
BOOL OrderDlg::OnInitDialog()
{
	CPosPage::InitDialog(_T("Picture\\order.png"));
	if (m_strTmplate.GetLength()==0)
	{
		if(theApp.m_bQuickService)
			m_strTmplate=_T("Page\\IDD_3_QUICK_ORDER.ini");
		else
			m_strTmplate=_T("Page\\IDD_3_ORDER.ini");
	}
	m_btnCtrl.GenaratePage2(m_strTmplate,this);
	m_TabCtrl.Create(TCS_DOWN|WS_CHILD|WS_VISIBLE,CRect(0,0,200,200),this,127);
	for(int i=0;i<MAX_CHECKS;i++)
	{
		m_checkDlg[i].Create(IDD_CHECK,&m_TabCtrl);	
	}
	m_TabCtrl.AddPage(&m_checkDlg[0],_T("1"),NULL);
	//强制调用onsize
	CRect rect2;
	GetParent()->GetWindowRect(&rect2);
	MoveWindow(rect2);
	m_ctrlHint.SetFont(&theApp.m_txtFont);
	m_logCtrl.SetFont(&theApp.m_txtFont);
	m_checkCtrl.SetFont(&theApp.m_txtFont);
	m_tableCtrl.SetFont(&theApp.m_txtFont);
	m_guestCtrl.SetFont(&theApp.m_txtFont);
	m_timeCtrl.SetFont(&theApp.m_txtFont);
	int space_x=5;
	//获取位置
	CRect rect;
	GetDlgItem(IDC_STATIC)->GetWindowRect(rect);
	ScreenToClient(&rect);
	SIZE buttonSize;
	buttonSize.cx=(rect.right-rect.left)/ITEM_COLUMNSIZE-8;
	buttonSize.cy=(rect.bottom-rect.top)/ITEM_LINESIZE-8;

	m_bpButtonItem[0]=Gdiplus::Image::FromFile(_T("Picture\\class\\item.png"));
	m_bpButtonItem[1]=Gdiplus::Image::FromFile(_T("Picture\\class\\item_.png"));
	m_bpButtonSLU[0]=Gdiplus::Image::FromFile(_T("Picture\\class\\slu.png"));
	m_bpButtonSLU[1]=Gdiplus::Image::FromFile(_T("Picture\\class\\slu_.png"));
	m_bpSlodoutItem[0]=Gdiplus::Image::FromFile(_T("Picture\\class\\sold.png"));
	m_bpSlodoutItem[1]=Gdiplus::Image::FromFile(_T("Picture\\class\\sold_.png"));
	
	LOGFONT m_tLogFont;
	memset(&m_tLogFont,0,sizeof(LOGFONT));
	m_tLogFont.lfHeight	= ScaleY(ITEM_TEXTSIZE);
	wcscpy_s(m_tLogFont.lfFaceName, _T("Microsoft YaHei"));
	//创建菜品按钮
	for(int i=0;i<ITEM_LINESIZE;i++)
	{
		POINT topleft;
		topleft.y=rect.top+i*(buttonSize.cy+8);
		CString str2;
		for(int j=0;j<ITEM_COLUMNSIZE;j++)
		{
			int count=i*ITEM_COLUMNSIZE+j;
			topleft.x=rect.left+j*(buttonSize.cx+space_x);
			CMenuButton* pBtn=new CMenuButton;
			pBtn->SetFont(&m_tLogFont);
			//最后2个按钮为翻页按钮
			if (count==m_nPageSize)
			{//前一页
				pBtn->Create(_T(""),WS_CHILD ,CRect(topleft,buttonSize),this,IDC_BUTTON_LEFT);
				pBtn->SetImage(_T("Picture\\bt_left.png"));
			}
			else if (count==m_nPageSize+1)
			{//后一页
				pBtn->Create(_T(""),WS_CHILD ,CRect(topleft,buttonSize),this,IDC_BUTTON_RIGHT);
				pBtn->SetImage(_T("Picture\\bt_right.png"));
			}
			else
			{
				pBtn->Create(_T(""),WS_CHILD ,CRect(topleft,buttonSize),this,IDC_DYNAMIC_CTRL+count);
				pBtn->SetImages(m_bpButtonItem[0],m_bpButtonItem[1],false);
				pBtn->SetTextColor(RGB(255,255,255));
			}
			m_itemButtons.push_back(pBtn);
		}
	}
	//创建主分类按钮
	//如果界面编辑器没有配置，从数据库初始化
	CWnd* pCtrl=GetDlgItem(IDC_CLASS_BUTTON+1);
	if (pCtrl==NULL)
	{
		m_bpButtonClass[0]=Gdiplus::Image::FromFile(_T("Picture\\class\\class.png"));
		m_bpButtonClass[1]=Gdiplus::Image::FromFile(_T("Picture\\class\\class_.png"));
		buttonSize.cx=m_bpButtonClass[0]->GetWidth()*CreatButton::m_nFullWidth/1024;
		buttonSize.cy=m_bpButtonClass[0]->GetHeight()*CreatButton::m_nFullHeight/768;
		try{
			CLASS_LINES=7;
			CRecordset rs( &theDB);
			CString strSQL;
			strSQL.Format(_T("SELECT *,sum(case when second_group_id>=0 then 1 else 0 end)as total FROM ")
				_T("(SELECT * FROM item_main_group WHERE main_group_id<=99 order by second_group_id DESC )T group by main_group_id;"));
			if(rs.Open(CRecordset::snapshot,strSQL))
			{
				POINT topleft;
				topleft.x=375*CreatButton::m_nFullWidth/1024;
				CLASS_LINES=10;
				int space_y=0;
				topleft.y=TOPHEIGHT*CreatButton::m_nFullHeight/768;
				m_tLogFont.lfHeight	= ScaleY(CLASS_TEXTSIZE);
				m_tLogFont.lfWeight =FW_BOLD;
				int count=0;
				while (!rs.IsEOF())
				{
					count++;
					rs.MoveNext();
				}
				if(count<=CLASS_LINES)
					m_uTotalPage=1;
				else
				{
					m_uTotalPage=ceil((count-2*(CLASS_LINES-1))/(CLASS_LINES-2.0))+2;
				}
				rs.MoveFirst();
				//分页，创建翻页按钮
				CRoundButton2* upBtn;
				CRoundButton2* downBtn;
				if (m_uTotalPage>1)
				{
					upBtn=new CRoundButton2;
					upBtn->SetFont(&m_tLogFont);
					upBtn->Create(_T(""),WS_CHILD ,CRect(topleft,buttonSize),this,IDC_BUTTON_UP);
					upBtn->SetImage(_T("Picture\\class\\bt_up.png"));
					m_btnCtrl.m_buttonList.push_back(upBtn);

					topleft.y+=(buttonSize.cy+space_y)*(CLASS_LINES-1);
					downBtn=new CRoundButton2;
					downBtn->SetFont(&m_tLogFont);
					downBtn->Create(_T(""),WS_CHILD ,CRect(topleft,buttonSize),this,IDC_BUTTON_DOWN);
					downBtn->SetImage(_T("Picture\\class\\bt_down.png"));
					m_btnCtrl.m_buttonList.push_back(downBtn);
				}
				CDBVariant variant;
				CString strName;
				int pIndex=0;//在页内的索引
				m_uCurPage=0;
				while(!rs.IsEOF())
				{
					rs.GetFieldValue(_T("main_group_id"),variant);
					if (variant.m_iVal<0||variant.m_iVal>100)
					{
						rs.MoveNext();
						continue;
					}
					topleft.y=pIndex*(buttonSize.cy+space_y)+TOPHEIGHT*CreatButton::m_nFullHeight/768;
					rs.GetFieldValue(_T("main_group_name"),strName);
					CRoundButton2* pBtn=new CRoundButton2;
					pBtn->SetFont(&m_tLogFont);
					pBtn->SetTextColor(RGB(0,0,0),RGB(236,148,65));
					CString strCount;
					rs.GetFieldValue(_T("total"),strCount);
					if(strCount==_T("1"))
					{//只有一个小类
						rs.GetFieldValue(_T("second_group_id"),variant);
						pBtn->Create(strName,WS_CHILD ,CRect(topleft,buttonSize),this,IDC_SLU_BUTTON+variant.m_iVal);
					}
					else
					{
						pBtn->Create(strName,WS_CHILD ,CRect(topleft,buttonSize),this,IDC_CLASS_BUTTON+variant.m_iVal);
					}
					pBtn->SetImages(m_bpButtonClass[0],m_bpButtonClass[1],false);
					m_btnCtrl.m_buttonList.push_back(pBtn);
					m_classButtons.push_back(pBtn);
					rs.MoveNext();
					//下一次的位置
					pIndex++;
					if(m_uTotalPage>1)
					{//有翻页
						if (pIndex==CLASS_LINES-1)
						{
							//如果不是最后一页,插入下一页
							if (m_uCurPage<m_uTotalPage-1)
							{
								m_classButtons.push_back(downBtn);
								m_classButtons.push_back(upBtn);
								pIndex=1;
								m_uCurPage++;
							}
						}
					}
				}
				rs.Close();
				m_uCurPage=0;
				ShowClassPage();
			}

		}catch(...)
		{
		}
	}
	pCtrl=GetDlgItem(IDC_BUTTON_SEARCH);
	if (pCtrl==NULL)//删除搜索按钮同时禁用键盘搜索
		m_bCloseSearch=TRUE;

	m_pOrderList=&((CPOSClientApp*)AfxGetApp())->m_orderList;
	m_searchDlg=new StringInputDlg;
	theLang.LoadString(m_searchDlg->m_strTitle,IDS_SEARCHHINT);
	m_searchDlg->m_bSearchMode=TRUE;
	m_searchDlg->Create(IDD_STRINGINPUT,this);
	m_searchDlg->ShowWindow(SW_HIDE);

// 	m_alterDlg=new AlterItemDlg;
// 	m_alterDlg->Create(IDD_ALTER,this);
// 	m_alterDlg->hParent=this->GetSafeHwnd();
// 	m_alterDlg->ShowWindow(SW_HIDE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
/************************************************************************
* 函数介绍：点击了SLU
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnSluBnClicked(UINT uID)
{
	try
	{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnSluBnClicked nID="<<uID);
		CRoundButton2* pButton2=(CRoundButton2*)GetDlgItem(uID);
		if (pButton2&&pButton2!=m_CheckedClassButton)
		{//切换选中分类
			if(m_CheckedClassButton)
			{
				m_CheckedClassButton->SetCheck(FALSE);
			}
			m_CheckedClassButton=pButton2;
			m_CheckedClassButton->SetCheck(TRUE);
		}
		CString strSQL;
		if(macrosInt[_T("SHOW_ITEM_ID")]==0)
		{
			strSQL.Format(_T("SELECT * FROM (SELECT * FROM menu_item LEFT JOIN menu_item_class ON menu_item.class_id")
				_T("=menu_item_class.item_class_id WHERE slu_id=\'%d\' ORDER BY slu_priority DESC,item_name1)AS T LEFT JOIN bargain_price_item ON T.item_id")
				_T("=bargain_price_item.bargain_item_number ;"),uID-IDC_SLU_BUTTON);
		}
		else
		{
			strSQL.Format(_T("SELECT * FROM (SELECT * FROM menu_item LEFT JOIN menu_item_class ON menu_item.class_id")
				_T("=menu_item_class.item_class_id WHERE slu_id=\'%d\' ORDER BY slu_priority DESC,item_id)AS T LEFT JOIN bargain_price_item ON T.item_id")
				_T("=bargain_price_item.bargain_item_number ;"),uID-IDC_SLU_BUTTON);
		}
		AddMenuItem(strSQL);
		m_nCurrentPage=0;
		m_nPageCount=(int)ceil((float)m_Items.size()/m_nPageSize);
		m_nCurrentClass=uID;
		ShowCurrentPage();
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：根据sql查询添加菜品
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::AddMenuItem(CString& strSQL)
{
	try{
		m_Items.clear();
		OpenDatabase();
		CRecordset rs( &theDB);
		rs.Open( CRecordset::forwardOnly,strSQL);
		while(!rs.IsEOF())
		{
			MenuItem item={0};
			CDBVariant variant;
			CString strVal;
			rs.GetFieldValue(_T("item_name1"), item.item_name1);
			rs.GetFieldValue(_T("item_name2"), item.item_name2);
			rs.GetFieldValue(_T("icon"), item.icon);
			rs.GetFieldValue(_T("item_id"), variant);
			item.item_number=variant.m_lVal;
			rs.GetFieldValue(_T("item_type"), variant);
			item.type=variant.m_iVal;
			if (item.type==ITEM_TEXT)
			{
				m_Items.push_back(item);
				rs.MoveNext();
				continue;
			}
			rs.GetFieldValue(_T("price_1"), strVal);
			item.price=_wtof(strVal);
			rs.GetFieldValue(_T("unit_1"),item.unit);
			//多种规格？
			variant.m_fltVal=0;
			rs.GetFieldValue(_T("price_2"), strVal);
			if(_wtof(strVal)>0)
				item.modify=TRUE;
			variant.m_lVal=0;
			rs.GetFieldValue(_T("required_condiment"), variant);
			item.condiment=variant.m_lVal;
			rs.GetFieldValue(_T("tax_group"), variant);
			item.tax_group=variant.m_lVal;
			variant.m_boolVal=FALSE;
			try{
				rs.GetFieldValue(_T("weight_entry_required"), variant);
				item.weight_required=variant.m_boolVal;
				variant.m_iVal=9;//不配置折扣级别的菜，默认最高级别，所有折扣都可使用
				rs.GetFieldValue(_T("discount_itemizer"), variant);
				item.n_discount_level=variant.m_iVal;
				variant.m_iVal=9;
				rs.GetFieldValue(_T("service_itemizer"), variant);
				item.n_service_level=variant.m_iVal;
				variant.m_iVal=0;
				rs.GetFieldValue(_T("is_time_price"), variant);
				if(variant.m_iVal>0)
				{
					variant.m_iVal=0;
					rs.GetFieldValue(_T("price_per_minute"), variant);
					item.time_accuracy=variant.m_iVal;
				}
			}catch(...)
			{
			}
			//估清内容
			variant.m_iVal=0;
			rs.GetFieldValue(_T("bargain_stype"), variant);
			if (variant.m_iVal==1||variant.m_iVal==3)
			{//估清
				item.bargain_stype=1;
				item.bargain_num_cur=0;
			}
			else if (variant.m_iVal==2)
			{//限量销售
				item.bargain_stype=2;
				variant.m_fltVal=0;
				rs.GetFieldValue(_T("bargain_num_cur"), variant);
				item.bargain_num_cur=variant.m_fltVal;
			}
			m_Items.push_back(item);
			rs.MoveNext();
		}
		rs.Close();
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
		return;
	}
}
/************************************************************************
* 函数介绍：点击了菜品大的分类
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnClassBnClicked(UINT uID)
{
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnClassBnClicked nID="<<uID);
		CRoundButton2* pButton2=(CRoundButton2*)GetDlgItem(uID);
		if (pButton2&&pButton2!=m_CheckedClassButton)
		{//切换选中分类
			if(m_CheckedClassButton)
			{
				m_CheckedClassButton->SetCheck(FALSE);
			}
			m_CheckedClassButton=pButton2;
			m_CheckedClassButton->SetCheck(TRUE);
		}
		m_Items.clear();
		OpenDatabase();
		CRecordset rs( &theDB);
		CString strSQL;//AND condiment_membership IS NULL
		strSQL.Format(_T("SELECT * FROM item_main_group  AS T1, descriptors_menu_item_slu AS T2 ")
			_T("WHERE T1.main_group_id=\'%d\' AND T1.second_group_id=T2.dmi_slu_id;"),uID-IDC_CLASS_BUTTON);
		rs.Open( CRecordset::forwardOnly,strSQL);
		while(!rs.IsEOF())
		{
			MenuItem item={0};
			item.type=ITEM_SLU;//SLU
			CDBVariant variant;
			rs.GetFieldValue(_T("dmi_slu_name"), item.item_name1);
			rs.GetFieldValue(_T("second_group_id"), variant);
			item.item_number=variant.m_lVal;
			m_Items.push_back(item);
			rs.MoveNext();
		}
		rs.Close();
		m_nCurrentPage=0;
		m_nPageCount=(int)ceil((float)m_Items.size()/m_nPageSize);
		ShowCurrentPage();
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
		return;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：显示当前页的菜品
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::ShowCurrentPage()
{
	try{
		unsigned int index;
		for(int i=0;i<m_nPageSize;i++)
		{
			index=i+m_nCurrentPage*m_nPageSize;
			CMenuButton* pButton2=(CMenuButton*)GetDlgItem(IDC_DYNAMIC_CTRL+i);
			if(index<m_Items.size())
			{
				CString itemName;
				if (m_Items[index].type>ITEM_SLU&&macrosInt[_T("SHOW_ITEM_ID")]!=0)
				{
					itemName.Format(_T("%d-"),m_Items[index].item_number);
				}
				if(macrosInt[_T("SHOW_ITEM_NAME")]==1){
					itemName.AppendFormat(_T("%s"),m_Items[index].item_name1);
				}
				else if(macrosInt[_T("SHOW_ITEM_NAME")]==2){
					if(m_Items[index].item_name2.IsEmpty())
						itemName.AppendFormat(_T("%s"),m_Items[index].item_name1);
					else
						itemName.AppendFormat(_T("%s"),m_Items[index].item_name2);
				}
				else
					itemName.AppendFormat(_T("%s\n%s"),m_Items[index].item_name1,m_Items[index].item_name2);


				if (m_Items[index].type<=ITEM_SLU)//ITEM_CONDIMENT_GROUP
				{//SLU 颜色
					pButton2->SetImages(m_bpButtonSLU[0],m_bpButtonSLU[1],false);
					pButton2->m_nType=0;
					pButton2->m_strBottom=_T("");
				}
				else
				{
					if (m_Items[index].icon.IsEmpty())
					{
						pButton2->SetImages(m_bpButtonItem[0],m_bpButtonItem[1],false);
						pButton2->m_nType=0;
					}
					else
					{
						Gdiplus::Image* img1=Gdiplus::Image::FromFile(m_Items[index].icon);
						if(img1&&img1->GetLastStatus()==Gdiplus::Ok)
						{
							pButton2->SetImages(img1,NULL,false);
							if(macrosInt[_T("ITEM_ICON_STYLE")]==1)
								pButton2->m_nType=0;
							else
								pButton2->m_nType=1;
						}
						else
						{
							pButton2->SetImages(m_bpButtonItem[0],m_bpButtonItem[1],false);
							pButton2->m_nType=0;
						}
					}
					pButton2->m_strBottom.Format(_T("%s")+theApp.m_decPlace,theApp.CURRENCY_SYMBOL,m_Items[index].price);
				}
				CString strTop;
				if (m_Items[index].bargain_stype!=0)
				{
					strTop.Format(_T("%.0f"),m_Items[index].bargain_num_cur);
					if (m_Items[index].bargain_num_cur<=0&&m_Items[index].icon.IsEmpty())
					{
						pButton2->SetImages(m_bpSlodoutItem[0],m_bpSlodoutItem[1],false);
					}
				}
				else
					strTop=_T("");
				pButton2->SetStrTop(strTop);

				pButton2->SetWindowText(itemName);
				if(!pButton2->IsWindowVisible())
					pButton2->ShowWindow(SW_SHOW);
			}
			else
			{
				pButton2->ShowWindow(SW_HIDE);
			}
		}
		CRoundButton2* pButton2=(CRoundButton2*)GetDlgItem(IDC_BUTTON_LEFT);
		if (m_nCurrentPage>0)
			pButton2->ShowWindow(SW_SHOW);
		else
			pButton2->ShowWindow(SW_HIDE);

		pButton2=(CRoundButton2*)GetDlgItem(IDC_BUTTON_RIGHT);
		if (m_nCurrentPage<m_nPageCount-1)
			pButton2->ShowWindow(SW_SHOW);
		else
			pButton2->ShowWindow(SW_HIDE);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：显示分类按钮的当前页
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::ShowClassPage()
{
	try{
		for(int i=0;i<m_classButtons.size();i++)
		{
			m_classButtons[i]->ShowWindow(SW_HIDE);
		}
		for (int i=m_uCurPage*CLASS_LINES;i<(m_uCurPage+1)*CLASS_LINES&&i<m_classButtons.size();i++)
		{
			m_classButtons[i]->ShowWindow(SW_SHOW);
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：点击了某样菜品
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnItemBnClicked(UINT uID)
{
	OrderDetail* item=NULL;
	try{
		UINT index=m_nCurrentPage*m_nPageSize+(uID-IDC_DYNAMIC_CTRL);
		if (index>=m_Items.size())
		{//越界
			LOG4CPLUS_ERROR(log_pos,"index=["<<index<<"] out of range. m_Items.size()="<<m_Items.size());
			return;
		}
		if (m_Items[index].type==ITEM_SLU)
		{//如果点击的是SLU
			OnSluBnClicked(m_Items[index].item_number+IDC_SLU_BUTTON);
			return;
		}
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		int active=m_TabCtrl.GetActivePageIndex();
		if (m_checkDlg[active].m_nStatus==1)
		{
			POSMessageBox(IDS_NOACTION);
			return;
		}
		//提示沽清
		if (m_Items[index].bargain_stype&&m_Items[index].bargain_num_cur<=0)
		{
			if (macrosInt[_T("NO_SOLDOUT_ITEM")])
			{
				return;
			}
			CString msg,str2;
			theLang.LoadString(str2,IDS_SOLDOUTCTN);
			msg.Format(_T("[%s]%s"),m_Items[index].item_name1,str2);
			if(POSMessageBox(msg,MB_YESNO)!=IDOK)
				return;
		}
		if (m_searchDlg->IsWindowVisible())
		{
			m_searchDlg->SendMessage(WM_COMMAND,IDC_BUTTON_CLEAR,0);
		}

		//添加到新点菜品列表
		item=new OrderDetail;
		memset(item,0,sizeof(OrderDetail));
		item->item_id=m_Items[index].item_number;
		item->n_discount_type=m_Items[index].n_discount_level;
		item->n_service_type=m_Items[index].n_service_level;
		item->n_eattype=pApp->m_nEatType;
		item->n_checkID=active+1;
		item->n_seat_num=m_nSeat;
		item->tax_group=m_Items[index].tax_group;
		LOG4CPLUS_INFO(log_pos,"OnItemBnClicked "<<item->item_id);
		//退款
		if(pApp->m_bRefund)
		{
			if(m_nQuantity>0)
				m_nQuantity*=-1;
			wcsncpy_s(item->authID,pApp->m_strUserID,9);
			wcsncpy_s(item->authUser,pApp->m_strUserName,9);
			int length=pApp->m_strRefundReason.GetLength();
			item->return_reason=new wchar_t[length+1];
			item->return_reason[length]=0;
			wcsncpy_s(item->return_reason,length+1,pApp->m_strRefundReason,length);
		}
		//退菜
		if (m_nVoidState)
		{
			if(m_nVoidState==1)
			{//需输入原因
				CVoidReasonDlg reasonDlg;
				int auth=RequeastAuth(m_strTmpUser,_T("void_item"),1,item->authUser);
				if(auth==0&&(reasonDlg.DoModal()==IDOK))
				{
					//选择退菜原因
				}
				else
				{
					m_nVoidState=0;
					UpdateHint(_T(""));
					goto finally;
				}
				pApp->m_strRefundReason=reasonDlg.m_strReason;
				int length=reasonDlg.m_strReason.GetLength();
				item->return_reason=new wchar_t[length+1];
				item->return_reason[length]=0;
				wcsncpy_s(item->return_reason,length+1,reasonDlg.m_strReason,length);
				if (m_Items[index].condiment>0||m_Items[index].weight_required)
				{//需要配料,保持退菜状态
					//称重产品，修改重量为负
					m_nVoidState=2;
				}
				else if (m_Items[index].type==ITEM_SET)
				{
					m_nVoidState=3;
				}
				else
					m_nVoidState=0;
			}
			else if(m_nVoidState==2||m_nVoidState==3)
			{//从全局数据得到退菜原因
				if(m_nVoidState==2)
					m_nVoidState=0;
				int length=pApp->m_strRefundReason.GetLength();
				item->return_reason=new wchar_t[length+1];
				item->return_reason[length]=0;
				wcsncpy_s(item->return_reason,length+1,pApp->m_strRefundReason,length);
			}
			wcsncpy_s(item->authID,m_strTmpUser,9);
			if(m_nQuantity>0)
				m_nQuantity*=-1;
		}

		item->quantity=m_nQuantity;
		item->item_price=m_Items[index].price;
		try{
			//搜索价格方案，更新成合适的价格
			CString strSQL;
			strSQL.Format(_T("select item_price,is_discount from price_scheme,price_scheme_group where menu_item_id=%d and price_scheme_group.disable=0 and price_scheme.group_id=price_scheme_group.group_id and (place_class=-1")
				_T(" or place_class in (select rvc_class_id from serving_rvc_class where rvc_center_id=%d)) and week&(1<<(DAYOFWEEK(CURDATE())-1))>0")
				_T(" and ((from_time<to_time and CURTIME() BETWEEN from_time AND to_time) or (from_time>=to_time and (from_time<CURTIME() or to_time>CURTIME())))")
				,item->item_id,theApp.m_nRVC);
			CRecordset rs(&theDB);
			if(rs.Open( CRecordset::forwardOnly,strSQL))
			{
				if(!rs.IsEOF())
				{
					CDBVariant variant;
					rs.GetFieldValue(_T("item_price"),variant);
					item->item_price=variant.m_dblVal;
					rs.GetFieldValue(_T("is_discount"),variant);
					if(variant.m_iVal==0)//不可打折
						item->n_discount_type=0;
				}
				rs.Close();
			}
		}catch(...)
		{
		}
		CString strTmp,str2;
		if(m_nSeat>0)
		{
			strTmp.Format(_T("Seat %d"),m_nSeat);
		}
		UpdateHint(strTmp);
		if (m_Items[index].type==ITEM_OPEN)
		{
			SetOpenItem(item);
			return;
		}
		else if (m_Items[index].type==ITEM_CONDIMENT)
		{//当前点击的是配料 (！！！不会运行到)
			OrderDetail* belong=m_checkDlg[active].GetLastItem();
			if (belong==NULL)
			{//没有找到菜品
				POSMessageBox(IDS_NOTALLOWCOND);
				goto finally;
			}
			item->belongto=belong;
			item->n_belong_item=belong->item_id;
			if (m_nQuantity<0)
				item->quantity=-1;
			else
				item->quantity=1;
		}
		else if(m_Items[index].type==ITEM_PRICE)
		{//当前点击是时价菜
			NumberInputDlg dlg;
			theLang.LoadString(dlg.m_strHint,IDS_INPUTPRICE);
			if(dlg.DoModal()==IDOK)
			{
				item->item_price=_wtof(dlg.m_strNum);
			}
			else
			{
				goto finally;
			}
		}
		else if (m_Items[index].type==ITEM_TEXT)
		{//文本信息
			item->b_notprint=TRUE;
			item->quantity=1;
			item->item_price=0;
			item->total_price=0;
			item->n_belong_item=1;
		}

		wcsncpy_s(item->unit,m_Items[index].unit,9);
		wcsncpy_s(item->item_name,m_Items[index].item_name1,63);
		wcsncpy_s(item->item_name2,m_Items[index].item_name2,31);
		if (m_Items[index].modify)
		{//选规格
			ModifyDlg dlg(this);
			dlg.m_item=item;
			if(dlg.DoModal()==IDCANCEL)
				goto finally;
		}
		if (m_Items[index].time_accuracy>0)
		{//计时菜品
			item->time_accuracy=m_Items[index].time_accuracy;
		}
		else if (m_Items[index].weight_required)
		{//是需要称重的菜品,重设数量
			item->weight_required=TRUE;
			int count=3;
			while(count>0)
			{
				count--;
				NumberInputDlg dlg;
				theLang.LoadString(dlg.m_strHint,IDS_INPUTWEIGHT);
				if(dlg.DoModal()==IDOK)
				{
					item->quantity=_wtof(dlg.m_strNum);
					if (item->quantity>MAX_ITEM_QUANTITY)
					{
						POSMessageBox(IDS_TOOMUCH);
						if (count==0)
						{
							goto finally;
						}
						continue;
					}
					//检查是否超出了数量估清
					if (m_Items[index].bargain_stype==2&&m_Items[index].bargain_num_cur>0)
					{
						if (item->quantity>m_Items[index].bargain_num_cur)
						{
							CString msg;
							msg.Format(IDS_SOLDOUTNUM,m_Items[index].bargain_num_cur);
							if (POSMessageBox(msg,MB_YESNO)!=IDOK)
								goto finally;
						}
					}
					if (m_nVoidState||pApp->m_bRefund)
					{
						item->quantity*=-1;
					}
					break;
				}
				else
				{
					goto finally;
				}
			}
		}
		ComputeTotalPrice(item);
		item->bargain_stype=m_Items[index].bargain_stype;
		//设置沽清
		if(m_Items[index].bargain_stype==2&&item->quantity>0)
		{
			m_Items[index].bargain_num_cur=m_Items[index].bargain_num_cur-item->quantity;
			CString strSQL;
			strSQL.Format(_T("UPDATE bargain_price_item SET bargain_num_cur=bargain_num_cur-%f WHERE bargain_item_number=%d")
				,item->quantity,item->item_id);
			theDB.ExecuteSQL(strSQL);
			//刷新显示
			CRoundButton2* pButton2=(CRoundButton2*)GetDlgItem(uID);
			CString strTop;
			strTop.Format(_T("%.0f"),m_Items[index].bargain_num_cur);
			pButton2->SetStrTop(strTop);			
			pButton2->Invalidate();
		}
		if (m_Items[index].type==ITEM_SET)
		{//套餐
			m_nQuantity=1;//恢复数量
			item->b_hascondiment=ITEM_SET;
			ComboMealDlg dlg;
			dlg.pParentItem=item;
			if(dlg.DoModal()==IDCANCEL)
				goto finally;
			else
			{//添加菜品
				m_checkDlg[active].AddOrderItem(item);
				m_pOrderList->AddTail(item);
				UpdateSencondScreen(active,item);
				while(!dlg.m_subOrderList.IsEmpty())
				{
					OrderDetail* item=dlg.m_subOrderList.GetHead();
					dlg.m_subOrderList.RemoveHead();
					m_checkDlg[active].AddOrderItem(item);
					m_pOrderList->AddTail(item);
					UpdateSoldoutInfo(item,-item->quantity);
					UpdateSencondScreen(active,item);
				}
				//刷新显示
				m_checkDlg[active].Invalidate();
				return;
			}
		}
		if(theApp.m_VCR.IsOpen())
		{
			CString strLine;
			CCheckDlg::FormatString(item,strLine,TRUE);
			strLine.Replace('|',' ');
			strLine.Append(_T("\n"));
			theApp.m_VCR.Print(strLine);
		}
		int pNewIndex=m_checkDlg[active].AddOrderItem(item);
		if(pNewIndex>=0)
		{//合并
			delete item;
			item=(OrderDetail *)m_checkDlg[active].m_ctrlDetailList.GetItemDataPtr(pNewIndex);
			UpdateSencondScreen(active,NULL);
			if(theApp.m_bQuickService)
			{
				m_checkDlg[active].m_ctrlDetailList.SetSel(-1,FALSE);
				m_checkDlg[active].m_ctrlDetailList.SetSel(pNewIndex);
			}
		}
		else
		{
			m_pOrderList->AddTail(item);
			UpdateSencondScreen(active,item);
			if(theApp.m_bQuickService)
			{
				m_checkDlg[active].m_ctrlDetailList.SetSel(-1,FALSE);
				m_checkDlg[active].m_ctrlDetailList.SetSel(m_checkDlg[active].m_ctrlDetailList.GetCount()-1);
			}
		}
		theApp.m_cusDisplay.DisplayPrice(item->item_price,item->item_name);

		//需要配料
		if (m_Items[index].condiment>0)
		{
			ShowCondiment(item,m_Items[index].condiment);
		}
		m_nQuantity=1;
		return;//跳过finally 删除
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
	finally:
	{
		delete item;
	}
}


/************************************************************************
* 函数介绍：付款
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedButtonPay()
{
	try{
		//验证是否有收银权限
		CString userid;
		int auth=RequeastAuth(userid,_T("payment"));
		if(auth!=0)
		{
			LOG4CPLUS_WARN(log_pos,"not have payment privilege!");
			return;
		}
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		//关联桌是否合并付款
		while(theApp.m_nPartyId>0)
		{
			CString strSQL;
			strSQL.Format(_T("SELECT * FROM order_head WHERE party_id=%d AND order_head_id<>%d"),theApp.m_nPartyId,theApp.m_nOrderHeadid);
			CRecordset rs(&theDB);
			if(!rs.Open(-1,strSQL))
				break;
			if (rs.GetRecordCount()==0)
				break;
			if(POSMessageBox(IDS_COMBINEPAY,MB_YESNO)!=IDOK)
				break;
			//保存已点的
			SaveOrderToDB(m_pOrderList,&m_checkDlg);
			while(!rs.IsEOF())
			{
				CDBVariant variant;
				rs.GetFieldValue(_T("table_id"),variant);
				strSQL.Format(_T("CALL join_table_1(%d,%d,@r);"),variant.m_lVal,pApp->m_nTableId);
				theDB.ExecuteSQL(strSQL);
				rs.MoveNext();
			}
			rs.Close();
			//刷新数据
			FunctionDlg::ClearPercentDsicServ(theApp.m_nOrderHeadid);
			pApp->m_bDirty=TRUE;
			OnSetActive();
			break;
		}
		//跳转到付款界面
		pApp->m_nActiveCheck=m_TabCtrl.GetActivePageIndex();
		PayDlg* pPayDlg=(PayDlg*) pApp->m_pMain->GetPage(DLG_PAY);
		//更新服务费和已付金额
		for (int i=0;i<MAX_CHECKS;i++)
		{
			pPayDlg->m_checkDlg[i].m_fPayed=m_checkDlg[i].m_fPayed;
			//pPayDlg->m_checkDlg[i].m_fService=m_checkDlg[i].m_fService;
			pPayDlg->m_checkDlg[i].m_nStatus=m_checkDlg[i].m_nStatus;
		}
		pApp->m_pMain->SetActivePage(DLG_PAY);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：取消交易，未保存的菜不保存；若数据库为空，取消该单
仅适用于table service
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedButtonCancel()
{
	if(POSMessageBox(IDS_CONFIRM,MB_YESNO)!=IDOK)
		return;
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedButtonCancel Begin");
	CString strSQL;
	try
	{
		strSQL.Format(_T("CALL flush_order(%d,%d);"),theApp.m_nOrderHeadid,theApp.m_nTableId);
		theDB.ExecuteSQL(strSQL);
		//释放锁
		COrderPage::m_strTAInfo.Empty();
		strSQL.Format(_T("SELECT RELEASE_LOCK('table%d');"),theApp.m_nTableId);
		theDB.ExecuteSQL(strSQL);
	}catch(...)
	{
	}
	//取消，返回到登陆界面
	theApp.m_pMain->SetActivePage(DLG_LOGIN);

}
/************************************************************************
* 函数介绍：注销登录,仅适用于QS
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::QuickOrderLogout()
{
	//注销前先挂单
	OnBnClickedSuspend();
	theApp.m_nOrderHeadid=0;
	if(theApp.m_bQuickOnce)
	{
		theApp.ClearQuickOnce();
		theApp.m_pMain->SetActivePage(DLG_FLOOR);
	}
	else
	{
		theApp.m_pMain->SetActivePage(DLG_LOGIN);
	}
}
/************************************************************************
* 函数介绍：删除当前单 
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedDeleteAll()
{
	if(POSMessageBox(IDS_CANCELTRANS,MB_YESNO)==IDCANCEL)
		return;
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedDeleteAll Begin");
	CVoidReasonDlg reasonDlg;
	CString userid;
	CString strSQL;
	CString strDetail;
	WCHAR username[10];
	int auth=RequeastAuth(userid,_T("void_item"),1,username);
	if (auth!=0)
		return;
	BOOL flag=TRUE;
	while(!m_pOrderList->IsEmpty())
	{
		OrderDetail* item=m_pOrderList->GetTail();
		if (item->n_saved>0)
		{
			if(flag)
			{
				//选择退菜原因
				if(reasonDlg.DoModal()==IDCANCEL)
					return;
				flag=FALSE;
			}
			strSQL.Format(_T("UPDATE order_detail SET is_return_item=1,return_reason=\'%s\',auth_id=\'%s\',return_time=now()")
				_T(",auth_name=\'%s\' WHERE order_detail_id=\'%d\';")
				,reasonDlg.m_strReason,userid,username,item->order_id);
			theDB.ExecuteSQL(strSQL);
			if (item->item_id==ITEM_ID_PAYINFO)
			{
				DeletePay(item->order_id);
			}
			if (item->item_id<0)
			{
				strDetail.AppendFormat(_T("%s "),item->item_name);
			}
			else
			{
				strDetail.AppendFormat(_T("%s(%g) "),item->item_name,item->quantity);
			}
		}

		m_pOrderList->RemoveTail();
		UpdateSoldoutInfo(item,item->quantity);
		delete item;
	}
	//清除actual_amount
	strSQL.Format(_T("UPDATE order_head SET actual_amount=0 WHERE order_head_id=%d"),theApp.m_nOrderHeadid);
	theDB.ExecuteSQL(strSQL);
	strSQL.Format(_T("CALL cut_items(%d,-1);"),theApp.m_nOrderHeadid);
	theDB.ExecuteSQL(strSQL);
	FloorDlg::ReleaseLock(theApp.m_nOrderHeadid);
	if(!strDetail.IsEmpty())
		CPOSClientApp::CriticalLogs(OPR_VOID,strDetail);
	EmptyCheck();
	m_checkDlg[0].m_fPayed=0;
	UpdateInfo(this);
	UpdateCheckData();
	UpdateSencondScreen(0,NULL);//放最后
}
void OrderDlg::EmptyCheck()
{
	theApp.m_nOrderHeadid=0;
	theApp.m_nEatType=TYPE_DINE;
	theApp.m_bRefund=FALSE;
	theApp.m_strChkName.Empty();
	if(bNewTainfo==FALSE)
		COrderPage::m_strTAInfo.Empty();
	theApp.m_nCheckNum=0;
	CTime time=CTime::GetCurrentTime();
	theApp.m_strBeginDate=time.Format(_T("%Y-%m-%d"));
	theApp.m_strBeginTime=time.Format(_T("%H:%M:%S"));
	theApp.m_nCheckNum=m_nChkNum+1;
}
/************************************************************************
* 函数介绍：更新菜品沽清信息
* 输入参数：diff -剩余菜品数量的变化
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::UpdateSoldoutInfo(OrderDetail* item,double diff)
{
	CString strSQL;
	if (item->bargain_stype==2&&item->quantity>0)
	{
		strSQL.Format(_T("UPDATE bargain_price_item SET bargain_num_cur=bargain_num_cur %+f WHERE bargain_item_number=%d AND bargain_stype=2;")
			,diff,item->item_id);
		theDB.ExecuteSQL(strSQL);
		//更新内存数量
		for(int j=0;j<m_Items.size();j++)
		{
			if (m_Items[j].item_number==item->item_id)
			{//内存中有
				m_Items[j].bargain_num_cur+=diff;
				int uID=j+IDC_DYNAMIC_CTRL;
				CRoundButton2* pButton2=(CRoundButton2*)GetDlgItem(uID);
				CString strTop;
				strTop.Format(_T("%.0f"),m_Items[j].bargain_num_cur);
				pButton2->SetStrTop(strTop);				
				pButton2->Invalidate();
				break;
			}
		}
	}
}
/************************************************************************
* 函数介绍：生成order_head并保存到数据库
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
BOOL OrderDlg::SaveOrderHead()
{
	//新开的单，生成唯一的单号
	CString strSQL;
	CRecordset rs(&theDB);
	//将开单信息存入数据库中
	strSQL.Format(_T("CALL update_checknum(%d);"),theApp.m_nDeviceId);
	theDB.ExecuteSQL(strSQL);
	strSQL.Format(_T("SELECT check_num FROM user_workstations WHERE workstations_id=%d;"),theApp.m_nDeviceId);
	rs.Open( CRecordset::forwardOnly,strSQL);
	CDBVariant variant;
	if (!rs.IsEOF()) 
	{
		rs.GetFieldValue((short)0,variant);
		m_nChkNum=variant.m_lVal;
		theApp.m_nCheckNum=m_nChkNum;
	}
	rs.Close();
	//对表加锁，保证原子性
	strSQL.Format(_T("lock table history_order_head READ ,order_head WRITE,order_head AS T2 READ,history_order_head AS T3 READ;"));
	theDB.ExecuteSQL(strSQL);
	long head_id=FloorDlg::GetHeadID(macrosInt[_T("HEAD_BEGIN_ID")]);
//	m_lHeadId=head_id;
	m_strCardID.Replace('\'',' ');
	strSQL.Format(_T("INSERT INTO order_head(order_head_id,check_number,table_id,table_name,check_id,customer_num,order_start_time,eat_type")
		_T(",open_employee_id,open_employee_name,pos_device_id,pos_name,rvc_center_id,rvc_center_name,check_name,customer_name)")
		_T(" VALUES(\'%d\',\'%d\',-1,\'%s\',1,1,NOW(),%d,\'%s\',\'%s\',\'%d\',\'%s\',\'%d\',\'%s\',\'%s\',\'%s\');"),
		head_id,m_nChkNum,theApp.m_strTblName,theApp.m_nEatType,theApp.m_strUserID,theApp.m_strUserName,theApp.m_nDeviceId,
		theApp.m_strDeviceName,theApp.m_nRVC,theApp.m_strRVC,theApp.m_strChkName,m_strCardID);
	theDB.ExecuteSQL(strSQL);
	m_strCardID.Empty();
	//解锁
	strSQL.Format(_T("unlock tables;"));
	theDB.ExecuteSQL(strSQL);
	theApp.m_nOrderHeadid=head_id;
	FloorDlg::GetLock(head_id);
	LOG4CPLUS_DEBUG(log_pos,"OrderDlg::SaveOrderHead "<<head_id);
	return TRUE;
}
/************************************************************************
* 函数介绍：挂起当前点的单 (快餐模式)
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedSuspend()
{
	if (theApp.m_nOrderHeadid==0)
	{
		//没有点东西，返回
		if (m_pOrderList->IsEmpty())
			return;
		if(SaveOrderHead()==FALSE)
			return;
	}
	//保存已点的
	SaveOrderToDB(m_pOrderList,&m_checkDlg);
	if(theApp.m_nAutoSendType==1)
	{
		SendOrder(m_checkDlg,1);
	}
	//开新单
	FloorDlg::ReleaseLock(theApp.m_nOrderHeadid);
	while(!m_pOrderList->IsEmpty())
	{
		OrderDetail* item=m_pOrderList->GetTail();
		m_pOrderList->RemoveTail();
		delete item;
	}
	EmptyCheck();
	m_checkDlg[0].m_fPayed=0;
	UpdateInfo(this);
	UpdateCheckData();
	UpdateSencondScreen(0,NULL);//放最后
}
/************************************************************************
* 函数介绍：快餐模式取单
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedPickCheck()
{
	try{
		if (theApp.m_nOrderHeadid==0)
		{
			if (!m_pOrderList->IsEmpty())
			{
				POSMessageBox(IDS_CANTPICK);
				return;
			}
		}
		else
		{
			SaveOrderToDB(m_pOrderList,&m_checkDlg);
		}
		CPickCheckDlg dlg;
		dlg.m_nFilter=theApp.m_nOrderHeadid;
		if(dlg.DoModal()==IDCANCEL)
			return;
		if(dlg.m_nHeadid<=0)
			return;
		if(FloorDlg::GetLock(dlg.m_nHeadid)==FALSE)
		{//被占用
			return;
		}
		CString strSQL,strVal;
		CRecordset rs(&theDB);
		CDBVariant variant;
		theApp.m_nOrderHeadid=dlg.m_nHeadid;
		//查询单子信息
		strSQL.Format(_T("SELECT * FROM order_head WHERE order_head_id=\'%d\'"),theApp.m_nOrderHeadid);
		if(!rs.Open(-1,strSQL))
			return;
		if(rs.IsEOF())
			return;
		rs.GetFieldValue(_T("check_number"),variant);
		theApp.m_nCheckNum=variant.m_lVal;
		rs.GetFieldValue(_T("check_name"),theApp.m_strChkName);
		variant.m_iVal=0;
		rs.GetFieldValue(_T("eat_type"),variant);
		theApp.m_nEatType=variant.m_iVal;
		rs.GetFieldValue(_T("delivery_info"),strVal);
		COrderPage::m_strTAInfo.Format(_T("%s"),strVal);
		rs.GetFieldValue(_T("actual_amount"),strVal);
		m_checkDlg[0].m_fPayed=_wtof(strVal);
		rs.GetFieldValue(_T("order_start_time"),variant);
		theApp.m_strBeginDate.Format(_T("%04d-%02d-%02d"),variant.m_pdate->year,variant.m_pdate->month,variant.m_pdate->day);
		theApp.m_strBeginTime.Format(_T("%02d:%02d:%02d"),variant.m_pdate->hour,variant.m_pdate->minute,variant.m_pdate->second);
		theApp.m_strEndTime.Empty();
		rs.GetFieldValue(_T("edit_time"),variant);
		if (variant.m_dwType!=DBVT_NULL)
		{//存在edit_time字段，说明是反结帐的单
			rs.GetFieldValue(_T("order_end_time"),theApp.m_strEndTime);
		}
		rs.Close();
		while(!m_pOrderList->IsEmpty())
		{
			OrderDetail* item=m_pOrderList->GetTail();
			m_pOrderList->RemoveTail();
			delete item;
		}
		strSQL.Format(_T("SELECT * FROM order_detail LEFT JOIN menu_item ON menu_item_id=item_id WHERE order_head_id=\'%d\' AND is_return_item=0")
			,theApp.m_nOrderHeadid);
		theApp.m_bRefund=FALSE;
		FloorDlg::GetOrderDetail(strSQL,m_pOrderList);
		UpdateInfo(this);
		UpdateCheckData();
		UpdateSencondScreen(0,NULL);
	}catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		AfxMessageBox(e->m_strError,MB_ICONEXCLAMATION);
		e->Delete();
		return;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：菜品显示翻页
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedButtonLeft()
{
	if(m_nCurrentPage>0)
		m_nCurrentPage--;
	else
		return;
	ShowCurrentPage();
}

void OrderDlg::OnBnClickedButtonRight()
{
	if(m_nCurrentPage<m_nPageCount-1)
		m_nCurrentPage++;
	else
		return;
	ShowCurrentPage();
}
/************************************************************************
* 函数介绍：分类显示翻页
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedButtonUp()
{
	if(m_uCurPage>0)
		m_uCurPage--;
	else
		return;
	ShowClassPage();
}
void OrderDlg::OnBnClickedButtonDown()
{
	if(m_uCurPage<m_uTotalPage-1)
		m_uCurPage++;
	else
		return;
	ShowClassPage();
}

void OrderDlg::OnBnClickedButtonVoid()
{
	int active=m_TabCtrl.GetActivePageIndex();
	VoidItem(active);
}
void OrderDlg::OnBnClickedButtonView()
{

}
/************************************************************************
* 函数介绍：功能屏
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedButtonFunction()
{
	if (theApp.m_bRefund)
		return;
	if(!theApp.m_bQuickService)
		SaveOrderToDB(m_pOrderList,&m_checkDlg);
	//设置账单状态
	for (int i=0;i<MAX_CHECKS;i++)
	{
		theApp.m_nCheckStatus[i]=m_checkDlg[i].m_nStatus;
	}
	theApp.m_pMain->SetActivePage(DLG_FUNCTION);
	return;
}

/************************************************************************
* 函数介绍：下单，存储到数据库
* 输入参数：
* 输出参数：
* 返回值  ：0	-失败
1	-添加了新菜
2	-未做实际存储
3   -没有新存储菜品
************************************************************************/
int OrderDlg::SaveOrderToDB(CTypedPtrList<CPtrList,OrderDetail *> *pOrderList,CCheckDlg (*m_checkDlg)[MAX_CHECKS])
{
	try
	{
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		OpenDatabase();
		CString strSQL;
		POSITION pos=NULL;
		int maxCheckid=1;
		int bSaved=2;
		COrder_Detail rsDetail(&theDB);
		rsDetail.Open(CRecordset::snapshot, NULL, CRecordset::appendOnly|CRecordset::optimizeBulkAdd);
		for(pos=pOrderList->GetHeadPosition();pos!=NULL;)
		{
			OrderDetail* item=pOrderList->GetNext(pos);
			if (item->n_saved&&item->time_accuracy>0)
			{//计时菜品更新总价
				strSQL.Format(_T("UPDATE order_detail SET quantity=%f ,actual_price=%f WHERE order_detail_id=%d")
					,item->quantity,item->total_price,item->order_id);
				theDB.ExecuteSQL(strSQL);
			}
			if (item->n_saved)
			{
				continue;
			}
			if(item->item_id>0)
				bSaved=1;
			else if (bSaved==2)
				bSaved=3;
			rsDetail.AddNew();
			SetRsDetail(rsDetail,item);
			rsDetail.Update();
			if(maxCheckid<item->n_checkID)//记录最大的checkID
				maxCheckid=item->n_checkID;
			//存储后删除退菜原因
			item->n_saved=1;
			delete item->return_reason;
			item->return_reason=NULL;
			//获取item_id
			strSQL.Format(_T("SELECT LAST_INSERT_ID();"));
			CRecordset rs(&theDB);
			rs.Open(-1,strSQL);
			CString strCount;
			rs.GetFieldValue((short)0,strCount);
			item->order_id=_wtol(strCount);
		}
		rsDetail.Close();
		//在order_head中添加check
		//从数据库获取最大单号 
		strSQL.Format(_T("SELECT MAX(check_id) FROM order_detail WHERE order_head_id=%d;"),pApp->m_nOrderHeadid);
		CRecordset rs(&theDB);
		rs.Open(-1,strSQL);
		CString strCount;
		rs.GetFieldValue((short)0,strCount);
		maxCheckid=_wtoi(strCount);
		rs.Close();
		for(int i=maxCheckid;i>0;i--)
		{
			if((*m_checkDlg)[i-1].m_ctrlDetailList.GetCount()>0)
			{//点了菜的才插入head
				strSQL.Format(_T("SELECT COUNT(*) FROM order_head WHERE order_head_id=\'%d\' AND check_id=\'%d\';"),
					pApp->m_nOrderHeadid,i);
				CRecordset rs( &theDB );
				rs.Open(-1,strSQL);
				CString strCount;
				rs.GetFieldValue((short)0,strCount);
				rs.Close();
				if (strCount=="0")//数据库中没有,插入
				{
					//取is_make
					strSQL.Format(_T("SELECT is_make FROM order_head WHERE order_head_id=\'%d\';"),pApp->m_nOrderHeadid);
					rs.Open(-1,strSQL);
					CString strCount;
					rs.GetFieldValue((short)0,strCount);
					rs.Close();
					strSQL.Format(_T("INSERT INTO order_head(order_head_id,check_number,table_id,table_name,check_id,customer_num,order_start_time")
						_T(",eat_type,open_employee_id,open_employee_name,pos_device_id,pos_name,rvc_center_id,rvc_center_name,is_make)")
						_T(" VALUES(\'%d\',\'%d\',\'%d\',\'%s\',\'%d\',\'%d\',NOW(),\'%d\',\'%s\',\'%s\',\'%d\',\'%s\',\'%d\',\'%s\',\'%s\');"),
						pApp->m_nOrderHeadid,pApp->m_nCheckNum,pApp->m_nTableId,pApp->m_strTblName,i,pApp->m_nGuests
						,pApp->m_nEatType,pApp->m_strUserID,pApp->m_strUserName,pApp->m_nDeviceId,pApp->m_strDeviceName,pApp->m_nRVC,pApp->m_strRVC,strCount);
					theDB.ExecuteSQL(strSQL);
				}
			}
			//更新需要付款的金额
			strSQL.Format(_T("UPDATE order_head SET should_amount=\'%0.2f\',tax_amount=\'%0.2f\' WHERE order_head_id=%d AND check_id=%d;"),
				(double)(*m_checkDlg)[i-1].m_fDebt,(*m_checkDlg)[i-1].m_fTax,pApp->m_nOrderHeadid,i);
			theDB.ExecuteSQL(strSQL);
		}
		if (COrderPage::bNewTainfo)
		{
			CString strSQL,strTrim;
			strTrim=m_strTAInfo;
			strTrim.Replace(_T("\""),_T("\"\""));
			strTrim.Replace(_T("\'"),_T("\'\'"));
			strSQL.Format(_T("UPDATE order_head SET delivery_info=\'%s\' WHERE order_head_id=%d AND check_id=1"),
				strTrim,theApp.m_nOrderHeadid);
			theDB.ExecuteSQL(strSQL);
			COrderPage::bNewTainfo=FALSE;
		}
		LOG4CPLUS_INFO(log_pos,"OrderDlg::SaveOrderToDB head="<<pApp->m_nOrderHeadid);
		return bSaved;
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		AfxMessageBox(e->m_strError,MB_ICONEXCLAMATION);
		e->Delete();
		return FALSE;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
		return FALSE;
	}
}
void OrderDlg::OnBnClickedButtonSave()
{
	SaveOrderToDB(m_pOrderList,&m_checkDlg);
}
void OrderDlg::SendOrder(CCheckDlg (&m_checkDlg)[MAX_CHECKS],int count)
{
	CString strSQL;
	vector<PrintItem> menu_list;
	for(int k=0;k<count;k++)
	{
		CListBoxEx* ptrDetailList=&(m_checkDlg[k].m_ctrlDetailList);
		for(int i=0;i<ptrDetailList->GetCount();i++)
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if (item->n_saved!=2&&(item->item_id>0))//未送厨
			{
				BOOL bFind=FALSE;
				if(macrosInt[_T("COMBINE_SENDITEM")]==1)
				{
					if (item->b_hascondiment==FALSE&&item->n_belong_item<=0
						&&item->description[0]==0)
					{//配料，有配料的产品不合并
						for (int j=0;j<menu_list.size();j++)
						{
							if (menu_list[j].item_id==item->item_id
								&&menu_list[j].item_price==item->item_price
								&&menu_list[j].n_eattype==item->n_eattype
								&&menu_list[j].description[0]==0
								&&menu_list[j].item_name.Compare(item->item_name)==0
								&&menu_list[j].unit.Compare(item->unit)==0)//相同规格
							{//找到一样的
								menu_list[j].quantity+=item->quantity;
								menu_list[j].total_price+=item->total_price;
								bFind=TRUE;
								break;
							}
						}
					}
				}
				if(!bFind)
				{
					//调味品合并打印
					int i=menu_list.size()-1;
					if (item->n_belong_item>0&&macrosInt[_T("COMBINE_CONDIMENT")]==1
						&&i>=0)
					{
						menu_list[i].description.AppendFormat(_T(" %s"),item->item_name);
						menu_list[i].total_price+=item->total_price;
					}
					else
					{
//LOG4CPLUS_WARN(log_pos,"1. before id="<<item->item_id<<" print_class="<<item->print_class);
						PrintItem pItem;
						CopyPrintItem(pItem,item);
						menu_list.push_back(pItem);
					}
				}
				//更新数据库为已送厨
				item->n_saved=2;
				strSQL.Format(_T("UPDATE order_detail SET is_send=1 WHERE order_detail_id=\'%d\';"),item->order_id);
				theDB.ExecuteSQL(strSQL);
			}
		}
	}
	//调用打印
	JSONVALUE root;
	root["template"]=TEMPLATE_SEND;
	FormatTableInfo(root);
	CRecordset rs(&theDB);
	if(!m_strTblRemark.IsEmpty())
		root[_T("remark")]=m_strTblRemark;
	root[_T("item_total")]=menu_list.size();
#ifdef EN_VERSION
	root[_T("subtotal")]=m_checkDlg[0].m_fSubtotal;//菜品总价
	if (!COrderPage::m_strTAInfo.IsEmpty())
		root[_T("tainfo")]=COrderPage::m_strTAInfo;
#endif
	PrintMenu(menu_list,root,macrosInt[_T("SENDITEM_ORDER")]);
	if(menu_list.size()>0&&macrosInt[_T("AUTO_PRINT_ORDER")])
	{
		PrintTableOrder(menu_list,m_checkDlg[0]);
		//PayDlg::PrintRound(m_checkDlg[0],0);
	}
	//通知厨显
	strSQL.Format(_T("UPDATE order_head SET is_make=0 WHERE order_head_id=%d"),theApp.m_nOrderHeadid);
	theDB.ExecuteSQL(strSQL);
	//查找关联桌，是否一起送厨？
	if(theApp.m_nPartyId>0)
	{
		strSQL.Format(_T("SELECT * FROM order_head WHERE party_id=%d AND order_head_id<>%d"),theApp.m_nPartyId,theApp.m_nOrderHeadid);
		CRecordset rs1(&theDB);
		if(!rs.Open(-1,strSQL))
			return ;
		if (rs.GetRecordCount()==0)
			return ;
		if(POSMessageBox(IDS_COMBINESEND,MB_YESNO)!=IDOK)
			return ;
		while(!rs.IsEOF())
		{//查询菜品
			menu_list.clear();
			CDBVariant variant;
			rs.GetFieldValue(_T("order_head_id"),variant);
			long head_id=variant.m_lVal;
			strSQL.Format(_T("SELECT * FROM order_detail WHERE order_head_id=%d AND is_return_item=0 AND is_send IS NULL"),head_id);
			rs1.Open(-1,strSQL);
			while(!rs1.IsEOF())
			{
				PrintItem pItem;
				CString strValue;
				rs1.GetFieldValue(_T("menu_item_id"),variant);
				pItem.item_id=variant.m_lVal;
				rs1.GetFieldValue(_T("condiment_belong_item"),variant);
				pItem.n_belong_item=variant.m_lVal;
				rs1.GetFieldValue(_T("menu_item_name"),pItem.item_name);
				rs1.GetFieldValue(_T("is_discount"),variant);
				pItem.b_isfree=variant.m_boolVal;
				rs1.GetFieldValue(_T("quantity"),variant);
				pItem.quantity=variant.m_fltVal;
				if (pItem.quantity<0)
					pItem.b_isvoid=TRUE;
				else
					pItem.b_isvoid=FALSE;
				rs1.GetFieldValue(_T("description"),pItem.description);
				rs1.GetFieldValue(_T("product_price"),variant);
				pItem.item_price=variant.m_fltVal;
				rs1.GetFieldValue(_T("eat_type"),variant);
				pItem.n_eattype=variant.m_iVal;
				rs1.GetFieldValue(_T("seat_num"),variant);
				pItem.n_seat=variant.m_iVal;
				rs1.GetFieldValue(_T("unit"),pItem.unit);
				rs1.GetFieldValue(_T("actual_price"),variant);
				pItem.total_price=variant.m_fltVal;
				rs1.GetFieldValue(_T("print_class"),variant);
				pItem.n_class=variant.m_iVal;
				menu_list.push_back(pItem);
				//更新数据库为已送厨
				rs1.GetFieldValue(_T("order_detail_id"),variant);
				strSQL.Format(_T("UPDATE order_detail SET is_send=1 WHERE order_detail_id=\'%d\';"),variant.m_lVal);
				theDB.ExecuteSQL(strSQL);
				rs1.MoveNext();
			}
			rs1.Close();
			//插入送厨标志?
			//更新桌态为已送厨
			rs.GetFieldValue(_T("table_id"),variant);
			int table_id=variant.m_lVal;
			strSQL.Format(_T("UPDATE tables SET table_status=3 WHERE table_id=\'%d\';"),table_id);
			theDB.ExecuteSQL(strSQL);
			//调用打印
			JSONVALUE root;
			root[_T("template")]=TEMPLATE_SEND;
			root[_T("serial")]=head_id;
			CString strValue;
			rs.GetFieldValue(_T("table_name"),strValue);
			root[_T("tbl_name")]=strValue;
			rs.GetFieldValue(_T("open_employee_name"),strValue);
			root[_T("emp_name")]=strValue;
			rs.GetFieldValue(_T("check_name"),strValue);
			if(!strValue.IsEmpty())
				root[_T("chk_name")]=strValue;
			root[_T("tbl_id")]=table_id;
			rs.GetFieldValue(_T("check_number"),variant);
			root[_T("chk_num")]=variant.m_lVal;
			rs.GetFieldValue(_T("customer_num"),variant);
			root[_T("gst_num")]=variant.m_lVal;
			rs.GetFieldValue(_T("remark"),strValue);
			root[_T("remark")]=strValue;
			CTime time=CTime::GetCurrentTime();
			root[_T("time")]=time.Format("%Y-%m-%d %H:%M");
			PrintMenu(menu_list,root,macrosInt[_T("SENDITEM_ORDER")]);
			rs.MoveNext();
		}
		rs.Close();
	}
}
/************************************************************************
* 函数介绍：打印桌台划菜单
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::PrintTableOrder(vector<PrintItem>& menu_list,CCheckDlg& currentChkDlg)
{
	PrintDeviceInfo printer;
	if(theApp.m_prePrinter.nDeviceType==-1||theApp.m_prePrinter.nPaperWidth==0)
	{
		if (theApp.m_payPrinter.nDeviceType==-1)
		{//没有配置打印机，无法打印
			return;
		}
		printer=theApp.m_payPrinter;
	}
	else
		printer=theApp.m_prePrinter;
	
		JSONVALUE printTask;
		CPOSClientApp::FormatPrintDevice(printTask,printer);
		PayDlg::FormatTableInfo(printTask);
		double round_total=0;
		JSONVALUE arrayObj;
		for(int i=0;i<menu_list.size();i++)
		{
			JSONVALUE item;
			PrintItem2Json(&menu_list[i],item);
			arrayObj.Push(item);
			round_total+=menu_list[i].total_price;
		}
		printTask[_T("items")]=arrayObj;
		printTask[_T("template")]=TEMPLATE_MAKE;
		printTask[_T("chk_id")]=1;
		if(currentChkDlg.m_fDebt<=0.01)
			printTask[_T("chk_total")]=currentChkDlg.m_fPayed;
		else
			printTask[_T("chk_total")]=currentChkDlg.m_fDebt;
		printTask[_T("round_total")]=round_total;//这一轮划菜单菜品总价
		printTask[_T("subtotal")]=currentChkDlg.m_fSubtotal;//菜品总价
		printTask[_T("item_total")]=currentChkDlg.m_nTotal;
		if (!COrderPage::m_strTAInfo.IsEmpty())
			printTask[_T("tainfo")]=COrderPage::m_strTAInfo;
		SYSTEMTIME st;
		GetLocalTime(&st);
		CString strTmp;
		strTmp.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),st.wYear,st.wMonth,st.wDay,
			st.wHour,st.wMinute,st.wSecond);
		printTask[_T("end_time")]=(LPCTSTR)strTmp;
		//划菜单使用本地打印
		theApp.PrintJson(printTask);
}
/************************************************************************
* 函数介绍：下单，发送到厨房打印机
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedSendOrder()
{
	//快餐模式，或退款
	if (theApp.m_bQuickService||theApp.m_bRefund)
		return;
	if (m_bNotReg)
	{
		POSMessageBox(IDS_NOTREG);
		PayDlg::CHECK_COUNT++;
		if (PayDlg::CHECK_COUNT>10)
			return;
		CString strSQL;
		CRecordset rs(&theDB);
		strSQL.Format(_T("SELECT (SELECT COUNT(order_head_id) FROM history_order_head WHERE date(order_start_time)=date(now()))+") 
			_T(" (SELECT COUNT(order_head_id) FROM order_head WHERE date(order_start_time)=date(now())) as hcount"));
		rs.Open( CRecordset::forwardOnly,strSQL);
		if (!rs.IsEOF()) 
		{
			rs.GetFieldValue((short)0,strSQL);
			int count=_wtoi(strSQL);
			if (count>=10)
				return;
		}
		rs.Close();
	}
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedSendOrder Begin");
		int active=m_TabCtrl.GetActivePageIndex();
		if (m_checkDlg[active].m_nStatus==1)
		{
			return;
		}
		CString strSQL;
		CString userid;
		int auth=OrderDlg::RequeastAuth(userid,_T("send_order"));
		if(auth!=0)
			return;
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		if (m_pOrderList->GetCount()==0)
		{
			strSQL.Format(_T("CALL flush_order(%d,%d);"),pApp->m_nOrderHeadid,pApp->m_nTableId);
			theDB.ExecuteSQL(strSQL);
		}
		else
		{
			//添加点单时间
			int count=m_TabCtrl.GetPageCount();
			CTime time=CTime::GetCurrentTime();
			for(int i=0;i<count;i++)
			{
				CListBoxEx* ptrDetailList=&m_checkDlg[i].m_ctrlDetailList;
				//如果已送厨不添加时间
				int tail=ptrDetailList->GetCount();
				if (tail==0)
					continue;
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(tail-1);

				if(item->item_id==ITEM_ID_SENDINFO)
					continue;//末尾是下单信息
				item=new OrderDetail;
				memset(item,0,sizeof(OrderDetail));
				item->item_id=ITEM_ID_SENDINFO;
				item->b_notprint=TRUE;
				item->n_checkID=i+1;
				CString name,str2;
				theLang.LoadString(str2,IDS_SEND);
				name.Format(_T("**%s %s %s**"),theApp.m_strUserID,str2,time.Format("%H:%M"));
				wcsncpy_s(item->item_name,name,63);
				//不会合并
				m_pOrderList->AddTail(item);
				m_checkDlg[i].AddOrderItem(item);
			}
			if(SaveOrderToDB(m_pOrderList,&m_checkDlg)==1)
			{
				strSQL.Format(_T("UPDATE order_head SET is_make=NULL WHERE order_head_id=%d;"),pApp->m_nOrderHeadid);
				theDB.ExecuteSQL(strSQL);
			}
			SendOrder(m_checkDlg,count);
			//更新桌态为已送厨
			strSQL.Format(_T("UPDATE tables SET table_status=3 WHERE table_id=\'%d\';"),pApp->m_nTableId);
			theDB.ExecuteSQL(strSQL);
		}
		COrderPage::m_strTAInfo.Empty();
		//查找并启动打印进程
		if(::FindWindow(_T("CPrintServerDlg"),_T("AgilePrintServer"))==NULL)
		{
			STARTUPINFO si = { sizeof(STARTUPINFO) };
			si.cb = sizeof(si);
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
			PROCESS_INFORMATION pi;
			CreateProcess(_T("PrintServer.exe"), NULL , NULL, NULL, FALSE, CREATE_NO_WINDOW , NULL, NULL, &si, &pi);
		}
		if(macrosInt[_T("EXIT_AFTER_SEND")]==0)
		{//送厨后不跳转
			return;
		}
		else if(macrosInt[_T("EXIT_AFTER_SEND")]==1)
		{//跳转到桌面图
			//释放锁
			strSQL.Format(_T("SELECT RELEASE_LOCK('table%d');"),pApp->m_nTableId);
			theDB.ExecuteSQL(strSQL);
			pApp->m_pMain->SetActivePage(DLG_FLOOR);
		}
		else
		{//跳转到登陆界面
			//释放锁
			strSQL.Format(_T("SELECT RELEASE_LOCK('table%d');"),pApp->m_nTableId);
			theDB.ExecuteSQL(strSQL);
			pApp->m_pMain->SetActivePage(DLG_LOGIN);
		}

	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
		return;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：菜品数量
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedQuantity()
{
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedQuantity Begin");
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{//未选中，数量下次生效
			NumberInputDlg dlg;
			theLang.LoadString(dlg.m_strHint,IDS_INPUTQUANTITY);
			if(dlg.DoModal()==IDOK)
			{
				int num=_wtoi(dlg.m_strNum);
				if (num<=0||num>MAX_ITEM_QUANTITY)
				{
					POSMessageBox(IDS_NUMERROR);
					return;
				}
				m_nQuantity=num;
				CString strTmp;
				strTmp.Format(_T("%d X"),m_nQuantity);
				UpdateHint(strTmp);
			}
			return;
		}
		BOOL bFirst=TRUE;
		double num=0,usednum;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->item_id<=0)
					continue;
				if(item->n_belong_item>0&&item->item_price==0)
				{//没有价格的配料不用修改
					ptrDetailList->SetSel(i,FALSE);
					continue;
				}
				if (item->n_belong_item>0&&item->item_price>0)
				{//非称重配料不修改
					if(item->weight_required==FALSE&&bFirst==FALSE)
					{
						ptrDetailList->SetSel(i,FALSE);
						continue;
					}
				}
				if (item->n_belong_item<0||item->b_hascondiment==ITEM_SET)
				{//套餐和套餐内部菜不能修改
					//ptrDetailList->SetSel(i,FALSE);
					continue;
				}
				if (item->n_saved!=2||item->weight_required==TRUE)
				{//未送厨,或者是称重菜品
					if(bFirst)
					{//第一次，弹出数量对话框
						NumberInputDlg dlg;
						theLang.LoadString(dlg.m_strHint,IDS_INPUTQUANTITY);
						if(dlg.DoModal()==IDCANCEL)
							return;
						num=_wtof(dlg.m_strNum);
						if (num<=0||num>MAX_ITEM_QUANTITY)
						{
							POSMessageBox(IDS_NUMERROR);
							return;
						}
						bFirst=FALSE;
					}
					CString strSQL;
					usednum=num;
					if (item->quantity<0)
					{//退菜不修改沽清值
						usednum*=-1;
					}
					UpdateSoldoutInfo(item,item->quantity-usednum);
					item->quantity=usednum;
					item->b_isfree=FALSE;
					//刷新显示
					ComputeTotalPrice(item);
					m_checkDlg[active].UpdateItemString(i);
					if(theApp.m_VCR.IsOpen())
					{
						CString line;
						line.Format(_T("QUANTITY %s %0.2f\n"),item->item_name,item->quantity);
						theApp.m_VCR.Print(line);
					}
					//存储的更新数据库
					if(item->order_id>0)
					{
						strSQL.Format(_T("UPDATE order_detail SET quantity=%f,actual_price=%f WHERE order_detail_id=%d")
							,item->quantity,item->total_price,item->order_id);
						theDB.ExecuteSQL(strSQL);
					}
				}
				else
				{
					POSMessageBox(IDS_ALREADYSEND);
					ptrDetailList->SetSel(i,FALSE);
					return;
				}
			}
		}
		UpdateSencondScreen(active,NULL);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}

	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedQuantity m_nQuantity="<<m_nQuantity);
}
void OrderDlg::OnBnClickedNextSeat()
{
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedNextSeat Begin");
	int active=m_TabCtrl.GetActivePageIndex();
	CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
	if (ptrDetailList==NULL)
		return;
	m_nSeat++;
	CString strTmp;
	if(m_nSeat>0)
	{
		strTmp.Format(_T("Seat %d"),m_nSeat);
	}
	UpdateHint(strTmp);
}
void OrderDlg::OnBnClickedSeatNum()
{
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedSeatNum Begin");
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{//未选中，数量下次生效
			NumberInputDlg dlg;
			theLang.LoadString(dlg.m_strHint,IDS_INPUTSEATNUM);
			if(dlg.DoModal()==IDOK)
			{
				int num=_wtoi(dlg.m_strNum);
				if (num>20)
				{
					POSMessageBox(IDS_NUMERROR);
					return;
				}
				m_nSeat=num;
				CString strTmp,str2;
				if(m_nSeat>0)
				{
					strTmp.Format(_T("Seat %d"),m_nSeat);
				}
				UpdateHint(strTmp);
			}
			return;
		}
		BOOL bFirst=TRUE;
		int num=0;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->n_belong_item!=0||item->item_id<=0)
				{
					ptrDetailList->SetSel(i,FALSE);
					continue;
				}
				if (item->n_saved!=2)
				{//未送厨
					if(bFirst)
					{//第一次，弹出数量对话框
						NumberInputDlg dlg;
						theLang.LoadString(dlg.m_strHint,IDS_INPUTSEATNUM);
						if(dlg.DoModal()==IDCANCEL)
							return;
						num=_wtoi(dlg.m_strNum);
						if (num>20)
						{
							POSMessageBox(IDS_NUMERROR);
							return;
						}
						bFirst=FALSE;
					}
					item->n_seat_num=num;
					//刷新显示
					m_checkDlg[active].UpdateItemString(i);
					//存储的更新数据库
					CString strSQL;
					if(item->order_id>0)
					{
						strSQL.Format(_T("UPDATE order_detail SET seat_num=%d WHERE order_detail_id=%d")
							,item->n_seat_num,item->order_id);
						theDB.ExecuteSQL(strSQL);
					}
				}
				else
				{
					POSMessageBox(IDS_ALREADYSEND);
					ptrDetailList->SetSel(i,FALSE);
					return;
				}
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：数字键
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnNumBnClicked(UINT uID)
{
	m_nQuantity=uID-IDC_BUTTON0;
	CString strTmp;
	strTmp.Format(_T("%d X"),m_nQuantity);
	UpdateHint(strTmp);
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnNumBnClicked m_nQuantity="<<m_nQuantity);
}
/************************************************************************
* 函数介绍：自动弹出必选配料
* 输入参数：pOrder -配料所属的菜
*			nGroup -默认显示的配料组
* 输出参数：
* 返回值  ：0,没有找到配料组，不显示； 1，正常显示了配料
************************************************************************/
int OrderDlg::ShowCondiment(OrderDetail* pOrder,int nGroup)
{
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::ShowCondiment nGroup="<<nGroup);
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return FALSE;
		int selIndex=ptrDetailList->GetCount()-1;
		for(int i=ptrDetailList->GetCount()-1;i>=0;i--)
		{
			OrderDetail* pTmp=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if (pTmp==pOrder)
			{
				selIndex=i;
				break;
			}
		}
		NotifyKitchenDlg dlg;
		dlg.m_nDefaultGroup=nGroup;
		theLang.LoadString(dlg.m_strTtitle,IDS_ADDREQUEST);
		dlg.type=1;
		dlg.pBelongItem=pOrder;
		dlg.m_insertIdx=selIndex+1;
		dlg.pOrderDlg=this;
		if(dlg.DoModal()==IDCANCEL)
		{//取消选中
			//ptrDetailList->SetSel(selIndex,FALSE);
			return FALSE;
		}
		wcsncpy_s(pOrder->description,dlg.m_strText,99);
		m_checkDlg[active].UpdateItemString(selIndex);
		UpdateSencondScreen(active,NULL);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
	return 1;
}

/************************************************************************
* 函数介绍：换台
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedTransfer()
{
	if (theApp.m_bRefund)
		return;
	FunctionDlg::TransTable();
	UpdateInfo(this);
}

void OrderDlg::OnBnClickedCombine()
{
	if (theApp.m_bRefund)
		return;
	FunctionDlg::CombineTable();
	//刷新显示
	OnSetActive();
}
/************************************************************************
* 函数介绍：重复选中的项目
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedRepeat()
{
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedRepeat Begin");
		int active=m_TabCtrl.GetActivePageIndex();
		if (m_checkDlg[active].m_nStatus==1)
		{
			return;
		}
		//复制选中的到数据库
		//如果没有选中的，默认复制最后一行
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList->GetSelCount()==0)
		{
			ptrDetailList->SetSel(ptrDetailList->GetCount()-1);
		}
		int size=ptrDetailList->GetCount();
		for(int i=0;i<size;i++) 
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* itemSource=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				//不复制服务费和折扣
				if(itemSource->item_id<0)
					continue;
				//添加到新点菜品列表
				OrderDetail* item=new OrderDetail;
				memcpy(item,itemSource,sizeof(OrderDetail));
				if(itemSource->return_reason!=NULL)
				{
					int len=wcslen(itemSource->return_reason);
					item->return_reason=new wchar_t[len+1];
					item->return_reason[len]=0;
					wcsncpy_s(item->return_reason,len+1,itemSource->return_reason,len);
				}
				item->n_saved=0;
				m_pOrderList->AddTail(item);
				CString strTmp;
				ptrDetailList->GetText(i,strTmp);
				int pos=ptrDetailList->AddString(strTmp);
				ptrDetailList->SetItemDataPtr(pos,item);
				m_checkDlg[active].m_fSubtotal+=item->total_price;
				ptrDetailList->SetSel(i,FALSE);
			}
		}
		m_checkDlg[active].ComputeSubtotal();
		UpdateSencondScreen(active,NULL);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：增加数量
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedIncrease()
{
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedIncrease Begin");
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{
			return;
		}
		int size=ptrDetailList->GetCount();
		for(int  i=0;i<size;i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->n_belong_item!=0||item->item_id<0)
					continue;
				if (item->b_hascondiment&&item->weight_required==FALSE)
				{//有配料或套餐不能修改
					continue;
				}
				if(macrosInt[_T("NO_AUTO_COMBINE")]==1)
				{//不合并菜品,新增
					OrderDetail* newItem=new OrderDetail;
					memcpy(newItem,item,sizeof(OrderDetail));
					newItem->return_reason=NULL;
					newItem->n_saved=0;
					m_pOrderList->AddTail(newItem);
					m_checkDlg[active].AddOrderItem(newItem);
					continue;
				}
				if (item->n_saved==0)
				{
					item->quantity+=1;
					//刷新显示
					if(!item->b_isfree)
						ComputeTotalPrice(item);
					m_checkDlg[active].UpdateItemString(i);
					ptrDetailList->SetSel(i);
					UpdateSoldoutInfo(item,-1);
				}
				else
				{
					POSMessageBox(IDS_CANNOTMODIFY);
					return;
				}
			}
		}
		UpdateSencondScreen(active,NULL);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：减少数量
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedDecrease()
{
	try{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedDecrease Begin");
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{
			return;
		}
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->n_belong_item!=0)
					continue;
				if (item->b_hascondiment&&item->weight_required==FALSE)
				{//有配料或套餐不能修改
					continue;
				}
				if (item->n_saved==0)
				{

					if (item->quantity<=1)
						continue;
					item->quantity-=1;
					//刷新显示
					if(!item->b_isfree)
						ComputeTotalPrice(item);
					m_checkDlg[active].UpdateItemString(i);
					ptrDetailList->SetSel(i);
					UpdateSoldoutInfo(item,1);
				}
				else
				{
					POSMessageBox(IDS_CANNOTMODIFY);
					return;
				}
			}
		}
		UpdateSencondScreen(active,NULL);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：加单
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedAddCheck()
{
	try{
		if (theApp.m_bRefund||theApp.m_bQuickService)
			return;
		if (m_TabCtrl.GetPageCount()>=MAX_CHECKS)
		{
			POSMessageBox(IDS_TOOMANYCHK);
			return;
		}
		CString strTmp;
		int count=m_TabCtrl.GetPageCount();
		strTmp.Format(_T("%d"),count+1);
		m_TabCtrl.AddPage(&m_checkDlg[count],strTmp,NULL);
		m_TabCtrl.SetActivePage(count);
		m_TabCtrl.UpdateWindow();
		if(count==m_TabCtrl.GetActivePageIndex())
		{
			//添加开台消费折扣
			CString strSQL;
			CRecordset rs(&theDB);
			CRecordset rs1(&theDB);
			strSQL.Format(_T("select * from order_detail_default where menu_item_id<0 and extend_1 in (select order_default_groupid from ")
				_T("order_default_group where  (serving_place_class=-1 or serving_place_class in (select ")
				_T(" rvc_class_id from serving_rvc_class where rvc_center_id=%d)) and (serving_period_class=-1")
				_T(" or serving_period_class in (select period_class_id from serving_period_class where ")
				_T(" period in(select period_id from periods where (start_time<end_time and CURTIME() BETWEEN start_time and end_time) or (start_time>=end_time and (start_time<CURTIME() or end_time>CURTIME()))))));")
				,theApp.m_nRVC);
			if(!rs1.Open( CRecordset::forwardOnly,strSQL))
				return ;
			while(!rs1.IsEOF())
			{
				CDBVariant variant;
				rs1.GetFieldValue(_T("menu_item_id"),variant);
				OrderDetail* order=new OrderDetail;
				memset(order,0,sizeof(OrderDetail));
				order->item_id=variant.m_lVal;
				order->n_checkID=count+1;
				rs1.GetFieldValue(_T("discount_service_id"),variant);
				if(FloorDlg::ReadDiscountService(order,variant.m_lVal)==FALSE)
				{
					rs1.MoveNext();
					continue;
				}
				m_pOrderList->AddTail(order);
				m_checkDlg[count].AddOrderItem(order);
				rs1.MoveNext();
			}
			rs1.Close();
		}
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		AfxMessageBox(e->m_strError,MB_ICONEXCLAMATION);
		e->Delete();
	}
}
/************************************************************************
* 函数介绍：锁屏自动注销
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
BOOL OrderDlg::LockScreen()
{
	if(theApp.m_bQuickService)
	{//快餐模式不锁屏
		return FALSE;
	}
	else
	{
		SaveOrderToDB(m_pOrderList,&m_checkDlg);
		//释放锁
		CString strSQL;
		COrderPage::m_strTAInfo.Empty();
		strSQL.Format(_T("SELECT RELEASE_LOCK('table%d');"),theApp.m_nTableId);
		theDB.ExecuteSQL(strSQL);
	}
	return TRUE;
}
/************************************************************************
* 函数介绍：返回前一页
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedPrepage()
{
	CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
	if(pApp->m_bQuickService)
	{
		QuickOrderLogout();
		return;
	}
	try
	{
		CString strSQL;
		if(SaveOrderToDB(m_pOrderList,&m_checkDlg)==1)
		{
			//提示是否送厨
			if(!theApp.m_bQuickService&&!theApp.m_bRefund
				&&macrosInt[_T("NO_SEND_HINT")]==0)
			{
				CString str2;
				theLang.LoadString(str2,IDS_SENDORDERHINT);
				if(POSMessageBox(str2,MB_YESNO)==IDOK)
				{
					OnBnClickedSendOrder();
					return;
				}
			}
			//新点了菜，修改桌态
			strSQL.Format(_T("UPDATE tables SET table_status=2 WHERE table_id=%d;"),pApp->m_nTableId);
			theDB.ExecuteSQL(strSQL);
			strSQL.Format(_T("UPDATE order_head SET is_make=NULL WHERE order_head_id=%d;"),pApp->m_nOrderHeadid);
			theDB.ExecuteSQL(strSQL);
		}
		if(macrosInt[_T("NO_AUTO_CLOSECHK")]==0)
		{
			strSQL.Format(_T("CALL flush_order(%d,%d);"),pApp->m_nOrderHeadid,pApp->m_nTableId);
			theDB.ExecuteSQL(strSQL);
		}
		//释放锁
		COrderPage::m_strTAInfo.Empty();
		strSQL.Format(_T("SELECT RELEASE_LOCK('table%d');"),pApp->m_nTableId);
		theDB.ExecuteSQL(strSQL);
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		AfxMessageBox(e->m_strError,MB_ICONEXCLAMATION);
		e->Delete();
		pApp->m_pMain->SetActivePage(DLG_LOGIN);
		return;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
	pApp->m_pMain->SetActivePage(DLG_FLOOR);
}
/************************************************************************
* 函数介绍：修改规格
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedModify()
{
	try{
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{
			return;
		}
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedModify Begin");
		BOOL bComplete=FALSE;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if (bComplete)
			{
				ptrDetailList->SetSel(i,FALSE);
			}
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->n_belong_item>0||item->item_id<0)
					continue;
				if (item->n_saved!=2)
				{
					ModifyDlg dlg(this);
					dlg.m_item=item;
					if(dlg.DoModal()==IDOK)
					{//刷新显示
						ComputeTotalPrice(item);
						item->b_isfree=FALSE;
						m_checkDlg[active].UpdateItemString(i);
						OpenDatabase();
						CString strSQL;
						strSQL.Format(_T("UPDATE order_detail SET product_price=\'%f\',actual_price=\'%f\',unit=\'%s\' WHERE order_detail_id=\'%d\';"),
							item->item_price,item->total_price,item->unit,item->order_id);
						theDB.ExecuteSQL(strSQL);
					}
					else
						return;
				}
				else
					POSMessageBox(IDS_ALREADYSEND);
				ptrDetailList->SetSel(i,FALSE);
				bComplete=TRUE;
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：修改单个菜的堂食或者外带
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedTakeOut()
{
	try{
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{
			return;
		}
		OpenDatabase();
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedTakeOut Begin");
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->n_belong_item>0||item->item_id<0)
					continue;
				if (item->n_saved!=2)
				{
					//刷新显示
					if(item->n_eattype==TYPE_TAKEOUT)
						item->n_eattype=TYPE_DINE;
					else
						item->n_eattype=TYPE_TAKEOUT;
					m_checkDlg[active].UpdateItemString(i,FALSE);
					CString strSQL;
					strSQL.Format(_T("UPDATE order_detail SET eat_type=\'%d\' WHERE order_detail_id=\'%d\';"),
						item->n_eattype,item->order_id);
					theDB.ExecuteSQL(strSQL);
				}
				else
				{
					POSMessageBox(IDS_ALREADYSEND);
					return;
				}
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：修改整单为堂食或者外带
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedTakeOutChk()
{
	PayDlg::OnBnClickedTakeOutChk(m_pOrderList);
	UpdateCheckData();
}
/************************************************************************
* 函数介绍：改价
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedRePrice()
{
	try{
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
			return;
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedRePrice Begin");
		CString userid;
		WCHAR username[10];
		int auth=OrderDlg::RequeastAuth(userid,_T("reprice"),1,username);
		if(auth!=0)
			return;
		double price=0;
		BOOL bRepriceTotal=FALSE;
		if(macrosInt[_T("REPRICE_TOTAL")]==1)
			bRepriceTotal=TRUE;
		BOOL bSet=FALSE;
		CString strDetail;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->n_belong_item!=0||item->item_id<0)
					continue;
				if(!bSet)
				{//输入菜品价格
					NumberInputDlg dlgn;
					if(bRepriceTotal)
						theLang.LoadString(dlgn.m_strHint,IDS_INPUTPRICETOTAL);
					else
						theLang.LoadString(dlgn.m_strHint,IDS_INPUTPRICE);
					if(dlgn.DoModal()!=IDOK)
						return;
					price=_wtof(dlgn.m_strNum);
					if(price>999999)
					{
						POSMessageBox(IDS_TOOMUCH);
						return;
					}
					bSet=TRUE;
				}
				//刷新显示
				if (item->b_reprice==FALSE)
				{//第一次改价
					item->orignal_price=item->item_price;
					item->b_reprice=TRUE;
				}
				if(bRepriceTotal)
				{
					if (item->quantity<0)
						item->total_price=-price;
					else
						item->total_price=price;
					item->item_price=item->total_price/item->quantity;
				}
				else
				{
					item->item_price=price;
					ComputeTotalPrice(item);
				}
				item->b_isfree=FALSE;
				m_checkDlg[active].UpdateItemString(i,FALSE);
				strDetail.AppendFormat(_T("%s(%0.2f) "),item->item_name,item->total_price);
				if (item->n_saved!=0)
				{
					CString strSQL;
					strSQL.Format(_T("UPDATE order_detail SET actual_price=\'%f\',product_price=\'%f\',original_price=IF(original_price IS NULL,\'%f\',original_price),")
						_T("is_discount=0,auth_id=\'%s\',auth_name=\'%s\' WHERE order_detail_id=\'%d\';"),
						item->total_price,item->item_price,item->orignal_price,userid,username,item->order_id);
					theDB.ExecuteSQL(strSQL);
				}
				else
				{
					wcsncpy_s(item->authID,userid,9);
					wcsncpy_s(item->authUser,username,9);
				}
			}
		}
		m_checkDlg[active].ComputeSubtotal();
		UpdateSencondScreen(active,NULL);
		if(theApp.m_VCR.IsOpen())
		{
			CString line;
			line.Format(_T("REPRICE %s\n"),strDetail);
			theApp.m_VCR.Print(line);
		}
		CPOSClientApp::CriticalLogs(OPR_REPRICE,strDetail);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：自定义菜功能按钮  (作废)
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedOpenItem()
{
	// 	try{
	// 		int active=m_TabCtrl.GetActivePageIndex();
	// 		if (m_checkDlg[active].m_nStatus==1)
	// 		{
	// 			POSMessageBox(IDS_NOACTION);
	// 			return;
	// 		}
	// 		CString userid;
	// 		int auth=OrderDlg::RequeastAuth(userid,_T("open_item"));
	// 		if(auth!=0)
	// 			return;
	// 		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedOpenItem");
	// 		m_Items.clear();
	// 		OpenDatabase();
	// 		CRecordset rs( &theDB);
	// 		CString strSQL;
	// 		strSQL.Format(_T("SELECT * FROM menu_item  LEFT JOIN menu_item_class ON menu_item.class_id")
	// 			_T("=menu_item_class.item_class_id WHERE item_id>0 AND item_id<%d;"),10);
	// 		rs.Open( CRecordset::forwardOnly,strSQL);
	// 		while(!rs.IsEOF())
	// 		{
	// 			MenuItem item={0};
	// 			item.type=ITEM_OPEN;//自定义
	// 			CDBVariant variant;
	// 			rs.GetFieldValue(_T("item_name1"), item.item_name1);
	// 			rs.GetFieldValue(_T("item_id"), variant);
	// 			item.item_number=variant.m_lVal;
	// 			variant.m_iVal=9;//不配置折扣级别的菜，默认最高级别，所有折扣都可使用
	// 			rs.GetFieldValue(_T("discount_itemizer"), variant);
	// 			item.n_discount_level=variant.m_iVal;
	// 			variant.m_iVal=9;
	// 			rs.GetFieldValue(_T("service_itemizer"), variant);
	// 			item.n_service_level=variant.m_iVal;
	// 			rs.GetFieldValue(_T("tax_group"), variant);
	// 			item.tax_group=variant.m_iVal;
	// 			m_Items.push_back(item);
	// 			rs.MoveNext();
	// 		}
	// 		rs.Close();
	// 		m_nCurrentPage=0;
	// 		m_nPageCount=(int)ceil((float)m_Items.size()/m_nPageSize);
	// 		ShowCurrentPage();
	// 	}
	// 	catch(CDBException* e)
	// 	{
	// 		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
	// 		e->Delete();
	// 		return;
	// 	}
	// 	catch(...)
	// 	{
	// 		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	// 	}
}
/************************************************************************
* 函数介绍：点击了自定义菜
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::SetOpenItem(OrderDetail* item)
{
	try
	{
		CString userid;
		int auth=OrderDlg::RequeastAuth(userid,_T("open_item"));
		if (auth!=0)
			goto finally;
		int active=m_TabCtrl.GetActivePageIndex();
		if (m_checkDlg[active].m_nStatus==1)
		{
			POSMessageBox(IDS_NOACTION);
			goto finally;
		}
		LOG4CPLUS_INFO(log_pos,"OrderDlg::SetOpenItem Begin");
		//输入菜品名称
		CString strname;
		StringInputDlg dlg;
		theLang.LoadString(dlg.m_strTitle,IDS_INPUTITEMNAME);
		dlg.m_bAutoComplete=TRUE;
		if(dlg.DoModal()==IDOK)
		{
			strname=dlg.m_strInput;
			theApp.AddRecentString(strname);
		}
		else
			goto finally;
		strname.Trim();
		if (strname.GetLength()==0)
		{
			POSMessageBox(IDS_CANNOTEMPTY);
			goto finally;
		}
		//输入菜品价格
		double price=0;
		NumberInputDlg dlgn;
		theLang.LoadString(dlgn.m_strHint,IDS_INPUTPRICE);
		while(true)
		{
			if(dlgn.DoModal()==IDOK)
			{
				price=_wtof(dlgn.m_strNum);
				if(price>999999){
					POSMessageBox(IDS_TOOMUCH);
					dlgn.m_strNum.Empty();
				}
				else
					break;
			}
			else
				goto finally;
		}
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		item->item_price=price;
		ComputeTotalPrice(item);
		item->n_eattype=pApp->m_nEatType;
		wcsncpy_s(item->item_name,strname,63);
		item->n_checkID=active+1;
		if(m_checkDlg[active].AddOrderItem(item)>=0)
		{//合并
			delete item;
		}
		else
		{
			m_pOrderList->AddTail(item);
		}
		return;
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
	finally:
	{
		delete item;
	}
}
/************************************************************************
* 函数介绍：编码点菜
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedSearch()
{
	m_searchDlg->m_ctrlEdit.SetWindowText(_T(""));
	m_searchDlg->ShowWindow(SW_SHOW);
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedSearch");
}
LRESULT OrderDlg::OnMsgSearch(WPARAM wParam, LPARAM lParam)
{
	TRACE(_T("!!!! %s %d\n"),(LPCTSTR)m_searchDlg->m_strInput,lParam);
	try{
		CString strSQL;
		if(lParam==FALSE)
		{
			strSQL.Format(_T("SELECT * FROM (SELECT * FROM menu_item LEFT JOIN menu_item_class ON menu_item.class_id")
				_T("=menu_item_class.item_class_id WHERE item_type<>1 AND slu_id NOT IN(SELECT second_group_id FROM item_main_group WHERE")
				_T(" main_group_id=9999) AND(item_id LIKE \'%s%%\'OR nlu LIKE \'%%%s%%\' OR item_name1 LIKE \'%s%%\') LIMIT 16)")
				_T("AS T LEFT JOIN bargain_price_item ON T.item_id")
				_T("=bargain_price_item.bargain_item_number ;"),
				m_searchDlg->m_strInput,m_searchDlg->m_strInput,m_searchDlg->m_strInput);
		}else{//扫描枪，精确搜索
			strSQL.Format(_T("SELECT * FROM (SELECT * FROM menu_item LEFT JOIN menu_item_class ON menu_item.class_id")
				_T("=menu_item_class.item_class_id WHERE item_type<>1 AND slu_id NOT IN(SELECT second_group_id FROM item_main_group WHERE")
				_T(" main_group_id=9999) AND(nlu = \'%s\') LIMIT 3)")
				_T("AS T LEFT JOIN bargain_price_item ON T.item_id")
				_T("=bargain_price_item.bargain_item_number ;"),
				m_searchDlg->m_strInput);
		}
		AddMenuItem(strSQL);
		m_nCurrentPage=0;
		m_nPageCount=(int)ceil((float)m_Items.size()/m_nPageSize);
		ShowCurrentPage();
		if (wParam==1||wParam==2)
		{//回车消息,点击默认菜品
			if(m_Items.size()>=1)
				OnItemBnClicked(IDC_DYNAMIC_CTRL);
			else{//找不到则提示
				Beep( 1046, 200);
				Beep( 1046, 200);
				Beep( 1046, 200);
				POSMessageBox(IDS_NOTFINDITEM);
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
	return 0;
}
/************************************************************************
* 函数介绍：西餐按分类催菜
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedRemindDishByClass()
{
	try
	{
		CRemindClassDlg dlg;
		dlg.DoModal();
		if(dlg.m_Class<0)
			return;
		POSITION pos=NULL;
		vector<PrintItem> menu_list;
		for(pos=m_pOrderList->GetHeadPosition();pos!=NULL;)
		{
			OrderDetail* item=m_pOrderList->GetNext(pos);
			if(item->n_saved!=2)//未送厨
				continue;
			if(item->item_id<=0)//非菜品
				continue;
			if(item->family_group==dlg.m_Class)
			{
				PrintItem pItem;
				CopyPrintItem(pItem,item);
				menu_list.push_back(pItem);
				item->rush_times++;
				CString strSQL;
				strSQL.Format(_T("UPDATE order_detail SET rush=rush+1 WHERE order_detail_id=%d"),item->order_id);
				theDB.ExecuteSQL(strSQL);
			}
		}
		if (menu_list.size()==0)
		{//没有添加到合适的
			return;
		}
		//调用打印
		JSONVALUE root;
		root["template"]=TEMPLATE_REMIND;
		FormatTableInfo(root);
		PrintMenu(menu_list,root,macrosInt[_T("SENDITEM_ORDER")]);
		//刷新当前页的显示
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if(item->family_group==dlg.m_Class)
			{
				m_checkDlg[active].UpdateItemString(i,FALSE);
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：临时修改菜品子分类
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedChangFamilyGroup()
{
	int active=m_TabCtrl.GetActivePageIndex();
	if (m_checkDlg[active].m_nStatus==1)
	{
		POSMessageBox(IDS_NOACTION);
		return;
	}
	try
	{
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{
			return;
		}
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedChangPrintClass Begin");
		CRemindClassDlg dlg;
		dlg.DoModal();
		if(dlg.m_Class<0)
			return;
		CString strDetail;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->item_id<0)
					continue;
				item->family_group=dlg.m_Class;
				if(item->n_saved!=0)
				{
					CString strSQL;
					strSQL.Format(_T("UPDATE order_detail SET print_class=%d WHERE order_detail_id=%d")
						,item->family_group,item->order_id);
					theDB.ExecuteSQL(strSQL);
				}
			}
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：催菜
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedRemindDish()
{
	try
	{
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedRemindDish Begin");
		int active=m_TabCtrl.GetActivePageIndex();
		if (m_checkDlg[active].m_nStatus==1)
		{
			return;
		}
		RemindDishDlg dlg;
		dlg.pCheckDlg=&m_checkDlg[active];
		if (dlg.DoModal()==IDCANCEL)
		{
			return;
		}
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList->GetSelCount()==0)
		{
			POSMessageBox(IDS_SELECTITEM);
			return;
		}
		CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
		int length=ptrDetailList->GetCount();
		vector<PrintItem> menu_list;
		CTime time=CTime::GetCurrentTime();
		CString str2;
		theLang.LoadString(str2,IDS_WAITMK);
		for(int i=0;i<length;i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item->item_id<=0||item->n_saved!=2)
					continue;//未送厨的不能催菜
				item->rush_times++;
				CString strSQL;
				strSQL.Format(_T("UPDATE order_detail SET rush=rush+1 WHERE order_detail_id=%d"),item->order_id);
				theDB.ExecuteSQL(strSQL);
				if(wcsstr(item->description,str2))
				{
					CString str=item->description;
					int index = str.Find(str2);
					str.Delete(index,str2.GetLength());
					theLang.LoadString(str2,IDS_MAKENOW);
					str.Append(str2);
					wcsncpy_s(item->description,str,99);
					if(item->n_saved!=0)
					{
						CString strSQL;
						strSQL.Format(_T("UPDATE order_detail SET description=\'%s\' WHERE order_detail_id=%d;")
							,item->description,item->order_id);
						theDB.ExecuteSQL(strSQL);
					}
				}
				m_checkDlg[active].UpdateItemString(i,FALSE);
				PrintItem pItem;
				CopyPrintItem(pItem,item);
				menu_list.push_back(pItem);
				//设置未选中
				ptrDetailList->SetSel(i,FALSE);
			}
		}
		if (menu_list.size()==0)
		{//没有添加到合适的
			POSMessageBox(IDS_REMINDSEND);
			return;
		}
		//调用打印
		JSONVALUE root;
		root["template"]=TEMPLATE_REMIND;
		FormatTableInfo(root);
		PrintMenu(menu_list,root,macrosInt[_T("SENDITEM_ORDER")]);
		POSMessageBox(IDS_REMINDOK);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：弹出修改对话框
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedAlterItem()
{
//	m_alterDlg->ShowWindow(SW_SHOW);
}
/************************************************************************
* 函数介绍：免费赠送菜品
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedFreeDish()
{
	int active=m_TabCtrl.GetActivePageIndex();
	if (m_checkDlg[active].m_nStatus==1)
	{
		POSMessageBox(IDS_NOACTION);
		return;
	}
	try
	{
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		if (ptrDetailList->GetSelCount()==0)
		{
			return;
		}
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedFreeDish Begin");
		CString strDetail;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item==NULL||item->item_id<0)
					continue;
				CString userid;
				WCHAR username[10];
				double total;
				if (item->b_reprice)
				{//改价过的菜品
					total=item->orignal_price*item->quantity;
					if (total<item->total_price)
						total=item->total_price;
				}
				else
					total=item->total_price;
				int auth=OrderDlg::RequeastAuth(userid,_T("authority_11"),total,username);
				if(auth==0)
				{
					//忽略套餐内菜品
					if(item->n_belong_item<0)
					{
						ptrDetailList->SetSel(i,FALSE);
						continue;
					}
					if(item->b_isfree)
					{
						item->b_isfree=FALSE;
						ComputeTotalPrice(item);
						CString str2;
						theLang.LoadString(IDS_CANCELFREE,str2);
						strDetail.AppendFormat(_T("%s(%s) "),item->item_name,str2);
					}
					else
					{
						item->total_price=0;
						item->tax_amount=0;
						item->b_isfree=TRUE;
						wcsncpy_s(item->authID,userid,9);
						wcsncpy_s(item->authUser,username,9);
						strDetail.AppendFormat(_T("%s "),item->item_name);
					}
					m_checkDlg[active].UpdateItemString(i,FALSE);
					if (item->n_saved!=0)
					{
						OpenDatabase();
						CString strSQL;
						strSQL.Format(_T("UPDATE order_detail SET actual_price=\'%f\',is_discount=%d,auth_id=\'%s\',auth_name=\'%s\' WHERE order_detail_id=\'%d\';"),
							item->total_price,item->b_isfree,userid,username,item->order_id);
						theDB.ExecuteSQL(strSQL);
					}
				}
				else if(auth<0)
				{//取消或者授权失败
					break;
				}
			}
		}
		m_checkDlg[active].ComputeSubtotal();
		UpdateSencondScreen(active,NULL);
		CPOSClientApp::CriticalLogs(OPR_FREE,strDetail);
		if(theApp.m_VCR.IsOpen())
		{
			CString strLine;
			strLine.Format(_T("FOR FREE %s\n"),strDetail);
			theApp.m_VCR.Print(strLine);
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：通知厨房，叫起、等叫等信息
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedNotifyKitchen()
{
	int active=m_TabCtrl.GetActivePageIndex();
	if (m_checkDlg[active].m_nStatus==1)
	{
		return;
	}
	CPOSClientApp* pApp=(CPOSClientApp*)AfxGetApp();
	NotifyKitchenDlg dlg;
	CString str2;
	theLang.LoadString(str2,IDS_SENDFROM);
	dlg.m_strTtitle.AppendFormat(str2,pApp->m_strTblName);
	dlg.DoModal();
}
/************************************************************************
* 函数介绍：添加做法备注信息
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedAddRequest()
{
	int active=m_TabCtrl.GetActivePageIndex();
	CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
	if (ptrDetailList==NULL)
		return;
	if (ptrDetailList->GetSelCount()==0)
	{//选中最后一个
		ptrDetailList->SetSel(ptrDetailList->GetCount()-1);
	}

	int selCount=ptrDetailList->GetSelCount();
	OrderDetail* pTmpOrder=NULL;
	CString desc;
	int selIndex=-1;
	BOOL bSetAll=TRUE,bFirst=TRUE;

	for(int  i=0;i<ptrDetailList->GetCount();i++)
	{
		if(ptrDetailList->GetSel(i))
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if (item==NULL||item->item_id<0||item->n_saved==2)
				return;
			if (item->n_belong_item>0)
			{//调味品
				selCount--;
				continue;
			}
			if(bFirst)
			{//第一个选中
				bFirst=FALSE;
				desc.Format(_T("%s"),item->description);
				pTmpOrder=item;
				selIndex=i;
			}
			else if(desc.Compare(item->description)!=0)
			{//内容不相同
				desc.Empty();
				bSetAll=FALSE;
				break;
			}
		}
	}
	if (pTmpOrder==NULL)
		return;//选中的不符合修改条件
	NotifyKitchenDlg dlg;
	if(!bSetAll)
		dlg.m_nShowHint=1;
	theLang.LoadString(dlg.m_strTtitle,IDS_ADDREQUEST);
	dlg.m_strText=desc;
	dlg.type=1;
	dlg.pBelongItem=pTmpOrder;
	dlg.m_insertIdx=selIndex+1;
	dlg.pOrderDlg=this;
	if(dlg.DoModal()==IDCANCEL)
	{//取消选中
		ptrDetailList->SetSel(selIndex,FALSE);
		return;
	}
	CString strSQL;
	for(int  i=0;i<ptrDetailList->GetCount();i++)
	{
		if(ptrDetailList->GetSel(i))
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if (item==NULL||item->item_id<0||item->n_saved==2)
				continue;
			if (item->n_belong_item>0)
			{
				ptrDetailList->SetSel(i,FALSE);
				continue;
			}
			if (bSetAll)
			{
				wcsncpy_s(item->description,dlg.m_strText,99);
			}
			else
			{
				if(dlg.m_strText.GetLength()!=0)
					wcscat_s(item->description,100,dlg.m_strText);
				else
					item->description[0]=0;
			}
			m_checkDlg[active].UpdateItemString(i,FALSE);
			//更新数据库
			if(item->n_saved==1)
			{
				strSQL.Format(_T("UPDATE order_detail SET description=\'%s\' WHERE order_detail_id=%d;")
					,item->description,item->order_id);
				theDB.ExecuteSQL(strSQL);
			}
		}
	}
	UpdateSencondScreen(active,NULL);
}
/************************************************************************
* 函数介绍：套餐换菜
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedSwapCourse()
{
	try{
		int active=m_TabCtrl.GetActivePageIndex();
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList->GetSelCount()==0)
			return;
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedSwapCourse Begin");
		OrderDetail* pCourseItem=NULL;
		int courseStart=0;
		BOOL bFind=FALSE;
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if (item==NULL||item->item_id<0)
				continue;
			if (item->n_belong_item==0&&item->b_hascondiment)
			{
				pCourseItem=item;
				courseStart=i;
			}
			if(ptrDetailList->GetSel(i)&&item->n_belong_item<0)
			{
				bFind=TRUE;
				break;
			}
		}
		if (!bFind)
		{
			POSMessageBox(IDS_CHOOSECOURSE);
			return;
		}
		//套餐的所有菜品
		ComboMealDlg dlg;
		dlg.m_nMode=1;
		dlg.pParentItem=pCourseItem;
		int courseEnd=ptrDetailList->GetCount();
		for (int i=courseStart+1;i<courseEnd;i++)
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if (item->n_belong_item==0)
			{
				courseEnd=i;
				break;
			}
			dlg.m_subOrderList.AddTail(item);
		}
		if(dlg.DoModal()==IDCANCEL)
			return;
		vector<PrintItem> menu_list;
		CString strDetail;
		CString strSQL,str2;
		//退掉删除的菜
		if(dlg.m_voidOrderList.GetCount()>0)
		{
			for (int i=courseEnd-1;i>courseStart;i--)
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				POSITION pos=dlg.m_voidOrderList.Find(item);
				if (pos!=NULL)
				{
					if(item->n_saved)
					{
						theLang.LoadString(IDS_SWAPCOURSE2,str2);
						//如果已送厨，通知厨房
						if (item->n_saved==2&&item->item_id>0&&macrosInt[_T("NOPRINT_VOID")]!=1&&item->b_notprint==FALSE)
						{
							PrintItem pItem;
							CopyPrintItem(pItem,item);
							pItem.description=str2;
							menu_list.insert(menu_list.begin(),pItem);
						}
						strSQL.Format(_T("UPDATE order_detail SET is_return_item=1,return_reason=\'%s\',auth_id=\'%s\',return_time=now()")
							_T(",auth_name=\'%s\' WHERE order_detail_id=\'%d\';"),
							str2,theApp.m_strUserID,theApp.m_strUserName,item->order_id);
						theDB.ExecuteSQL(strSQL);
						if(item->item_id>0)
							strDetail.AppendFormat(_T("%s(%g%s) "),item->item_name,item->quantity,item->unit);
					}
					UpdateSoldoutInfo(item,item->quantity);
					ptrDetailList->DeleteString(i);
					POSITION pos2=m_pOrderList->Find(item);
					m_pOrderList->RemoveAt(pos2);
					delete item;
					courseEnd--;
				}

			}
			if(!strDetail.IsEmpty())
				CPOSClientApp::CriticalLogs(OPR_VOID,strDetail);
			if(macrosInt[_T("NOPRINT_VOID")]!=1)
			{
				//调用打印
				JSONVALUE root;
				root["template"]=TEMPLATE_VOID;
				FormatTableInfo(root);
				PrintMenu(menu_list,root,0);
			}
		}
		//原有菜更新显示
		for (int i=courseEnd-1;i>courseStart;i--)
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if(item->n_sortindex==1)
			{
				m_checkDlg[active].UpdateItemString(i,FALSE);
				//更新数据库
				if(item->n_saved==1)
				{
					strSQL.Format(_T("UPDATE order_detail SET description=\'%s\' WHERE order_detail_id=%d;")
						,item->description,item->order_id);
					theDB.ExecuteSQL(strSQL);
				}
			}

		}
		//插入新增的菜
		POSITION pos;
		for(pos=dlg.m_addOrderList.GetHeadPosition();pos!=NULL;)
		{
			BOOL bAdd=FALSE;
			OrderDetail* order=dlg.m_addOrderList.GetNext(pos);
			if(order->n_belong_item>0)
			{//调味品
				for(int i=courseEnd-1;i>courseStart;i--)
				{
					OrderDetail* pBefore=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
					if(pBefore<=NULL)
						continue;
					if (pBefore==order->belongto||pBefore->order_id==order->n_belong_item)
					{
						pBefore->b_hascondiment=TRUE;
						m_checkDlg[active].InsertCondiment(i+1,order,FALSE);
						bAdd=TRUE;
						break;
					}
				}
			}
			if(bAdd==FALSE)
				m_checkDlg[active].AddOrderItem(order,FALSE,courseEnd);
			courseEnd++;
			m_pOrderList->AddTail(order);
		}
		m_checkDlg[active].ComputeSubtotal();
		UpdateSencondScreen(active,NULL);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());	
	}
}
/************************************************************************
* 函数介绍：打印划菜单
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedPrintRound()
{
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedPrintRound Begin");
	int active=m_TabCtrl.GetActivePageIndex();
	PayDlg::PrintRound(m_checkDlg[active],active);
}
/************************************************************************
* 函数介绍：打印预结单
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedPrePrint()
{
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedPrint Begin");
	int active=m_TabCtrl.GetActivePageIndex();
	PayDlg::PrintCheck(m_checkDlg[active],active,0);
}
/************************************************************************
* 函数介绍：宴会功能，将已点菜品价格置为0，添加自定义的宴会价格
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedButtonParty()
{
	int active=m_TabCtrl.GetActivePageIndex();
	if (m_checkDlg[active].m_nStatus==1)
	{
		POSMessageBox(IDS_NOACTION);
		return;
	}
	try
	{
		CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
		if (ptrDetailList==NULL)
			return;
		LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedButtonParty Begin");
		//输入菜品价格
		double price=0;
		NumberInputDlg dlgn;
		theLang.LoadString(dlgn.m_strHint,IDS_INPUTPARTY);
		while(true)
		{
			if(dlgn.DoModal()==IDOK)
			{
				price=_wtof(dlgn.m_strNum);
				if(price>9999999){
					POSMessageBox(IDS_TOOMUCH);
					dlgn.m_strNum.Empty();
				}
				else
					break;
			}
			else
				return;
		}
		//改价为0
		for(int  i=0;i<ptrDetailList->GetCount();i++)
		{
			OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
			if (item==NULL||item->item_id<0)
				continue;
			if (item->b_reprice==FALSE)
			{//第一次改价
				item->orignal_price=item->item_price;
				item->b_reprice=TRUE;
			}
			item->total_price=0;
			item->tax_amount=0;
			m_checkDlg[active].UpdateItemString(i);
			if (item->n_saved!=0)
			{
				CString strSQL;
				strSQL.Format(_T("UPDATE order_detail SET actual_price=0,original_price=\'%f\' WHERE order_detail_id=\'%d\';"),
					item->orignal_price,item->order_id);
				theDB.ExecuteSQL(strSQL);
			}
		}
		//添加宴会金额
		OrderDetail* item=new OrderDetail;
		memset(item,0,sizeof(OrderDetail));
		item->item_id=1;
		item->quantity=1;
		item->item_price=price;
		item->total_price=price;
		item->n_discount_type=9;
		item->n_service_type=9;
		CString str2;
		theLang.LoadString(str2,IDS_PARTY);
		wcsncpy_s(item->item_name,str2,63);
		item->n_checkID=active+1;
		if(m_checkDlg[active].AddOrderItem(item)>=0)
		{//合并
			delete item;
		}
		else
		{
			m_pOrderList->AddTail(item);
		}
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
/************************************************************************
* 函数介绍：单品转台
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedButtonTransItem()
{
	if (theApp.m_bRefund)
		return;
	LOG4CPLUS_INFO(log_pos,"OrderDlg::OnBnClickedButtonTransItem Begin");
	int active=m_TabCtrl.GetActivePageIndex();
	if (m_checkDlg[active].m_nStatus==1)
	{
		return;
	}
	CString userid;
	int auth=OrderDlg::RequeastAuth(userid,_T("trans_table"));
	if(auth!=0)
		return;
	CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
	if (ptrDetailList->GetSelCount()==0)
	{
		POSMessageBox(IDS_SELECTITEM);
		return;
	}
	try{
		//选择要转到的桌号
		CFloorChooseDlg dlg;
		dlg.m_nFilterId=theApp.m_nTableId;
		theLang.LoadString(dlg.m_strPrefix,IDS_INPUTTOTBL);
		if(dlg.DoModal()==IDCANCEL)
			return ;
		//查询目标桌order_head_id
		CString strSQL,strDetail,strTblTo;
		long order_head;
		int length=ptrDetailList->GetCount();
		strSQL.Format(_T("SELECT * FROM order_head WHERE table_id=%d"),dlg.m_nTableId);
		CRecordset rs( &theDB);
		rs.Open( CRecordset::forwardOnly,strSQL);
		if(!rs.IsEOF())
		{
			CDBVariant variant;
			rs.GetFieldValue(_T("order_head_id"),variant);
			order_head=variant.m_lVal;
			rs.GetFieldValue(_T("table_name"),strTblTo);
		}
		else
		{//目标桌没有开台
			POSMessageBox(IDS_TBLNOTOPEN);
			return;
		}
		rs.Close();
		//目标桌已买单不能转台
		strSQL.Format(_T("SELECT status FROM order_head WHERE table_id=%d AND check_id=%d"),dlg.m_nTableId,active+1);
		rs.Open( CRecordset::forwardOnly,strSQL);
		if(!rs.IsEOF())
		{
			CDBVariant variant;
			rs.GetFieldValue(_T("status"),variant);
			if (variant.m_iVal==1)
			{
				POSMessageBox(IDS_NOACTION);
				return;
			}
		}
		rs.Close();
		strDetail.Format(_T("[%s] -> [%s] "),theApp.m_strTblName,strTblTo);
		long courseNum=0;//是否有套餐
		for(int i=0;i<ptrDetailList->GetCount();i++)
		{
			if(ptrDetailList->GetSel(i))
			{
				OrderDetail* item=(OrderDetail*)ptrDetailList->GetItemDataPtr(i);
				if (item->item_id<=0)
					continue;
				if (item->b_hascondiment==ITEM_SET)
				{
					if(item->n_saved==0)
						courseNum=-item->item_id;
					else
						courseNum=-item->order_id;
				}

				if (item->n_belong_item<0&&item->n_belong_item!=courseNum)
					continue;//如果单独选套餐中的配菜,不能转台
				//修改数据库
				if(item->order_id>0)
				{
					strSQL.Format(_T("UPDATE order_detail SET order_head_id=%d WHERE order_detail_id=%d"),order_head,item->order_id);
					theDB.ExecuteSQL(strSQL);
				}
				else
				{//未存储,新增到数据库
					COrder_Detail rsDetail(&theDB);
					rsDetail.Open(CRecordset::snapshot, NULL, CRecordset::appendOnly);
					rsDetail.AddNew();
					SetRsDetail(rsDetail,item);
					rsDetail.m_order_head_id=order_head;
					rsDetail.Update();
					rsDetail.Close();
				}
				//ptrDetailList->DeleteString(i);
				m_checkDlg[active].DeleteOrderItem(i);
				i--;
				POSITION pos=m_pOrderList->Find(item);
				if (pos!=NULL)
				{
					m_pOrderList->RemoveAt(pos);
				}
				strDetail.AppendFormat(_T("%s "),item->item_name);
				delete item;
			}
		}
		m_checkDlg[active].ComputeSubtotal();
		CPOSClientApp::CriticalLogs(OPR_TRANS,strDetail);
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}

void OrderDlg::OnBnClickedSelectAll()
{
	int active=m_TabCtrl.GetActivePageIndex();
	if (m_checkDlg[active].m_nStatus==1)
	{
		return;
	}
	CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
	if (ptrDetailList->GetCount()<=0)
		return;
	if(ptrDetailList->GetSel(0))
	{
		ptrDetailList->SelItemRange(FALSE,0,ptrDetailList->GetCount());
	}
	else
	{
		ptrDetailList->SelItemRange(TRUE,0,ptrDetailList->GetCount());
	}
}
/************************************************************************
* 函数介绍：整桌备注
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
void OrderDlg::OnBnClickedTableRemark()
{
	//查询并显示
	try{
		NotifyKitchenDlg dlg;
		CRecordset rs(&theDB);
		CString strSQL,str2;
		// 		strSQL.Format(_T("SELECT remark FROM order_head WHERE order_head_id=\'%d\' AND remark IS NOT NULL")
		// 			,theApp.m_nOrderHeadid);
		// 		rs.Open(CRecordset::forwardOnly,strSQL);
		// 		if (!rs.IsEOF())
		// 		{
		// 			rs.GetFieldValue((short)0,dlg.m_strText);
		// 		}
		// 		rs.Close();
		dlg.m_strText=m_strTblRemark;
		dlg.type=2;
		theLang.LoadString(IDS_TABLEREMARK,str2);
		dlg.m_strTtitle=str2;
		if(dlg.DoModal()==IDOK)
		{
			// 			strSQL.Format(_T("UPDATE order_head SET remark=\'%s\' WHERE order_head_id=\'%d\'")
			// 				,dlg.m_strText,theApp.m_nOrderHeadid);
			// 			theDB.ExecuteSQL(strSQL);
			m_strTblRemark=dlg.m_strText;
		}
	}
	catch(CDBException* e)
	{
		LOG4CPLUS_ERROR(log_pos,(LPCTSTR)e->m_strError);
		e->Delete();
	}
	catch(...)
	{
		LOG4CPLUS_ERROR(log_pos,"Catch Exception GetLastError="<<GetLastError());
	}
}
void OrderDlg::OnBnClickedRefund()
{
	if(theApp.m_bRefund)
	{
		theApp.m_bRefund=FALSE;
		UpdateHint(_T(""));
	}
	else
	{
		CString userid;
		WCHAR authUser[10];
		int auth=OrderDlg::RequeastAuth(userid,_T("refund"),1,authUser);
		if (auth!=0)
			return;
		theApp.m_strUserID=userid;
		theApp.m_strUserName.Format(_T("%s"),authUser);
		CVoidReasonDlg reasonDlg;
		if(reasonDlg.DoModal()==IDCANCEL)
			return;
		theApp.m_strRefundReason=reasonDlg.m_strReason;
		theApp.m_nGuests=0;
		theApp.m_bRefund=TRUE;
		CString str2;
		theLang.LoadString(str2,IDS_REFUND);
		UpdateHint(str2);
		if(theApp.m_VCR.IsOpen())
		{
			theApp.m_VCR.Print(_T("REFUND\n"));
		}
		CPOSClientApp::CriticalLogs(OPR_REFUND,theApp.m_strTblName);
	}
}
BOOL OrderDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
		if(theApp.m_pMain->GetActiveIndex()!=DLG_ORDER)
			return TRUE;
		switch (pMsg->wParam)
		{
		case VK_END:
		case VK_RETURN:
			OnBnClickedButtonPay();
			return TRUE;
		case VK_MULTIPLY:
			OnBnClickedQuantity();
			return TRUE;
		case VK_DELETE:
			OnBnClickedButtonVoid();
			return TRUE;
		case VK_F11:
			OnBnClickedPrintRound();
			return TRUE;
		case VK_F1:
			{
				CString userid;
				int auth=OrderDlg::RequeastAuth(userid,_T("open_drawer"));
				if(auth!=0)
					return TRUE;
				theApp.OpenDrawer();
			}
			return TRUE;
		case VK_ADD:
		case VK_SUBTRACT:
		case VK_LEFT:
		case VK_RIGHT:
			{
				int active=m_TabCtrl.GetActivePageIndex();
				CListBoxEx* ptrDetailList=&m_checkDlg[active].m_ctrlDetailList;
				if (ptrDetailList->GetSelCount()==0)
				{//选中最后一个
					ptrDetailList->SetSel(ptrDetailList->GetCount()-1);
				}
				if(pMsg->wParam==VK_ADD||pMsg->wParam==VK_RIGHT)
					OnBnClickedIncrease();
				else
					OnBnClickedDecrease();

				return TRUE;
			}
		case VK_UP:
			{
				CListBoxEx* ptrDetailList=&m_checkDlg[m_TabCtrl.GetActivePageIndex()].m_ctrlDetailList;
				if (ptrDetailList->GetSelCount()==0)
				{//选中最后一个
					ptrDetailList->SetSel(ptrDetailList->GetCount()-1);
				}
				else
				{
					BOOL bSet=FALSE;
					for(int  i=ptrDetailList->GetCount()-1;i>=0;i--)
					{
						if (bSet)
						{
							ptrDetailList->SetSel(i,FALSE);
							continue;
						}
						if(ptrDetailList->GetSel(i))
						{
							ptrDetailList->SetSel(i,FALSE);
							i--;
							if(i>=0)
								ptrDetailList->SetSel(i,TRUE);
							bSet=TRUE;
						}
					}
				}
				return TRUE;
			}
		case VK_DOWN:
			{
				CListBoxEx* ptrDetailList=&m_checkDlg[m_TabCtrl.GetActivePageIndex()].m_ctrlDetailList;
				if (ptrDetailList->GetSelCount()==0)
				{//选中第一个
					ptrDetailList->SetSel(0);
				}
				else
				{
					BOOL bSet=FALSE;
					for(int i=0;i<ptrDetailList->GetCount();i++)
					{
						if (bSet)
						{
							ptrDetailList->SetSel(i,FALSE);
							continue;
						}
						if(ptrDetailList->GetSel(i))
						{
							ptrDetailList->SetSel(i,FALSE);
							i++;
							ptrDetailList->SetSel(i,TRUE);
							bSet=TRUE;
						}
					}
				}
				return TRUE;
			}
		}
		if(m_bCloseSearch==FALSE)
		{
			if ((pMsg->wParam>='A'&&pMsg->wParam<='Z')||(pMsg->wParam>='0'&&pMsg->wParam<='9'))
			{
				m_searchDlg->ShowWindow(SW_SHOW);
				CString msg;
				msg.AppendChar(pMsg->wParam);
				m_searchDlg->m_ctrlEdit.SetWindowText(msg);
				m_searchDlg->m_ctrlEdit.SetFocus();
				m_searchDlg->m_ctrlEdit.SetEditSel(1,1);
			}
			else if (pMsg->wParam>=VK_NUMPAD0&&pMsg->wParam<=VK_NUMPAD9)
			{
				m_searchDlg->ShowWindow(SW_SHOW);
				CString msg;
				msg.Format(_T("%d"),pMsg->wParam-VK_NUMPAD0);
				m_searchDlg->m_ctrlEdit.SetWindowText(msg);
				m_searchDlg->m_ctrlEdit.SetFocus();
				m_searchDlg->m_ctrlEdit.SetEditSel(1,1);
			}
			return CPosPage::PreTranslateMessage(pMsg);
		}
	}
	return CPosPage::PreTranslateMessage(pMsg);
}