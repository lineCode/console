#include "stdafx.h"
#include "resource.h"

#include "Cursors.h"
#include "DlgSettingsTabs.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsTabs::DlgSettingsTabs(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
, m_page1()
, m_page2()
{
	IDD = IDD_SETTINGS_TABS;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_tabSettings.Load(m_pOptionsRoot);
  m_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
	m_listCtrl.Attach(GetDlgItem(IDC_LIST_TABS));

	m_listCtrl.SetExtendedListViewStyle(m_listCtrl.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT);
	m_listCtrl.InsertColumn(0, L"Tab name");
	m_listCtrl.SetColumnWidth(0, 188);
	m_listCtrl.SetImageList(m_ImageList, LVSIL_SMALL);
	m_listCtrl.SetIconSpacing(16, 16);

	TabDataVector::iterator	it = m_tabSettings.tabDataVector.begin();
	for (; it != m_tabSettings.tabDataVector.end(); ++it)
	{
		CIcon tabSmallIcon;
		if ((*it)->bUseDefaultIcon || ((*it)->strIcon.length() > 0))
		{
			if ((*it)->strIcon.length() > 0)
			{
				tabSmallIcon.Attach(
					static_cast<HICON>(
						::LoadImage(
							NULL,
							Helpers::ExpandEnvironmentStrings((*it)->strIcon).c_str(),
							IMAGE_ICON,
							16,
							16,
							LR_DEFAULTCOLOR|LR_LOADFROMFILE
						)
					)
				);
			}
			else if ((*it)->bUseDefaultIcon)
			{
				tabSmallIcon.Attach(
					static_cast<HICON>(
						::LoadImage(
							::GetModuleHandle(NULL),
							MAKEINTRESOURCE(IDR_MAINFRAME),
							IMAGE_ICON,
							16,
							16,
							LR_DEFAULTCOLOR
						)
					)
				);
			}
		}
		int nIcon = tabSmallIcon.m_hIcon? m_ImageList.AddIcon(tabSmallIcon.m_hIcon) : -1;
		int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), (*it)->strTitle.c_str(), nIcon);
		m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(it->get()));
	}

	if (m_listCtrl.GetItemCount() < 2) GetDlgItem(IDC_BTN_DELETE).EnableWindow(FALSE);

	m_tabCtrl.Attach(GetDlgItem(IDC_TABS));

	m_tabCtrl.InsertItem(0, L"Main");
	m_tabCtrl.InsertItem(1, L"Background");

	CRect	rect;
	m_tabCtrl.GetWindowRect(&rect);
	ScreenToClient(&rect);

	rect.DeflateRect(10, 25, 10, 10);

	m_page1.Create(m_hWnd, rect);
	m_page1.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
	m_page1.ShowWindow(SW_SHOW);

	m_page2.Create(m_hWnd, rect);
	m_page2.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);

	m_tabCtrl.SetCurSel(0);
	m_listCtrl.SelectItem(0);

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnTabTitleChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_listCtrl.SetItemText(m_listCtrl.GetSelectedIndex(), 0, m_page1.GetTabTitle());
	return 0;
}

