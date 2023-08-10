//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include "base/qrs.h"
#include "base/dmap.h"
#include "base/gui.h"
#include "subscr.h"
#include "zq/zq_subscr.h"
#include "jwin.h"
#include "zq/zquest.h"
#include "base/zsys.h"
#include "zq/zq_misc.h"
#include "tiles.h"
#include "qst.h"
#include "init.h"
#include <assert.h>
#include "dialog/info.h"
#include "dialog/subscr_props.h"

#ifndef _MSC_VER
#include <strings.h>
#endif
#include <string.h>

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

extern void large_dialog(DIALOG *d);
extern void large_dialog(DIALOG *d, float RESIZE_AMT);

int32_t curr_subscreen_object;
char *str_oname;
ZCSubscreen subscr_edit;
bool sso_selection[MAXSUBSCREENITEMS];
SubscrWidget* propCopyWidg = nullptr;

gamedata *game;

void delete_subscreen(int32_t subscreenidx);

SubscrPage& subscr_edit_page()
{
	if(subscr_edit.pages.empty())
		subscr_edit.pages.emplace_back();
    if(subscr_edit.pages.size() >= subscr_edit.curpage)
		subscr_edit.curpage = 0;
	return subscr_edit.pages[subscr_edit.curpage];
}
SubscrWidget* subscr_edit_widg()
{
	SubscrPage& p = subscr_edit_page();
	if(p.contents.empty() || curr_subscreen_object < 0)
	{
		curr_subscreen_object=-1;
		return nullptr;
	}
	if(curr_subscreen_object >= p.contents.size())
		curr_subscreen_object = 0;
	
	return p.contents[curr_subscreen_object];
}

static ListData item_list(itemlist_num, &font);

int32_t sso_properties(SubscrWidget* widg, int32_t obj_ind)
{
    return call_subscrprop_dialog(widg,obj_ind) ? 0 : -1;
}

int32_t onBringToFront();
int32_t onBringForward();
int32_t onSendBackward();
int32_t onSendToBack();
int32_t onReverseArrangement();
int32_t onAlignLeft();
int32_t onAlignCenter();
int32_t onAlignRight();
int32_t onAlignTop();
int32_t onAlignMiddle();
int32_t onAlignBottom();
int32_t onDistributeLeft();
int32_t onDistributeCenter();
int32_t onDistributeRight();
int32_t onDistributeTop();
int32_t onDistributeMiddle();
int32_t onDistributeBottom();
int32_t onGridSnapLeft();
int32_t onGridSnapCenter();
int32_t onGridSnapRight();
int32_t onGridSnapTop();
int32_t onGridSnapMiddle();
int32_t onGridSnapBottom();
void copySSOProperties(SubscrWidget const* src, SubscrWidget* dest);

int32_t onSubscreenObjectProperties()
{
	SubscrPage& pg = subscr_edit_page();
	SubscrWidget* widg = subscr_edit_widg();
    if(widg)
    {
        if(sso_properties(widg,pg.cursor_pos)!=-1)
        {
            for(int32_t i=0; i<pg.contents.size(); i++)
            {
                if(!sso_selection[i])
                    continue;
                
                copySSOProperties(widg, pg.contents[i]);
            }
        }
    }
    
    return D_O_K;
}

int32_t onNewSubscreenObject();

int32_t onDeleteSubscreenObject()
{
	SubscrPage& pg = subscr_edit_page();
    size_t objs=pg.contents.size();
    
    if(objs==0)
        return D_O_K;
    
	//erase the one object
	auto cnt = curr_subscreen_object;
	for(auto it = pg.contents.begin(); it != pg.contents.end();)
	{
		if(cnt--) ++it;
		else
		{
			it = pg.contents.erase(it);
			break;
		}
	}
	
	//...shift the selection array
    for(int32_t i=curr_subscreen_object; i<objs-1; ++i)
    {
        sso_selection[i]=sso_selection[i+1];
    }
    
    sso_selection[objs-1]=false;
    
    if(curr_subscreen_object==objs-1)
        --curr_subscreen_object;
    
    update_sso_name();
    update_up_dn_btns();
    
    return D_O_K;
}

int32_t onAddToSelection()
{
    if(curr_subscreen_object >= 0)
    {
        sso_selection[curr_subscreen_object]=true;
    }
    
    return D_O_K;
}

int32_t onRemoveFromSelection()
{
    if(curr_subscreen_object >= 0)
    {
        sso_selection[curr_subscreen_object]=false;
    }
    
    return D_O_K;
}

int32_t onInvertSelection()
{
    for(int32_t i=0; i<subscr_edit_page().contents.size(); ++i)
    {
        sso_selection[i]=!sso_selection[i];
    }
    
    return D_O_K;
}

int32_t onClearSelection()
{
    for(int32_t i=0; i<MAXSUBSCREENITEMS; ++i)
    {
        sso_selection[i]=false;
    }
    
    return D_O_K;
}

int32_t onDuplicateSubscreenObject()
{
	SubscrPage& pg = subscr_edit_page();
    size_t objs = pg.contents.size();
    
    if(objs==0)
        return D_O_K;
    
    int32_t counter=0;
    
    for(int32_t i=0; i<objs; ++i)
    {
        if(sso_selection[i] || i==curr_subscreen_object)
        {
			SubscrWidget* widg = pg.contents[i]->clone();
			if(!widg) continue;
			
            widg->x+=zc_max(zinit.ss_grid_x,8);
            widg->y+=zc_max(zinit.ss_grid_y,8);
        }
    }
    
    update_sso_name();
    update_up_dn_btns();
    
    return D_O_K;
}

static int32_t onToggleInvis();
static int32_t onEditGrid();
static int32_t onSelectionOptions();
static int32_t onShowHideGrid();


static MENU subscreen_rc_menu[] =
{
    { (char *)"Properties ",       NULL,  NULL, 0, NULL },
    { (char *)"Copy Properties ",  NULL,  NULL, 0, NULL },
    { (char *)"Paste Properties ", NULL,  NULL, 0, NULL },
    { NULL,                        NULL,  NULL, 0, NULL }
};


int32_t d_subscreen_proc(int32_t msg,DIALOG *d,int32_t)
{
	switch(msg)
	{
	case MSG_CLICK:
	{
		SubscrPage& pg = subscr_edit_page();
		for(int32_t i=pg.contents.size()-1; i>=0; --i)
		{
			SubscrWidget* widg = pg.contents[i];
			int32_t x=(widg->getX()+widg->getXOffs())*2;
			int32_t y=widg->getY()*2;
			int32_t w=widg->getW()*2;
			int32_t h=widg->getH()*2;
			
			if(isinRect(gui_mouse_x(),gui_mouse_y(),d->x+x, d->y+y, d->x+x+w-1, d->y+y+h-1))
			{
				if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
				{
					if(sso_selection[i]==true)
					{
						sso_selection[i]=false;
					}
					else
					{
						sso_selection[curr_subscreen_object]=true;
						curr_subscreen_object=i;
						update_sso_name();
						update_up_dn_btns();
					}
				}
				else
				{
					onClearSelection();
					curr_subscreen_object=i;
					update_sso_name();
					update_up_dn_btns();
				}
				
				break;
			}
		}
		
		if(gui_mouse_b()&2) //right mouse button
		{
			object_message(d,MSG_DRAW,0);
			
			// Disable "Paste Properties" if the copy source is invalid
			if(!propCopyWidg || propCopyWidg->getType()==ssoNULL)
				subscreen_rc_menu[2].flags|=D_DISABLED;
			else
				subscreen_rc_menu[2].flags&=~D_DISABLED;
			
			int32_t m = popup_menu(subscreen_rc_menu,gui_mouse_x(),gui_mouse_y());
			
			switch(m)
			{
			case 0: // Properties
				onSubscreenObjectProperties();
				break;
			case 1: // Copy Properties
			{
				SubscrWidget* w = subscr_edit_widg();
				if(w)
					propCopyWidg = w->clone();
				else
				{
					delete propCopyWidg;
					propCopyWidg = nullptr;
				}
				break;
			}
			case 2: // Paste Properties
				if(propCopyWidg) // Hopefully unnecessary)
				{
					SubscrPage& pg = subscr_edit_page();
					for(int32_t i=0; i<pg.contents.size(); i++)
					{
						if(i == curr_subscreen_object || sso_selection[i])
							copySSOProperties(propCopyWidg, pg.contents[i]);
					}
				}
				break;
			}
		}
		
		return D_O_K;
	}
	break;
	
	case MSG_VSYNC:
		d->flags|=D_DIRTY;
		break;
		
	case MSG_DRAW:
	{
		Sitems.animate();
		BITMAP *buf = create_bitmap_ex(8,d->w,d->h);//In Large Mode, this is actually 2x as large as needed, but whatever.
		
		if(buf)
		{
			clear_bitmap(buf);
			ZCSubscreen* subs = (ZCSubscreen*)(d->dp);
			show_custom_subscreen(buf, subs, 0, 0, true, sspUP | sspDOWN | sspSCROLLING);
			
			SubscrPage& pg = subs->cur_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i == curr_subscreen_object)
				{
					auto c = i != curr_subscreen_object
						? vc(zinit.ss_bbox_2_color)
						: vc(zinit.ss_bbox_1_color);
					sso_bounding_box(buf, pg.contents[i], c);
				}
			}
			
			if(zinit.ss_flags&ssflagSHOWGRID)
			{
				for(int32_t x=zinit.ss_grid_xofs; x<d->w; x+=zinit.ss_grid_x)
				{
					for(int32_t y=zinit.ss_grid_yofs; y<d->h; y+=zinit.ss_grid_y)
					{
						buf->line[y][x]=vc(zinit.ss_grid_color);
					}
				}
			}
			
			stretch_blit(buf,screen,0,0,d->w/2,d->h/2,d->x,d->y,d->w,d->h);
			
			destroy_bitmap(buf);
		}
	}
	break;
	
	case MSG_WANTFOCUS:
		return D_WANTFOCUS;
		break;
	}
	
	return D_O_K;
}