LRESULT DlgSettingsTabs::OnTabIconChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  wstring strIcon         = m_page1.GetTabIcon();
  bool    bUseDefaultIcon = m_page1.UseDefaultIcon() ? true : false;

  CIcon tabSmallIcon;
  if (bUseDefaultIcon || (!strIcon.empty()))
  {
    if (!strIcon.empty())
    {
      tabSmallIcon.Attach(
        static_cast<HICON>(
          ::LoadImage(
            NULL,
            Helpers::ExpandEnvironmentStrings(strIcon).c_str(),
            IMAGE_ICON,
            16,
            16,
            LR_DEFAULTCOLOR|LR_LOADFROMFILE
          )
        )
      );
    }
    else if (bUseDefaultIcon)
    {
      tabSmallIcon.Attach(
        static_cast<HICON>(
          ::LoadImage(
            ::GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDR_MAINFRAME),
            IMAGE_ICON,
            16,
            16,
            LR_DEFAULTCOLOR
          )
        )
      );
    }
  }
  int nIcon = tabSmallIcon.m_hIcon? m_ImageList.AddIcon(tabSmallIcon.m_hIcon) : -1;
  // list control is not refreshed when an empty icon is set ...
  // so the text is updated too !
  m_listCtrl.SetItem(m_listCtrl.GetSelectedIndex(), 0, LVIF_TEXT|LVIF_IMAGE, m_page1.GetTabTitle(), nIcon, 0, 0, 0);

  return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		m_page1.Save();
		m_page2.Save();

		DoDataExchange(DDX_SAVE);

		m_tabSettings.Save(m_pOptionsRoot);

		TabSettings& tabSettings = g_settingsHandler->GetTabSettings();

		tabSettings.tabDataVector.clear();
		tabSettings.tabDataVector.insert(
									tabSettings.tabDataVector.begin(), 
									m_tabSettings.tabDataVector.begin(), 
									m_tabSettings.tabDataVector.end());
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ConsoleSettings& consoleSettings = g_settingsHandler->GetConsoleSettings();

	shared_ptr<TabData>	tabData(new TabData(consoleSettings.strShell, consoleSettings.strInitialDir));

	m_tabSettings.tabDataVector.push_back(tabData);

	int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), tabData->strTitle.c_str(), -1);
	m_listCtrl.SetItemData(nItem, reinterpret_cast<DWORD_PTR>(tabData.get()));

	m_listCtrl.SelectItem(nItem);
	GetDlgItem(IDC_BTN_DELETE).EnableWindow();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnClone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();

	m_page1.Save();
	m_page2.Save();

	shared_ptr<TabData>	tabData(new TabData(*(m_tabSettings.tabDataVector[nItem].get())));

	m_tabSettings.tabDataVector.insert(m_tabSettings.tabDataVector.begin() + nItem + 1, tabData);
	int nItemClone = m_listCtrl.InsertItem(nItem + 1, tabData->strTitle.c_str(), -1);
	m_listCtrl.SetItemData(nItemClone, reinterpret_cast<DWORD_PTR>(tabData.get()));

	m_listCtrl.SelectItem(nItemClone);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();

	if (nItem > 0) MoveListItem(nItem, -1);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nItem = m_listCtrl.GetSelectedIndex();

	if (nItem < m_listCtrl.GetItemCount() - 1) MoveListItem(nItem, 1);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (::MessageBox(m_hWnd, L"Delete selected tab?", L"Console", MB_YESNO|MB_ICONQUESTION) != IDYES) return 0;

	int nItem = m_listCtrl.GetSelectedIndex();

	m_listCtrl.DeleteItem(nItem);
	m_tabSettings.tabDataVector.erase(m_tabSettings.tabDataVector.begin() + nItem);

	if (nItem > 0) --nItem;

	m_listCtrl.SelectItem(nItem);
	if (m_listCtrl.GetItemCount() < 2) GetDlgItem(IDC_BTN_DELETE).EnableWindow(FALSE);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	NMLISTVIEW*	pnmv		= reinterpret_cast<NMLISTVIEW*>(pnmh);

	if (pnmv->iItem < 0) return 0;

	if (pnmv->uNewState & LVIS_SELECTED)
	{
		// selecting new item
		m_page1.Load(m_tabSettings.tabDataVector[m_listCtrl.GetSelectedIndex()]);
		m_page2.Load(m_tabSettings.tabDataVector[m_listCtrl.GetSelectedIndex()]);
	}
	else if (pnmv->uOldState & LVIS_SELECTED)
	{
		// deselecting item
		m_page1.Save();
		m_page2.Save();
		m_listCtrl.SetItemText(pnmv->iItem, 0, m_page1.GetTabTitle());
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsTabs::OnTabItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
	int	nItem = m_tabCtrl.GetCurSel();

	if (nItem == 0)
	{
		m_page1.ShowWindow(SW_SHOW);
		m_page2.ShowWindow(SW_HIDE);
	}
	else if (nItem == 1)
	{
		m_page1.ShowWindow(SW_HIDE);
		m_page2.ShowWindow(SW_SHOW);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void DlgSettingsTabs::MoveListItem(int nItem, int nDirection)
{
	shared_ptr<TabData>	tmpData(*(m_tabSettings.tabDataVector.begin() + nItem));

	m_listCtrl.DeleteItem(nItem);
	int nNewItem = m_listCtrl.InsertItem(nItem + nDirection, tmpData->strTitle.c_str());

	m_tabSettings.tabDataVector.erase(m_tabSettings.tabDataVector.begin() + nItem);
	m_tabSettings.tabDataVector.insert(m_tabSettings.tabDataVector.begin() + nItem + nDirection, tmpData);

	m_listCtrl.SetItemData(nNewItem, reinterpret_cast<DWORD_PTR>(tmpData.get()));
	m_listCtrl.SelectItem(nNewItem);
}

//////////////////////////////////////////////////////////////////////////////