int32_t onSSUp();
int32_t onSSDown();
int32_t onSSLeft();
int32_t onSSRight();
int32_t onSSPgDn();
int32_t onSSPgUp();

int32_t d_ssup_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_ssdn_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_sslt_btn_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_ssrt_btn_proc(int32_t msg,DIALOG *d,int32_t c);

int32_t onSSUp()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?-zinit.ss_grid_y:-1;
	
	SubscrPage& pg = subscr_edit_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_subscreen_object)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->h+=delta;
			}
			else
			{
				pg.contents[i]->y+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t onSSDown()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?zinit.ss_grid_y:1;
	
	SubscrPage& pg = subscr_edit_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_subscreen_object)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->h+=delta;
			}
			else
			{
				pg.contents[i]->y+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t onSSLeft()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?-zinit.ss_grid_x:-1;
	
	SubscrPage& pg = subscr_edit_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_subscreen_object)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->w+=delta;
			}
			else
			{
				pg.contents[i]->x+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t onSSRight()
{
	int32_t delta=(key[KEY_LSHIFT]||key[KEY_RSHIFT])?zinit.ss_grid_x:1;
	
	SubscrPage& pg = subscr_edit_page();
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(sso_selection[i] || i==curr_subscreen_object)
		{
			if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
			{
				pg.contents[i]->w+=delta;
			}
			else
			{
				pg.contents[i]->x+=delta;
			}
		}
	}
	
	return D_O_K;
}

int32_t d_ssup_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			onSSUp();
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			onSSDown();
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			onSSLeft();
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn2_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			onSSRight();
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_ssup_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			SubscrPage& pg = subscr_edit_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_subscreen_object)
				{
					--pg.contents[i]->h;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			SubscrPage& pg = subscr_edit_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_subscreen_object)
				{
					++pg.contents[i]->h;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			SubscrPage& pg = subscr_edit_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_subscreen_object)
				{
					--pg.contents[i]->w;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn3_proc(int32_t msg,DIALOG *d,int32_t c)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			jwin_button_proc(msg, d, c);
			SubscrPage& pg = subscr_edit_page();
			for(int32_t i=0; i<pg.contents.size(); ++i)
			{
				if(sso_selection[i] || i==curr_subscreen_object)
				{
					++pg.contents[i]->w;
				}
			}
			return D_O_K;
		}
		break;
	}
	
	return jwin_button_proc(msg, d, c);
}

int32_t d_ssup_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        subscr_edit.cur_page().move_cursor(SEL_UP);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        subscr_edit.cur_page().move_cursor(SEL_DOWN);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        subscr_edit.cur_page().move_cursor(SEL_LEFT);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn4_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        subscr_edit.cur_page().move_cursor(SEL_RIGHT);
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

const char *sso_textstyle[sstsMAX]=
{
    "sstsNORMAL", "sstsSHADOW", "sstsSHADOWU", "sstsOUTLINE8", "sstsOUTLINEPLUS", "sstsOUTLINEX", "sstsSHADOWED", "sstsSHADOWEDU", "sstsOUTLINED8", "sstsOUTLINEDPLUS", "sstsOUTLINEDX"
};

const char *sso_fontname[ssfMAX]=
{
    "ssfZELDA", "ssfSS1", "ssfSS2", "ssfSS3", "ssfSS4", "ssfZTIME", "ssfSMALL", "ssfSMALLPROP", "ssfZ3SMALL", "ssfGBLA", "ssfZ3",
    "ssfGORON", "ssfZORAN", "ssfHYLIAN1", "ssfHYLIAN2", "ssfHYLIAN3", "ssfHYLIAN4", "ssfPROP", "ssfGBORACLE", "ssfGBORACLEP",
    "ssfDSPHANTOM", "ssfDSPHANTOMP",
	
	"ssfAT800", "ssfACORN", "ssfALLEG", "ssfAPL2", "ssfAPL280", "ssfAPL2GS", "ssfAQUA", "ssfAT400", "ssfC64", "ssfC64HR",
	"ssfCGA", "ssfCOCO", "ssfCOCO2", "ssfCOUPE", "ssfCPC", "ssfFANTASY", "ssfFDSKANA", "ssfFDSLIKE", "ssfFDSROM", "ssfFF",
	"ssfFUTHARK", "ssfGAIA", "ssfHIRA", "ssfJP", "ssfKONG", "ssfMANA", "ssfML", "ssfMOT", "ssfMSX0", "ssfMSX1", "ssfPET",
	"ssfPSTART", "ssfSATURN", "ssfSCIFI", "ssfSHERW", "ssfSINQL", "ssfSPEC", "ssfSPECLG", "ssfTI99", "ssfTRS", "ssfZ2", "ssfZX", "ssfLISA"
	
};

const char *sso_colortype[2]=
{
    "ssctSYSTEM", "ssctMISC"
};

const char *sso_specialcolor[ssctMAX]=
{
    "ssctTEXT", "ssctCAPTION", "ssctOVERWBG", "ssctDNGNBG", "ssctDNGNFG", "ssctCAVEFG", "ssctBSDK", "ssctBSGOAL", "ssctCOMPASSLT", "ssctCOMPASSDK", "ssctSUBSCRBG", "ssctSUBSCRSHADOW",
    "ssctTRIFRAMECOLOR", "ssctBMAPBG", "ssctBMAPFG", "ssctHERODOT", "ssctMSGTEXT"
};

const char *sso_specialcset[sscsMAX]=
{
    "sscsTRIFORCECSET", "sscsTRIFRAMECSET", "sscsOVERWORLDMAPCSET", "sscsDUNGEONMAPCSET", "sscsBLUEFRAMECSET", "sscsHCPIECESCSET", "sscsSSVINECSET"
};

const char *sso_specialtile[ssmstMAX]=
{
    "ssmstSSVINETILE", "ssmstMAGICMETER"
};

const char *sso_counterobject[sscMAX]=
{
    "sscRUPEES", "sscBOMBS", "sscSBOMBS", "sscARROWS", "sscGENKEYMAGIC", "sscGENKEYNOMAGIC", "sscLEVKEYMAGIC",
    "sscLEVKEYNOMAGIC", "sscANYKEYMAGIC", "sscANYKEYNOMAGIC", "sscSCRIPT1", "sscSCRIPT2", "sscSCRIPT3", "sscSCRIPT4",
    "sscSCRIPT5", "sscSCRIPT6", "sscSCRIPT7", "sscSCRIPT8", "sscSCRIPT9", "sscSCRIPT10"
};

const char *sso_alignment[3]=
{
    "sstaLEFT", "sstaCENTER", "sstaRIGHT"
};

int32_t onActivePassive();

static MENU ss_arrange_menu[] =
{
    { (char *)"Bring to Front",       onBringToFront,          NULL, 0, NULL },
    { (char *)"Bring Forward",        onBringForward,          NULL, 0, NULL },
    { (char *)"Send Backward",        onSendBackward,          NULL, 0, NULL },
    { (char *)"Send to Back",         onSendToBack,            NULL, 0, NULL },
    { (char *)"Reverse",              onReverseArrangement,    NULL, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_grid_snap_menu[] =
{
    { (char *)"Left Edges",           onGridSnapLeft,          NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onGridSnapCenter,        NULL, 0, NULL },
    { (char *)"Right Edges",          onGridSnapRight,         NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"Top Edges",            onGridSnapTop,           NULL, 0, NULL },
    { (char *)"Vertical Centers",     onGridSnapMiddle,        NULL, 0, NULL },
    { (char *)"Bottom Edges",         onGridSnapBottom,        NULL, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_align_menu[] =
{
    { (char *)"Left Edges",           onAlignLeft,             NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onAlignCenter,           NULL, 0, NULL },
    { (char *)"Right Edges",          onAlignRight,            NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"Top Edges",            onAlignTop,              NULL, 0, NULL },
    { (char *)"Vertical Centers",     onAlignMiddle,           NULL, 0, NULL },
    { (char *)"Bottom Edges",         onAlignBottom,           NULL, 0, NULL },
    { (char *)"",                     NULL,                    NULL, 0, NULL },
    { (char *)"To Grid",              NULL,                    ss_grid_snap_menu, 0, NULL },
    { NULL,                           NULL,                    NULL, 0, NULL }
};

static MENU ss_distribute_menu[] =
{
    { (char *)"Left Edges",           onDistributeLeft,             NULL, 0, NULL },
    { (char *)"Horizontal Centers",   onDistributeCenter,           NULL, 0, NULL },
    { (char *)"Right Edges",          onDistributeRight,            NULL, 0, NULL },
    { (char *)"",                     NULL,                         NULL, 0, NULL },
    { (char *)"Top Edges",            onDistributeTop,              NULL, 0, NULL },
    { (char *)"Vertical Centers",     onDistributeMiddle,           NULL, 0, NULL },
    { (char *)"Bottom Edges",         onDistributeBottom,           NULL, 0, NULL },
    { NULL,                           NULL,                         NULL, 0, NULL }
};

static MENU ss_edit_menu[] =
{
    { (char *)"&New\tIns",           onNewSubscreenObject,                 NULL, 0, NULL },
    { (char *)"&Delete\tDel",        onDeleteSubscreenObject,              NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Duplicate",          onDuplicateSubscreenObject,           NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Properties",         onSubscreenObjectProperties,          NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Arrange",            NULL,                                 ss_arrange_menu, 0, NULL },
    { (char *)"Al&ign",              NULL,                                 ss_align_menu, 0, NULL },
    { (char *)"Dis&tribute",         NULL,                                 ss_distribute_menu, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"Switch Active/Passive",              onActivePassive,                      NULL, 0, NULL },
    { (char *)"",                    NULL,                                 NULL, 0, NULL },
    { (char *)"&Take Snapshot\tZ",   onSnapshot,                           NULL, 0, NULL },
    { NULL,                          NULL,                                 NULL, 0, NULL }
};

static MENU ss_view_menu[] =
{
    { (char *)"Show in&visible items",           onToggleInvis,                 NULL, 0, NULL },
    { (char *)"&Edit grid",                      onEditGrid,                    NULL, 0, NULL },
    { (char *)"&Show grid",                      onShowHideGrid,                NULL, 0, NULL },
    { NULL,                                 NULL,                          NULL, 0, NULL }
};

static MENU ss_selection_menu[] =
{
    { (char *)"&Add to Selection\tA",       onAddToSelection,                 NULL, 0, NULL },
    { (char *)"&Remove from Selection\tR",  onRemoveFromSelection,            NULL, 0, NULL },
    { (char *)"&Invert Selection\tI",       onInvertSelection,                NULL, 0, NULL },
    { (char *)"&Clear Selection\tC",        onClearSelection,                 NULL, 0, NULL },
    { (char *)"",                           NULL,                             NULL, 0, NULL },
    { (char *)"&Options",                   onSelectionOptions,               NULL, 0, NULL },
    { NULL,                                 NULL,                             NULL, 0, NULL }
};

static MENU subscreen_menu[] =
{
    { (char *)"&Edit",               NULL,                                 ss_edit_menu, 0, NULL },
    { (char *)"&View",               NULL,                                 ss_view_menu, 0, NULL },
    { (char *)"&Selection",          NULL,                                 ss_selection_menu, 0, NULL },
    { NULL,                         NULL,                                 NULL, 0, NULL }
};


static DIALOG subscreen_dlg[] =
{
    // (dialog proc)       (x)   (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,    0,      320,    240,  vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Subscreen Editor", NULL, NULL },
    { jwin_button_proc,     192,  215,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     255,  215,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_frame_proc,      4,    37,     260,    172,  0,                  0,                0,       0,          FR_DEEP,       0,       NULL, NULL, NULL },
    { d_subscreen_proc,     6,    39,     256,    168,  0,                  0,                0,       0,          0,             0,       NULL, NULL, NULL },
    // 5
    { d_box_proc,           11,   211,    181,    8,    0,                  0,                0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       11,   211,    181,    16,   0,                  0,                0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       4,    225,    30,     16,   0,                  0,                0,       0,          0,             0, (void *) "Name:", NULL, NULL },
    { jwin_edit_proc,       34,   221,    155,    16,   0,                  0,                0,       0,          64,            0,       NULL, NULL, NULL },
    
    { d_ssup_btn_proc,      284,  23,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x88", NULL, NULL },
    { d_ssdn_btn_proc,      284,  53,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x89", NULL, NULL },
    { d_sslt_btn_proc,      269,  38,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8A", NULL, NULL },
    { d_ssrt_btn_proc,      299,  38,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8B", NULL, NULL },
    
    { d_ssup_btn2_proc,     284,  70,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x88", NULL, NULL },
    { d_ssdn_btn2_proc,     284,  100,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x89", NULL, NULL },
    { d_sslt_btn2_proc,     269,  85,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8A", NULL, NULL },
    { d_ssrt_btn2_proc,     299,  85,     15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8B", NULL, NULL },
    
    { d_ssup_btn3_proc,     284,  117,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x98", NULL, NULL },
    { d_ssdn_btn3_proc,     284,  147,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x99", NULL, NULL },
    { d_sslt_btn3_proc,     269,  132,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x9A", NULL, NULL },
    { d_ssrt_btn3_proc,     299,  132,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x9B", NULL, NULL },
    
    { d_ssup_btn4_proc,     284,  164,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x88", NULL, NULL },
    { d_ssdn_btn4_proc,     284,  194,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x89", NULL, NULL },
    { d_sslt_btn4_proc,     269,  179,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8A", NULL, NULL },
    { d_ssrt_btn4_proc,     299,  179,    15,     15,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "\x8B", NULL, NULL },
    
    { jwin_menu_proc,       4,    23,     0,      13,    0,                 0,                0,       0,          0,             0, (void *) subscreen_menu, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_UP,        0, (void *) onSSUp, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_DOWN,      0, (void *) onSSDown, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_LEFT,      0, (void *) onSSLeft, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_RIGHT,     0, (void *) onSSRight, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_PGUP,      0, (void *) onSSPgUp, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_PGDN,      0, (void *) onSSPgDn, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'a',     0,          0,             0, (void *) onAddToSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'r',     0,          0,             0, (void *) onRemoveFromSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'i',     0,          0,             0, (void *) onInvertSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'c',     0,          0,             0, (void *) onClearSelection, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_INSERT,    0, (void *) onNewSubscreenObject, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                0,       0,          KEY_DEL,       0, (void *) onDeleteSubscreenObject, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'd',     0,          0,             0, (void *) onDuplicateSubscreenObject, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'e',     0,          0,             0, (void *) onSubscreenObjectProperties, NULL, NULL },
    { d_keyboard_proc,      0,     0,     0,       0,    0,                 0,                'z',     0,          0,             0, (void *) onSnapshot, NULL, NULL },
    { d_vsync_proc,         0,     0,     0,       0,    0,                 0,                0,       0,          0,             0,       NULL, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t onActivePassive()
{
    if(subscr_edit.sub_type == sstPASSIVE)
    {
        subscr_edit.sub_type = sstACTIVE;
        subscreen_dlg[3].h=172*2-4;
        subscreen_dlg[4].h=subscreen_dlg[3].h-4;
    }
	else
    {
        subscr_edit.sub_type = sstPASSIVE;
        subscreen_dlg[3].h=116;
        subscreen_dlg[4].h=subscreen_dlg[3].h-4;
    }
    
    return D_REDRAW;
}


const char *color_str[16] =
{
    "Black", "Blue", "Green", "Cyan", "Red", "Magenta", "Brown", "Light Gray",
    "Dark Gray", "Light Blue", "Light Green", "Light Cyan", "Light Red", "Light Magenta", "Yellow", "White"
};

const char *colorlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return color_str[index];
}

static ListData color_list(colorlist, &font);

static DIALOG grid_dlg[] =
{
    // (dialog proc)       (x)   (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,    0,      158,    120,  vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Edit Grid Properties", NULL, NULL },
    { jwin_button_proc,     18,    95,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     81,    95,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_text_proc,       6,     29,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "X Size:", NULL, NULL },
    { jwin_edit_proc,       42,    25,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    // 5
    { jwin_text_proc,       6,     49,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Y Size:", NULL, NULL },
    { jwin_edit_proc,       42,    45,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       78,    29,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "X Offset:", NULL, NULL },
    { jwin_edit_proc,       126,   25,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       78,    49,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Y Offset:", NULL, NULL },
    // 10
    { jwin_edit_proc,       126,   45,     26,    16,   0,                  0,                0,       0,          3,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       6,     69,    186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Color:", NULL, NULL },
    { jwin_droplist_proc,   36,    65,    116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

static DIALOG sel_options_dlg[] =
{
    // (dialog proc)       (x)    (y)    (w)     (h)   (fg)                (bg)              (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,     0,    225,   120,   vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Selection Options", NULL, NULL },
    { jwin_button_proc,     51,    95,    61,    21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,    114,    95,    61,    21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { jwin_frame_proc,       6,    28,   213,    51,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,     0,       NULL, NULL, NULL },
    { jwin_text_proc,       10,    25,    48,     8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) " Selection Outlines ", NULL, NULL },
    // 5
    { jwin_text_proc,       14,    41,   186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Primary Color:", NULL, NULL },
    { jwin_droplist_proc,   94,    37,   116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { jwin_text_proc,       14,    61,   186,    16,   0,                  0,                0,       0,          0,             0, (void *) "Secondary Color:", NULL, NULL },
    { jwin_droplist_proc,   94,    57,   116,    16,   0,                  0,                0,       0,          0,             0, (void *) &color_list, NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

const char *sso_str[ssoMAX]=
{
    "NULL", "(None)", "2x2 Frame", "Text", "Line", "Rectangle", "BS-Zelda Time", "Game Time", "Game Time (Quest Rule)", "Magic Meter", "Life Meter",
    "Button Item", "-Icon (Not Implemented)", "Counter", "Counter Block", "Minimap Title", "Minimap", "Large Map", "Background Color", "Current Item", "-Item (Not Implemented)",
    "Triangle Frame", "McGuffin Piece", "Tile Block", "Minitile", "Selector 1", "Selector 2", "Magic Gauge Piece", "Life Gauge Piece", "Text Box", "-Current Item -> Tile (Not Implemented)",
    "-Selected Item -> Tile (Not Implemented)", "-Current Item -> Text (Not Implemented)", "-Current Item Name (Not Implemented)", "Selected Item Name",
    "-Current Item Class -> Text (Not Implemented)", "-Current Item Class Name (Not Implemented)", "-Selected Item Class Name (Not Implemented)"
};

char *sso_name(int32_t type)
{
    char *tempname;
    tempname=(char*)malloc(255);
    
    if(type>=0 && type <ssoMAX)
    {
        sprintf(tempname, "%s", sso_str[type]);
    }
    else
    {
        sprintf(tempname, "INVALID OBJECT!  type=%d", type);
    }
    
    return tempname;
}

sso_struct bisso[ssoMAX];
int32_t bisso_cnt=-1;

void build_bisso_list()
{
    int32_t start=1;
    bisso_cnt=0;
    
    for(int32_t i=start; i<ssoMAX; i++)
    {
        if(sso_str[i][0]!='-')
        {
            bisso[bisso_cnt].s = (char *)sso_str[i];
            bisso[bisso_cnt].i = i;
            ++bisso_cnt;
        }
    }
    
    for(int32_t i=start; i<bisso_cnt-1; i++)
    {
        for(int32_t j=i+1; j<bisso_cnt; j++)
        {
            if(stricmp(bisso[i].s,bisso[j].s)>0)
            {
                std::swap(bisso[i],bisso[j]);
            }
        }
    }
}

const char *ssolist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = bisso_cnt;
        return NULL;
    }
    
    return bisso[index].s;
}

static ListData sso_list(ssolist, &font);

static DIALOG ssolist_dlg[] =
{
    // (dialog proc)      (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,        0,    0,   255,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Select Object Type", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_abclist_proc,    4,   24,   247,  95,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0, (void *) &sso_list, NULL, NULL },
    { jwin_button_proc,    65,  123,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,   128,  123,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void doNewSubscreenObject(int32_t type)
{
	SubscrWidget* widg = SubscrWidget::newType(type);
	widg->posflags = sspUP | sspDOWN | sspSCROLLING;
	widg->w=1;
	widg->h=1;
	
	int32_t temp_cso=curr_subscreen_object;
	curr_subscreen_object=subscr_edit_page().contents.size();
	
	SubscrPage& pg = subscr_edit_page();
	if(sso_properties(widg,pg.contents.size())!=-1)
	{
		pg.contents.push_back(widg);
		update_sso_name();
		update_up_dn_btns();
	}
	else
	{
		curr_subscreen_object=temp_cso;
		delete widg;
	}
}

std::string getssname(int32_t type)
{
	switch(type)
	{
		case ssoNULL: return "ssoNULL";
		case ssoNONE: return "ssoNONE";
		case sso2X2FRAME: return "sso2X2FRAME";
		case ssoTEXT: return "ssoTEXT";
		case ssoLINE: return "ssoLINE";
		case ssoRECT: return "ssoRECT";
		case ssoBSTIME: return "ssoBSTIME";
		case ssoTIME: return "ssoTIME";
		case ssoSSTIME: return "ssoSSTIME";
		case ssoMAGICMETER: return "ssoMAGICMETER";
		case ssoLIFEMETER: return "ssoLIFEMETER";
		case ssoBUTTONITEM: return "ssoBUTTONITEM";
		case ssoICON: return "ssoICON";
		case ssoCOUNTER: return "ssoCOUNTER";
		case ssoCOUNTERS: return "ssoCOUNTERS";
		case ssoMINIMAPTITLE: return "ssoMINIMAPTITLE";
		case ssoMINIMAP: return "ssoMINIMAP";
		case ssoLARGEMAP: return "ssoLARGEMAP";
		case ssoCLEAR: return "ssoCLEAR";
		case ssoCURRENTITEM: return "ssoCURRENTITEM";
		case ssoITEM: return "ssoITEM";
		case ssoTRIFRAME: return "ssoTRIFRAME";
		case ssoMCGUFFIN: return "ssoMCGUFFIN";
		case ssoTILEBLOCK: return "ssoTILEBLOCK";
		case ssoMINITILE: return "ssoMINITILE";
		case ssoSELECTOR1: return "ssoSELECTOR1";
		case ssoSELECTOR2: return "ssoSELECTOR2";
		case ssoMAGICGAUGE: return "ssoMAGICGAUGE";
		case ssoLIFEGAUGE: return "ssoLIFEGAUGE";
		case ssoTEXTBOX: return "ssoTEXTBOX";
		case ssoCURRENTITEMTILE: return "ssoCURRENTITEMTILE";
		case ssoSELECTEDITEMTILE: return "ssoSELECTEDITEMTILE";
		case ssoCURRENTITEMTEXT: return "ssoCURRENTITEMTEXT";
		case ssoCURRENTITEMNAME: return "ssoCURRENTITEMNAME";
		case ssoSELECTEDITEMNAME: return "ssoSELECTEDITEMNAME";
		case ssoCURRENTITEMCLASSTEXT: return "ssoCURRENTITEMCLASSTEXT";
		case ssoCURRENTITEMCLASSNAME: return "ssoCURRENTITEMCLASSNAME";
		case ssoSELECTEDITEMCLASSNAME: return "ssoSELECTEDITEMCLASSNAME";
		case ssoMAX: return "ssoMAX";
	}
	return "NIL_UNDEFINED_VAL";
}

int32_t onNewSubscreenObject()
{
    int32_t ret=-1;
    ssolist_dlg[0].dp2=get_zc_font(font_lfont);
    build_bisso_list();
    
    large_dialog(ssolist_dlg);
        
    ret=zc_popup_dialog(ssolist_dlg,2);
    
    if(ret!=0&&ret!=4)
    {
		doNewSubscreenObject(bisso[ssolist_dlg[2].d1].i);
    }
    
    return D_O_K;
}


void align_objects(bool *selection, int32_t align_type)
{
	auto& pg = subscr_edit_page();
	auto* curwidg = subscr_edit_widg();
	if(!curwidg) return;
	int32_t l=curwidg->getX()+curwidg->getXOffs();
	int32_t t=curwidg->getY();
	int32_t w=curwidg->getW();
	int32_t h=curwidg->getH();
	
	int32_t r=l+w-1;
	int32_t b=t+h-1;
	int32_t c=l+w/2;
	int32_t m=t+h/2;
	
	for(int32_t i=0; i<pg.contents.size(); ++i)
	{
		if(selection[i]&&i!=curr_subscreen_object)
		{
			SubscrWidget& widg = *pg.contents[i];
			int32_t tl=widg.getX()+widg.getXOffs();
			int32_t tt=widg.getY();
			int32_t tw=widg.getW();
			int32_t th=widg.getH();
			
			int32_t tr=tl+tw-1;
			int32_t tb=tt+th-1;
			int32_t tc=tl+tw/2;
			int32_t tm=tt+th/2;
			
			switch(align_type)
			{
			case ssoaBOTTOM:
				widg.y+=b-tb;
				break;
				
			case ssoaMIDDLE:
				widg.y+=m-tm;
				break;
				
			case ssoaTOP:
				widg.y+=t-tt;
				break;
				
			case ssoaRIGHT:
				widg.x+=r-tr;
				break;
				
			case ssoaCENTER:
				widg.x+=c-tc;
				break;
				
			case ssoaLEFT:
			default:
				widg.x+=l-tl;
				break;
			}
		}
	}
}

void grid_snap_objects(bool *selection, int32_t snap_type)
{
	auto& pg = subscr_edit_page();
	for(int32_t i=0; i < pg.contents.size(); ++i)
	{
		if(selection[i]||i==curr_subscreen_object)
		{
			SubscrWidget& widg = *pg.contents[i];
			int32_t tl=widg.getX()+widg.getXOffs();
			int32_t tt=widg.getY();
			int32_t tw=widg.getW();
			int32_t th=widg.getH();
			
			int32_t tr=tl+tw-1;
			int32_t tb=tt+th-1;
			int32_t tc=tl+tw/2;
			int32_t tm=tt+th/2;
			int32_t l1=(tl-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
			int32_t l2=l1+zinit.ss_grid_x;
			int32_t c1=(tc-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
			int32_t c2=c1+zinit.ss_grid_x;
			int32_t r1=(tr-zinit.ss_grid_xofs)/zinit.ss_grid_x*zinit.ss_grid_x+zinit.ss_grid_xofs;
			int32_t r2=r1+zinit.ss_grid_x;
			
			int32_t t1=(tt-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
			int32_t t2=t1+zinit.ss_grid_y;
			int32_t m1=(tm-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
			int32_t m2=m1+zinit.ss_grid_y;
			int32_t b1=(tb-zinit.ss_grid_yofs)/zinit.ss_grid_y*zinit.ss_grid_y+zinit.ss_grid_yofs;
			int32_t b2=b1+zinit.ss_grid_y;
			
			switch(snap_type)
			{
			case ssosBOTTOM:
				widg.y+=(abs(b1-tb)>abs(b2-tb))?(b2-tb):(b1-tb);
				break;
				
			case ssosMIDDLE:
				widg.y+=(abs(m1-tm)>abs(m2-tm))?(m2-tm):(m1-tm);
				break;
				
			case ssosTOP:
				widg.y+=(abs(t1-tt)>abs(t2-tt))?(t2-tt):(t1-tt);
				break;
				
			case ssosRIGHT:
				widg.x+=(abs(r1-tr)>abs(r2-tr))?(r2-tr):(r1-tr);
				break;
				
			case ssosCENTER:
				widg.x+=(abs(c1-tc)>abs(c2-tc))?(c2-tc):(c1-tc);
				break;
				
			case ssosLEFT:
			default:
				widg.x+=(abs(l1-tl)>abs(l2-tl))?(l2-tl):(l1-tl);
				break;
			}
		}
	}
}

typedef struct dist_obj
{
    int32_t index;
    int32_t l;
    int32_t t;
    int32_t w;
    int32_t h;
    int32_t r;
    int32_t b;
    int32_t c;
    int32_t m;
} dist_obj;

void distribute_objects(bool *, int32_t distribute_type)
{
	int32_t count=0;
	dist_obj temp_do[MAXSUBSCREENITEMS];
	
	auto& pg = subscr_edit_page();
	for(int32_t i=0; i < pg.contents.size(); ++i)
	{
		if(sso_selection[i]==true||i==curr_subscreen_object)
		{
			SubscrWidget& widg = *pg.contents[i];
			temp_do[count].index=i;
			temp_do[count].l=widg.getX()+widg.getXOffs();
			temp_do[count].t=widg.getY();
			temp_do[count].w=widg.getW();
			temp_do[count].h=widg.getH();
			
			temp_do[count].r=temp_do[count].l+temp_do[count].w-1;
			temp_do[count].b=temp_do[count].t+temp_do[count].h-1;
			temp_do[count].c=temp_do[count].l+temp_do[count].w/2;
			temp_do[count].m=temp_do[count].t+temp_do[count].h/2;
			++count;
		}
	}
	
	if(count<3)
	{
		return;
	}
	
	//sort all objects in order of position, then index (yeah, bubble sort; sue me)
	dist_obj tempdo2;
	
	for(int32_t j=0; j<count-1; j++)
	{
		for(int32_t k=0; k<count-1-j; k++)
		{
			switch(distribute_type)
			{
			case ssodBOTTOM:
				if(temp_do[k+1].b<temp_do[k].b||((temp_do[k+1].b==temp_do[k].b)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodMIDDLE:
				if(temp_do[k+1].m<temp_do[k].m||((temp_do[k+1].m==temp_do[k].m)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodTOP:
				if(temp_do[k+1].t<temp_do[k].t||((temp_do[k+1].t==temp_do[k].t)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodRIGHT:
				if(temp_do[k+1].r<temp_do[k].r||((temp_do[k+1].r==temp_do[k].r)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodCENTER:
				if(temp_do[k+1].c<temp_do[k].c||((temp_do[k+1].c==temp_do[k].c)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
				
			case ssodLEFT:
			default:
				if(temp_do[k+1].l<temp_do[k].l||((temp_do[k+1].l==temp_do[k].l)&&(temp_do[k+1].index<temp_do[k].index)))
				{
					tempdo2=temp_do[k];
					temp_do[k]=temp_do[k+1];
					temp_do[k+1]=tempdo2;
				}
				
				break;
			}
		}
	}
	
	int32_t ld=temp_do[count-1].l-temp_do[0].l;
	int32_t td=temp_do[count-1].t-temp_do[0].t;
	int32_t rd=temp_do[count-1].r-temp_do[0].r;
	int32_t bd=temp_do[count-1].b-temp_do[0].b;
	int32_t cd=temp_do[count-1].c-temp_do[0].c;
	int32_t md=temp_do[count-1].m-temp_do[0].m;
	
	for(int32_t i=1; i<count-1; ++i)
	{
		if(unsigned(temp_do[i].index) >= pg.contents.size())
			continue;
		SubscrWidget& widg = *pg.contents[temp_do[i].index];
		switch(distribute_type)
		{
		case ssodBOTTOM:
			widg.y+=bd*i/(count-1)-temp_do[i].b+temp_do[0].b;
			break;
			
		case ssodMIDDLE:
			widg.y+=md*i/(count-1)-temp_do[i].m+temp_do[0].m;
			break;
			
		case ssodTOP:
			widg.y+=td*i/(count-1)-temp_do[i].t+temp_do[0].t;
			break;
			
		case ssodRIGHT:
			widg.x+=rd*i/(count-1)-temp_do[i].r+temp_do[0].r;
			break;
			
		case ssodCENTER:
			widg.x+=cd*i/(count-1)-temp_do[i].c+temp_do[0].c;
			break;
			
		case ssodLEFT:
		default:
			widg.x+=ld*i/(count-1)-temp_do[i].l+temp_do[0].l;
			break;
		}
	}
}

int32_t onBringToFront()
{
    while(curr_subscreen_object<subscr_edit_page().contents.size()-1)
    {
        onBringForward();
    }
    
    return D_O_K;
}

int32_t onSendToBack()
{
    while(curr_subscreen_object>0)
    {
        onSendBackward();
    }
    
    return D_O_K;
}

int32_t onReverseArrangement()
{
	auto& pg = subscr_edit_page();
    int32_t i=0;
    int32_t j=pg.contents.size()-1;
    subscreen_object tempsso;
    
    sso_selection[curr_subscreen_object]=true;
    
    while(true)
    {
        while(i<pg.contents.size() && !sso_selection[i])
            i++;
            
        while(j>=0 && !sso_selection[j])
            j--;
            
        if(i>=j)
        {
            sso_selection[curr_subscreen_object]=false;
            return D_O_K;
        }
        
        if(curr_subscreen_object==i)
            curr_subscreen_object=j;
        else if(curr_subscreen_object==j)
            curr_subscreen_object=i;
		
		zc_swap(pg.contents[i],pg.contents[j]);
        
        i++;
        j--;
    }
}

int32_t onAlignLeft()
{
    align_objects(sso_selection, ssoaLEFT);
    return D_O_K;
}

int32_t onAlignCenter()
{
    align_objects(sso_selection, ssoaCENTER);
    return D_O_K;
}

int32_t onAlignRight()
{
    align_objects(sso_selection, ssoaRIGHT);
    return D_O_K;
}

int32_t onAlignTop()
{
    align_objects(sso_selection, ssoaTOP);
    return D_O_K;
}

int32_t onAlignMiddle()
{
    align_objects(sso_selection, ssoaMIDDLE);
    return D_O_K;
}

int32_t onAlignBottom()
{
    align_objects(sso_selection, ssoaBOTTOM);
    return D_O_K;
}

int32_t onDistributeLeft()
{
    distribute_objects(sso_selection, ssodLEFT);
    return D_O_K;
}

int32_t onDistributeCenter()
{
    distribute_objects(sso_selection, ssodCENTER);
    return D_O_K;
}

int32_t onDistributeRight()
{
    distribute_objects(sso_selection, ssodRIGHT);
    return D_O_K;
}

int32_t onDistributeTop()
{
    distribute_objects(sso_selection, ssodTOP);
    return D_O_K;
}

int32_t onDistributeMiddle()
{
    distribute_objects(sso_selection, ssodMIDDLE);
    return D_O_K;
}

int32_t onDistributeBottom()
{
    distribute_objects(sso_selection, ssodBOTTOM);
    return D_O_K;
}

int32_t onGridSnapLeft()
{
    grid_snap_objects(sso_selection, ssosLEFT);
    return D_O_K;
}

int32_t onGridSnapCenter()
{
    grid_snap_objects(sso_selection, ssosCENTER);
    return D_O_K;
}

int32_t onGridSnapRight()
{
    grid_snap_objects(sso_selection, ssosRIGHT);
    return D_O_K;
}

int32_t onGridSnapTop()
{
    grid_snap_objects(sso_selection, ssosTOP);
    return D_O_K;
}

int32_t onGridSnapMiddle()
{
    grid_snap_objects(sso_selection, ssosMIDDLE);
    return D_O_K;
}

int32_t onGridSnapBottom()
{
    grid_snap_objects(sso_selection, ssosBOTTOM);
    return D_O_K;
}

static int32_t onToggleInvis()
{
    bool show=!(zinit.ss_flags&ssflagSHOWINVIS);
    zinit.ss_flags&=~ssflagSHOWINVIS;
    zinit.ss_flags|=(show?ssflagSHOWINVIS:0);
    ss_view_menu[0].flags=zinit.ss_flags&ssflagSHOWINVIS?D_SELECTED:0;
    return D_O_K;
}

static int32_t onEditGrid()
{
    grid_dlg[0].dp2=get_zc_font(font_lfont);
    char xsize[11];
    char ysize[11];
    char xoffset[4];
    char yoffset[4];
    sprintf(xsize, "%d", zc_max(zinit.ss_grid_x,1));
    sprintf(ysize, "%d", zc_max(zinit.ss_grid_y,1));
    sprintf(xoffset, "%d", zinit.ss_grid_xofs);
    sprintf(yoffset, "%d", zinit.ss_grid_yofs);
    grid_dlg[4].dp=xsize;
    grid_dlg[6].dp=ysize;
    grid_dlg[8].dp=xoffset;
    grid_dlg[10].dp=yoffset;
    grid_dlg[12].d1=zinit.ss_grid_color;
    
    large_dialog(grid_dlg);
        
    int32_t ret = zc_popup_dialog(grid_dlg,2);
    
    if(ret==1)
    {
        zinit.ss_grid_x=zc_max(atoi(xsize),1);
        zinit.ss_grid_xofs=atoi(xoffset);
        zinit.ss_grid_y=zc_max(atoi(ysize),1);
        zinit.ss_grid_yofs=atoi(yoffset);
        zinit.ss_grid_color=grid_dlg[12].d1;
    }
    
    return D_O_K;
}

static int32_t onShowHideGrid()
{
    bool show=!(zinit.ss_flags&ssflagSHOWGRID);
    zinit.ss_flags&=~ssflagSHOWGRID;
    zinit.ss_flags|=(show?ssflagSHOWGRID:0);
    ss_view_menu[2].flags=zinit.ss_flags&ssflagSHOWGRID?D_SELECTED:0;
    return D_O_K;
}

int32_t onSelectionOptions()
{
    sel_options_dlg[0].dp2=get_zc_font(font_lfont);
    sel_options_dlg[6].d1=zinit.ss_bbox_1_color;
    sel_options_dlg[8].d1=zinit.ss_bbox_2_color;
    
    large_dialog(sel_options_dlg);
        
    int32_t ret = zc_popup_dialog(sel_options_dlg,2);
    
    if(ret==1)
    {
        zinit.ss_bbox_1_color=sel_options_dlg[6].d1;
        zinit.ss_bbox_2_color=sel_options_dlg[8].d1;
    }
    
    return D_O_K;
}


void update_up_dn_btns()
{
    if(curr_subscreen_object<1)
    {
        subscreen_dlg[10].flags|=D_DISABLED;
    }
    else
    {
        subscreen_dlg[10].flags&=~D_DISABLED;
    }
    
    if(curr_subscreen_object>=subscr_edit_page().contents.size()-1)
    {
        subscreen_dlg[9].flags|=D_DISABLED;
    }
    else
    {
        subscreen_dlg[9].flags&=~D_DISABLED;
    }
    
    subscreen_dlg[9].flags|=D_DIRTY;
    subscreen_dlg[10].flags|=D_DIRTY;
}

int32_t onSSCtrlPgUp()
{
    return onBringForward();
}

int32_t onSSCtrlPgDn()
{
    return onSendBackward();
}

int32_t onSendBackward()
{
	auto& contents = subscr_edit_page().contents;
	if(curr_subscreen_object >= 0 && curr_subscreen_object<subscr_edit_page().contents.size()-1)
	{
		zc_swap(contents[curr_subscreen_object],contents[curr_subscreen_object-1]);
		zc_swap(sso_selection[curr_subscreen_object],sso_selection[curr_subscreen_object-1]);
		++curr_subscreen_object;
		update_sso_name();
	}
	
	update_up_dn_btns();
	return D_O_K;
}

int32_t onSSPgDn()
{
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        return onSSCtrlPgDn();
    }
    else
    {
        --curr_subscreen_object;
        
        if(curr_subscreen_object<0)
        {
            curr_subscreen_object=subscr_edit_page().contents.size()-1;
        }
        
        update_sso_name();
        update_up_dn_btns();
    }
    
    return D_O_K;
}

// Send forward
int32_t onBringForward()
{
	auto& contents = subscr_edit_page().contents;
	if(curr_subscreen_object<subscr_edit_page().contents.size()-1)
	{
		zc_swap(contents[curr_subscreen_object],contents[curr_subscreen_object+1]);
		zc_swap(sso_selection[curr_subscreen_object],sso_selection[curr_subscreen_object+1]);
		++curr_subscreen_object;
		update_sso_name();
	}
	
	update_up_dn_btns();
	return D_O_K;
}


int32_t onSSPgUp()
{
    if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
    {
        return onSSCtrlPgUp();
    }
    else
    {
        if(!subscr_edit_page().contents.empty())
        {
            ++curr_subscreen_object;
            
            if(curr_subscreen_object>=subscr_edit_page().contents.size())
            {
                curr_subscreen_object=0;
            }
        }
        
        update_sso_name();
        update_up_dn_btns();
    }
    
    return D_O_K;
}

int32_t d_ssup_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        return onSSCtrlPgUp();
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssdn_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        return onSSCtrlPgDn();
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_sslt_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSPgDn();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}

int32_t d_ssrt_btn_proc(int32_t msg,DIALOG *d,int32_t c)
{
    switch(msg)
    {
    case MSG_CLICK:
    {
        jwin_button_proc(msg, d, c);
        onSSPgUp();
        return D_O_K;
    }
    break;
    }
    
    return jwin_button_proc(msg, d, c);
}



bool edit_subscreen()
{
	game = new gamedata();
	game->set_time(0);
	resetItems(game,&zinit,true);
	
	//so that these will show up on the subscreen -DD
	if(game->get_bombs() == 0)
		game->set_bombs(1);
		
	if(game->get_sbombs() == 0)
		game->set_sbombs(1);
		
	if(game->get_arrows() == 0)
		game->set_arrows(1);
		
	subscreen_dlg[0].dp2=get_zc_font(font_lfont);
	load_Sitems();
	curr_subscreen_object=0;
	int32_t i;
	
	if(subscr_edit.pages.empty())
		subscr_edit.pages.emplace_back();
	if(subscr_edit.pages.size() >= subscr_edit.curpage)
		subscr_edit.curpage = 0;
	if(subscr_edit.pages[subscr_edit.curpage].contents.empty())
	{
		curr_subscreen_object=-1;
	}
	
	onClearSelection();
	ss_view_menu[0].flags=zinit.ss_flags&ssflagSHOWINVIS?D_SELECTED:0;
	ss_view_menu[2].flags=zinit.ss_flags&ssflagSHOWGRID?D_SELECTED:0;
		
	subscreen_dlg[4].dp=(void *)&subscr_edit;
	subscreen_dlg[5].fg=jwin_pal[jcBOX];
	subscreen_dlg[5].bg=jwin_pal[jcBOX];
	str_oname=(char *)malloc(255);
	char namebuf[64] = {0};
	strncpy(namebuf,subscr_edit.name.c_str(),63);
	subscreen_dlg[6].dp=(void *)str_oname;
	subscreen_dlg[8].dp=(void *)namebuf;
	update_sso_name();
	subscreen_dlg[10].flags|=D_DISABLED;
	
	if(subscr_edit.sub_type==sstPASSIVE)
	{
		subscreen_dlg[21].flags|=D_DISABLED;
		subscreen_dlg[22].flags|=D_DISABLED;
		subscreen_dlg[23].flags|=D_DISABLED;
		subscreen_dlg[24].flags|=D_DISABLED;
	}
	else
	{
		subscreen_dlg[21].flags&=~D_DISABLED;
		subscreen_dlg[22].flags&=~D_DISABLED;
		subscreen_dlg[23].flags&=~D_DISABLED;
		subscreen_dlg[24].flags&=~D_DISABLED;
	}
	
	subscr_edit.cur_page().move_cursor(SEL_VERIFY_RIGHT);
	
	bool enlarge = subscreen_dlg[0].d1==0;
	
	if(enlarge)
	{
		large_dialog(subscreen_dlg,2);
		subscreen_dlg[4].y-=32;
		subscreen_dlg[3].y-=31;
		subscreen_dlg[3].x+=1;
		
		if(subscr_edit.sub_type == sstPASSIVE)
			subscreen_dlg[3].h=60*2-4;
		else if(subscr_edit.sub_type == sstACTIVE)
			subscreen_dlg[3].h=172*2-4;
			
		subscreen_dlg[4].h=subscreen_dlg[3].h-4;
	}
	
	int32_t ret = zc_popup_dialog(subscreen_dlg,2);
	
	if(ret==1)
	{
		subscr_edit.name = namebuf;
		saved=false;
		zinit.subscreen=ssdtMAX;
	}
	
	delete game;
	game=NULL;
	return ret == 1;
}

const char *allsubscrtype_str[30] =
{
    "Original (Top, Triforce)", "Original (Top, Map)",
    "New Subscreen (Top, Triforce)", "New Subscreen (Top, Map)",
    "Revision 2 (Top, Triforce)", "Revision 2 (Top, Map)",
    "BS Zelda Original (Top, Triforce)", "BS Zelda Original (Top, Map)",
    "BS Zelda Modified (Top, Triforce)", "BS Zelda Modified (Top, Map)",
    "BS Zelda Enhanced (Top, Triforce)", "BS Zelda Enhanced (Top, Map)",
    "BS Zelda Complete (Top, Triforce)", "BS Zelda Complete (Top, Map)",
    "Zelda 3 (Top)",
    "Original (Bottom, Magic)", "Original (Bottom, No Magic)",
    "New Subscreen (Bottom, Magic)", "New Subscreen (Bottom, No Magic)",
    "Revision 2 (Bottom, Magic)", "Revision 2 (Bottom, No Magic)",
    "BS Zelda Original (Bottom, Magic)", "BS Zelda Original (Bottom, No Magic)",
    "BS Zelda Modified (Bottom, Magic)", "BS Zelda Modified (Bottom, No Magic)",
    "BS Zelda Enhanced (Bottom, Magic)", "BS Zelda Enhanced (Bottom, No Magic)",
    "BS Zelda Complete (Bottom, Magic)", "BS Zelda Complete (Bottom, No Magic)",
    "Zelda 3 (Bottom)"
};

const char *activesubscrtype_str[16] =
{
    "Blank",
    "Original (Top, Triforce)", "Original (Top, Map)",
    "New Subscreen (Top, Triforce)", "New Subscreen (Top, Map)",
    "Revision 2 (Top, Triforce)", "Revision 2 (Top, Map)",
    "BS Zelda Original (Top, Triforce)", "BS Zelda Original (Top, Map)",
    "BS Zelda Modified (Top, Triforce)", "BS Zelda Modified (Top, Map)",
    "BS Zelda Enhanced (Top, Triforce)", "BS Zelda Enhanced (Top, Map)",
    "BS Zelda Complete (Top, Triforce)", "BS Zelda Complete (Top, Map)",
    "Zelda 3 (Top)"
};

const char *activelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return activesubscrtype_str[index];
}

const char *passivesubscrtype_str[16] =
{
    "Blank",
    "Original (Bottom, Magic)", "Original (Bottom, No Magic)",
    "New Subscreen (Bottom, Magic)", "New Subscreen (Bottom, No Magic)",
    "Revision 2 (Bottom, Magic)", "Revision 2 (Bottom, No Magic)",
    "BS Zelda Original (Bottom, Magic)", "BS Zelda Original (Bottom, No Magic)",
    "BS Zelda Modified (Bottom, Magic)", "BS Zelda Modified (Bottom, No Magic)",
    "BS Zelda Enhanced (Bottom, Magic)", "BS Zelda Enhanced (Bottom, No Magic)",
    "BS Zelda Complete (Bottom, Magic)", "BS Zelda Complete (Bottom, No Magic)",
    "Zelda 3 (Bottom)"
};

const char *passivelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = 16;
        return NULL;
    }
    
    return passivesubscrtype_str[index];
}

const char *activepassive_str[sstMAX] =
{
    "Active", "Passive"
};

const char *activepassivelist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = sstMAX;
        return NULL;
    }
    
    return activepassive_str[index];
}

static ListData passive_list(passivelist, &font);
static ListData active_list(activelist, &font);

int32_t sstype_drop_proc(int32_t msg,DIALOG *d,int32_t c)
{
    int32_t tempd1=d->d1;
    int32_t ret=jwin_droplist_proc(msg,d,c);
    
    if(tempd1!=d->d1)
    {
        (d+1)->dp=(d->d1)?(void*)&passive_list:(void*)&active_list;
        object_message(d+1,MSG_START,0);
        (d+1)->flags|=D_DIRTY;
    }
    
    return ret;
}

static ListData activepassive_list(activepassivelist, &font);

static DIALOG sstemplatelist_dlg[] =
{
    // (dialog proc)         (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,          0,    0,   265,  87,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "New Subscreen", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_text_proc,         4,   28,     8,   8,   0,                  0,                0,       0,          0,             0, (void *) "Type:", NULL, NULL },
    { jwin_text_proc,         4,   46,     8,   8,   0,                  0,                0,       0,          0,             0, (void *) "Template:", NULL, NULL },
    { sstype_drop_proc,      33,   24,    72,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,         0,     0,             0, (void *) &activepassive_list, NULL, NULL },
    { jwin_droplist_proc,    50,   42,   211,  16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,         0,     0,             0, (void *) &active_list, NULL, NULL },
    { jwin_button_proc,      61,   62,    61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     142,   62,    61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

bool show_new_ss=true;

const char *subscreenlist(int32_t index, int32_t *list_size)
{
    if(index<0)
    {
        *list_size = new_subscreen.size()+(show_new_ss?1:0);
        return NULL;
    }
    if(show_new_ss && index == new_subscreen.size())
		return "<New>";
	if(unsigned(index) < new_subscreen.size())
		return new_subscreen[index].name.c_str();
	return "";
}

const char *subscreenlist_either(int32_t index, int32_t *list_size, byte type)
{
    if(index<0)
    {
        int32_t i=0, j=0;
        
		while(j < new_subscreen.size())
		{
			if(new_subscreen[j].sub_type==type)
				++i;
			
			++j;
		}
        
        *list_size = i;
        return NULL;
    }
    
    int32_t i=-1, j=0;
    
	while(j < new_subscreen.size() && i!=index)
	{
		if(new_subscreen[j].sub_type==type)
			++i;
		
		++j;
	}
    
    return new_subscreen[j-1].name.c_str();
}

const char *subscreenlist_a(int32_t index, int32_t *list_size)
{
    return subscreenlist_either(index,list_size,sstACTIVE);
}

const char *subscreenlist_b(int32_t index, int32_t *list_size)
{
    return subscreenlist_either(index,list_size,sstPASSIVE);
}

static ListData subscreen_list(subscreenlist, &font);

DIALOG sslist_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,     0,   0,   234,  148,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Select Subscreen", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { jwin_abclist_proc,    12,   24,   211,  95,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0, (void *) &subscreen_list, NULL, NULL },
    { jwin_button_proc,     12,   123,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Edit", NULL, NULL },
    { jwin_button_proc,     85,  123,  61,   21,   vc(14),  vc(1),  KEY_DEL,     D_EXIT,     0,             0, (void *) "Delete", NULL, NULL },
    { jwin_button_proc,     158,  123,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Done", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t onEditSubscreens()
{
    int32_t ret=-1;
    sslist_dlg[0].dp2=get_zc_font(font_lfont);
    sstemplatelist_dlg[0].dp2=get_zc_font(font_lfont);
    
    large_dialog(sslist_dlg);
        
    while(ret!=0&&ret!=5)
    {
        ret=zc_popup_dialog(sslist_dlg,2);
        
		auto ind = sslist_dlg[2].d1;
        if(ret==4)
        {
            int32_t confirm = jwin_alert("Confirm Delete", "You are about to delete the selected subscreen!", "Are you sure?", NULL, "OK", "Cancel", KEY_ENTER, KEY_ESC, get_zc_font(font_lfont));
            
            if(confirm==1)
            {
                delete_subscreen(ind);
                saved=false;
            }
        }
        else if(ret==2 || ret ==3)
        {
			if(ind < new_subscreen.size())
				subscr_edit = new_subscreen[ind];
			else subscr_edit.clear();
            if(subscr_edit.sub_type==sstPASSIVE)
            {
                subscreen_dlg[3].h=116;
                subscreen_dlg[4].h=subscreen_dlg[3].h-4;
            }
			else
            {
				subscr_edit.sub_type = sstACTIVE;
                subscreen_dlg[3].h=340;
                subscreen_dlg[4].h=subscreen_dlg[3].h-4;
            }
            
            bool edit_it=true;
            
            if(subscr_edit.pages.empty())
            {
                large_dialog(sstemplatelist_dlg);
                    
                ret=zc_popup_dialog(sstemplatelist_dlg,4);
                
                if(ret==6)
                {
                    if(sstemplatelist_dlg[5].d1<15)
                    {
                        if(sstemplatelist_dlg[5].d1 != 0)
                        {
                            subscreen_object *tempsub;
                            
                            if(sstemplatelist_dlg[4].d1==0)
                            {
                                tempsub = default_subscreen_active[(sstemplatelist_dlg[5].d1-1)/2][(sstemplatelist_dlg[5].d1-1)&1];
                            }
                            else
                            {
                                tempsub = default_subscreen_passive[(sstemplatelist_dlg[5].d1-1)/2][(sstemplatelist_dlg[5].d1-1)&1];
                            }
							subscr_edit.load_old(tempsub);
                        }
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            subscr_edit.sub_type=sstACTIVE;
                            subscr_edit.name = activesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=172*2;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                        else
                        {
                            subscr_edit.sub_type=sstPASSIVE;
                            subscr_edit.name = passivesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=120;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                    }
                    else //Z3
                    {
                        subscreen_object *tempsub;
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            tempsub = z3_active_a;
                        }
                        else
                        {
                            tempsub = z3_passive_a;
                        }
						subscr_edit.load_old(tempsub);
                        
                        if(sstemplatelist_dlg[4].d1==0)
                        {
                            subscr_edit.sub_type=sstACTIVE;
                            subscr_edit.name = activesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=344;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                            
                        }
                        else
                        {
                            subscr_edit.sub_type=sstPASSIVE;
                            subscr_edit.name = passivesubscrtype_str[sstemplatelist_dlg[5].d1];
                            subscreen_dlg[3].h=120;
                            subscreen_dlg[4].h=subscreen_dlg[3].h-4;
                        }
                    }
                }
                else
                {
                    edit_it=false;
                }
            }
            if(subscr_edit.pages.empty())
				subscr_edit.pages.emplace_back();
            
            if(edit_it && edit_subscreen())
			{
				if(ind < new_subscreen.size())
					new_subscreen[ind] = subscr_edit;
				else new_subscreen.push_back(subscr_edit);
			}
        }
    }
    
    position_mouse_z(0);
    return D_O_K;
}

void update_sso_name()
{
	SubscrWidget* w = subscr_edit_widg();
	if(w)
		sprintf(str_oname, "%3d:  %s", curr_subscreen_object, sso_name(w->getType()));
	else
		sprintf(str_oname, "No object selected");
	
	subscreen_dlg[5].flags|=D_DIRTY;
	subscreen_dlg[6].flags|=D_DIRTY;
}

void center_zq_subscreen_dialogs()
{
    jwin_center_dialog(grid_dlg);
    jwin_center_dialog(sel_options_dlg);
    jwin_center_dialog(sslist_dlg);
    jwin_center_dialog(ssolist_dlg);
    jwin_center_dialog(sstemplatelist_dlg);
    jwin_center_dialog(subscreen_dlg);
}

void delete_subscreen(int32_t subscreenidx)
{
	if(subscreenidx >= new_subscreen.size())
		return;
	
	int32_t i = 0;
	int32_t active_ind = 0, passive_ind = 0;
	for(auto it = new_subscreen.begin(); it != new_subscreen.end(); ++it)
	{
		if(i++ == subscreenidx)
		{
			if(it->sub_type == sstACTIVE) passive_ind = -1;
			else active_ind = -1;
			i = -1;
			new_subscreen.erase(it);
			break;
		}
		if(it->sub_type == sstACTIVE) ++active_ind;
		else ++passive_ind;
	}
	if(i != -1) active_ind = passive_ind = -1; //no deletion
	
	//fix dmaps
	int32_t dmap_count=count_dmaps();
	
	for(int32_t i=0; i<dmap_count; i++)
	{
		//decrement
		if(active_ind > -1 && DMaps[i].active_subscreen > active_ind)
			DMaps[i].active_subscreen--;
			
		if(passive_ind > -1 && DMaps[i].passive_subscreen > passive_ind)
			DMaps[i].passive_subscreen--;
	}
}

void copySSOProperties(SubscrWidget const* src, SubscrWidget* dest)
{
    if(src->getType()!=dest->getType() || src==dest)
        return;
	dest->copy_prop(src);
}
