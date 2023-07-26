#include "base/zdefs.h"
#include "new_subscr.h"
#include "subscr.h"
#include "base/misctypes.h"
#include "base/fonts.h"
#include "base/zsys.h"
#include "base/dmap.h"
#include "base/qrs.h"
#include "base/mapscr.h"
#include "tiles.h"
#include "items.h"

extern gamedata* game; //!TODO ZDEFSCLEAN move to gamedata.h
extern zinitdata zinit; //!TODO ZDEFSCLEAN move to zinit.h
int32_t get_dlevel();
int32_t get_currdmap();
int32_t get_homescr();
bool has_item(int32_t item_type, int32_t item);

//!TODO subscr.h/subscr.cpp trim
extern item *sel_a, *sel_b;
void subscreenitem(BITMAP *dest, int32_t x, int32_t y, int32_t itemtype);
int32_t subscreen_color(int32_t c1, int32_t c2);
void draw_textbox(BITMAP *dest, int32_t x, int32_t y, int32_t w, int32_t h, FONT *tempfont, char *thetext, bool wword, int32_t tabsize, int32_t alignment, int32_t textstyle, int32_t color, int32_t shadowcolor, int32_t backcolor);
int32_t Bweapon(int32_t pos);
void lifegauge(BITMAP *dest,int32_t x,int32_t y, int32_t container, int32_t notlast_tile, int32_t notlast_cset, bool notlast_mod, int32_t last_tile, int32_t last_cset, bool last_mod,
			   int32_t cap_tile, int32_t cap_cset, bool cap_mod, int32_t aftercap_tile, int32_t aftercap_cset, bool aftercap_mod, int32_t frames, int32_t speed, int32_t delay, bool unique_last);
void magicgauge(BITMAP *dest,int32_t x,int32_t y, int32_t container, int32_t notlast_tile, int32_t notlast_cset, bool notlast_mod, int32_t last_tile, int32_t last_cset, bool last_mod,
				int32_t cap_tile, int32_t cap_cset, bool cap_mod, int32_t aftercap_tile, int32_t aftercap_cset, bool aftercap_mod, int32_t frames, int32_t speed, int32_t delay, bool unique_last, int32_t show);
int32_t get_subscreenitem_id(int32_t itemtype, bool forceItem);


int shadow_x(int shadow)
{
	switch(shadow)
	{
		case sstsSHADOWU:
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsSHADOWEDU:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return -1;
	}
	return 0;
}
int shadow_y(int shadow)
{
	switch(shadow)
	{
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return -1;
	}
	return 0;
}
int shadow_w(int shadow)
{
	switch(shadow)
	{
		case sstsSHADOW:
		case sstsSHADOWU:
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsSHADOWED:
		case sstsSHADOWEDU:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return 1;
	}
	return 0;
}
int shadow_h(int shadow)
{
	switch(shadow)
	{
		case sstsSHADOW:
		case sstsSHADOWU:
		case sstsOUTLINE8:
		case sstsOUTLINEPLUS:
		case sstsOUTLINEX:
		case sstsSHADOWED:
		case sstsSHADOWEDU:
		case sstsOUTLINED8:
		case sstsOUTLINEDPLUS:
		case sstsOUTLINEDX:
			return 1;
	}
	return 0;
}

int32_t SubscrColorInfo::get_color() const
{
	int32_t ret;
	
	switch(type)
	{
		case ssctSYSTEM:
			ret=(color==-1)?color:vc(color);
			break;
			
		case ssctMISC:
			switch(color)
			{
				case ssctTEXT:
					ret=QMisc.colors.text;
					break;
					
				case ssctCAPTION:
					ret=QMisc.colors.caption;
					break;
					
				case ssctOVERWBG:
					ret=QMisc.colors.overw_bg;
					break;
					
				case ssctDNGNBG:
					ret=QMisc.colors.dngn_bg;
					break;
					
				case ssctDNGNFG:
					ret=QMisc.colors.dngn_fg;
					break;
					
				case ssctCAVEFG:
					ret=QMisc.colors.cave_fg;
					break;
					
				case ssctBSDK:
					ret=QMisc.colors.bs_dk;
					break;
					
				case ssctBSGOAL:
					ret=QMisc.colors.bs_goal;
					break;
					
				case ssctCOMPASSLT:
					ret=QMisc.colors.compass_lt;
					break;
					
				case ssctCOMPASSDK:
					ret=QMisc.colors.compass_dk;
					break;
					
				case ssctSUBSCRBG:
					ret=QMisc.colors.subscr_bg;
					break;
					
				case ssctSUBSCRSHADOW:
					ret=QMisc.colors.subscr_shadow;
					break;
					
				case ssctTRIFRAMECOLOR:
					ret=QMisc.colors.triframe_color;
					break;
					
				case ssctBMAPBG:
					ret=QMisc.colors.bmap_bg;
					break;
					
				case ssctBMAPFG:
					ret=QMisc.colors.bmap_fg;
					break;
					
				case ssctHERODOT:
					ret=QMisc.colors.hero_dot;
					break;
					
				default:
					ret=(zc_oldrand()*1000)%256;
					break;
			}
			
			break;
			
		default:
			ret=(type<<4)+color;
	}
	
	return ret;
}

int32_t SubscrColorInfo::get_cset() const
{
	int32_t ret=type;
	
	switch(type)
	{
		case ssctMISC:
			switch(color)
			{
				case sscsTRIFORCECSET:
					ret=QMisc.colors.triforce_cset;
					break;
					
				case sscsTRIFRAMECSET:
					ret=QMisc.colors.triframe_cset;
					break;
					
				case sscsOVERWORLDMAPCSET:
					ret=QMisc.colors.overworld_map_cset;
					break;
					
				case sscsDUNGEONMAPCSET:
					ret=QMisc.colors.dungeon_map_cset;
					break;
					
				case sscsBLUEFRAMECSET:
					ret=QMisc.colors.blueframe_cset;
					break;
					
				case sscsHCPIECESCSET:
					ret=QMisc.colors.HCpieces_cset;
					break;
					
				case sscsSSVINECSET:
					ret=wpnsbuf[iwSubscreenVine].csets&15;
					break;
					
				default:
					ret=(zc_oldrand()*1000)%256;
					break;
			}
			break;
	}
	
	return ret;
}

void SubscrColorInfo::load_old(subscreen_object const& old, int indx)
{
	if(indx < 1 || indx > 3) return;
	switch(indx)
	{
		case 1:
			type = old.colortype1;
			color = old.color1;
			break;
		case 2:
			type = old.colortype2;
			color = old.color2;
			break;
		case 3:
			type = old.colortype3;
			color = old.color3;
			break;
	}
}

SubscrWidget::SubscrWidget(subscreen_object const& old) : SubscrWidget()
{
	load_old(old);
}
bool SubscrWidget::load_old(subscreen_object const& old)
{
	type = old.type;
	posflags = old.pos;
	x = old.x;
	y = old.y;
	w = old.w;
	h = old.h;
	return true;
}
int16_t SubscrWidget::getX() const
{
	return x;
}
int16_t SubscrWidget::getY() const
{
	return y;
}
word SubscrWidget::getW() const
{
	return w;
}
word SubscrWidget::getH() const
{
	return h;
}
int16_t SubscrWidget::getXOffs() const
{
	return 0;
}
int16_t SubscrWidget::getYOffs() const
{
	return 0;
}
byte SubscrWidget::getType() const
{
	return type;
}
int32_t SubscrWidget::getItemVal() const
{
	return -1;
}
void SubscrWidget::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	
}
bool SubscrWidget::visible(byte pos, bool showtime) const
{
	return posflags&pos;
}

SW_2x2Frame::SW_2x2Frame(subscreen_object const& old) : SW_2x2Frame()
{
	load_old(old);
}
bool SW_2x2Frame::load_old(subscreen_object const& old)
{
	if(old.type != sso2X2FRAME)
		return false;
	SubscrWidget::load_old(old);
	tile = old.d1;
	cs.load_old(old,1);
	return true;
}
word SW_2x2Frame::getW() const
{
	return w*8;
}
word SW_2x2Frame::getH() const
{
	return h*8;
}
byte SW_2x2Frame::getType() const
{
	return sso2X2FRAME;
}
void SW_2x2Frame::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	frame2x2(dest, x+xofs, y+yofs, tile, cs.get_cset(), w, h, 0,
		flags&SUBSCR_2X2FR_OVERLAY, flags&SUBSCR_2X2FR_TRANSP);
}

SW_Text::SW_Text(subscreen_object const& old) : SW_Text()
{
	load_old(old);
}
bool SW_Text::load_old(subscreen_object const& old)
{
	if(old.type != ssoTEXT)
		return false;
	SubscrWidget::load_old(old);
	if(old.dp1) text = (char*)old.dp1;
	else text.clear();
	fontid = to_real_font(old.d1);
	align = old.d2;
	shadtype = old.d3;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Text::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Text::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Text::getW() const
{
	return text_length(get_zc_font(fontid), text.c_str());
}
word SW_Text::getH() const
{
	return text_height(get_zc_font(fontid));
}
int16_t SW_Text::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Text::getType() const
{
	return ssoTEXT;
}
void SW_Text::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	textout_styled_aligned_ex(dest,tempfont,text.c_str(),getX()+xofs,getY()+yofs,
		shadtype,align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}

SW_Line::SW_Line(subscreen_object const& old) : SW_Line()
{
	load_old(old);
}
bool SW_Line::load_old(subscreen_object const& old)
{
	if(old.type != ssoLINE)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_LINE_TRANSP,old.d4);
	c_line.load_old(old,1);
	return true;
}
byte SW_Line::getType() const
{
	return ssoLINE;
}
void SW_Line::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(flags&SUBSCR_LINE_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	line(dest, x+xofs, y+yofs, x+xofs+w-1, y+yofs+h-1, c_line.get_color());
	
	if(flags&SUBSCR_LINE_TRANSP)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

SW_Rect::SW_Rect(subscreen_object const& old) : SW_Rect()
{
	load_old(old);
}
bool SW_Rect::load_old(subscreen_object const& old)
{
	if(old.type != ssoRECT)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_RECT_TRANSP,old.d2);
	SETFLAG(flags,SUBSCR_RECT_FILLED,old.d1);
	c_fill.load_old(old,2);
	c_outline.load_old(old,1);
	return true;
}
byte SW_Rect::getType() const
{
	return ssoRECT;
}
void SW_Rect::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(flags&SUBSCR_RECT_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	auto x2 = x+xofs, y2 = y+yofs;
	if(flags&SUBSCR_RECT_FILLED)
		rectfill(dest, x2, y2, x2+w-1, y2+h-1, c_fill.get_color());
	
	rect(dest, x2, y2, x2+w-1, y2+h-1, c_outline.get_color());
	
	if(flags&SUBSCR_RECT_TRANSP)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

SW_Time::SW_Time(subscreen_object const& old) : SW_Time()
{
	load_old(old);
}
bool SW_Time::load_old(subscreen_object const& old)
{
	if(old.type != ssoBSTIME && old.type != ssoTIME
		&& old.type != ssoSSTIME)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	align = old.d2;
	shadtype = old.d3;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Time::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Time::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Time::getW() const
{
	char *ts;
	auto tm = game ? game->get_time() : 0;
	switch(type)
	{
		case ssoBSTIME:
			ts = time_str_short2(tm);
			break;
		case ssoTIME:
		case ssoSSTIME:
			ts = time_str_med(tm);
			break;
	}
	return text_length(get_zc_font(fontid), ts) + shadow_w(shadtype);
}
word SW_Time::getH() const
{
	return text_height(get_zc_font(fontid)) + shadow_h(shadtype);
}
int16_t SW_Time::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Time::getType() const
{
	return type; //ssoBSTIME,ssoTIME,ssoSSTIME
}
void SW_Time::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	char *ts;
	auto tm = game ? game->get_time() : 0;
	switch(type)
	{
		case ssoBSTIME:
			ts = time_str_short2(tm);
			break;
		case ssoTIME:
		case ssoSSTIME:
			ts = time_str_med(tm);
			break;
	}
	FONT* tempfont = get_zc_font(fontid);
	textout_styled_aligned_ex(dest,tempfont,ts,getX()+xofs,getY()+yofs,
		shadtype,align,c_text.get_color(),c_shadow.get_color(),c_bg.get_color());
}
bool SW_Time::visible(byte pos, bool showtime) const
{
	return showtime && SubscrWidget::visible(pos,showtime);
}

SW_MagicMeter::SW_MagicMeter(subscreen_object const& old) : SW_MagicMeter()
{
	load_old(old);
}
bool SW_MagicMeter::load_old(subscreen_object const& old)
{
	if(old.type != ssoMAGICMETER)
		return false;
	SubscrWidget::load_old(old);
	return true;
}
int16_t SW_MagicMeter::getX() const
{
	return x-10;
}
word SW_MagicMeter::getW() const
{
	return 82;
}
word SW_MagicMeter::getH() const
{
	return 8;
}
byte SW_MagicMeter::getType() const
{
	return ssoMAGICMETER;
}
void SW_MagicMeter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	magicmeter(dest, getX()+xofs, getY()+yofs);
}

SW_LifeMeter::SW_LifeMeter(subscreen_object const& old) : SW_LifeMeter()
{
	load_old(old);
}
bool SW_LifeMeter::load_old(subscreen_object const& old)
{
	if(old.type != ssoLIFEMETER)
		return false;
	SubscrWidget::load_old(old);
	rows = old.d3 ? 3 : 2;
	SETFLAG(flags,SUBSCR_LIFEMET_BOT,old.d2);
	return true;
}
int16_t SW_LifeMeter::getY() const
{
	if(flags&SUBSCR_LIFEMET_BOT)
		return y;
	return (4-rows)*8;
}
word SW_LifeMeter::getW() const
{
	return 64;
}
word SW_LifeMeter::getH() const
{
	return 8*rows;
}
byte SW_LifeMeter::getType() const
{
	return ssoLIFEMETER;
}
void SW_LifeMeter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	lifemeter(dest, getX()+xofs, getY()+yofs, 1, flags&SUBSCR_LIFEMET_BOT);
}

SW_ButtonItem::SW_ButtonItem(subscreen_object const& old) : SW_ButtonItem()
{
	load_old(old);
}
bool SW_ButtonItem::load_old(subscreen_object const& old)
{
	if(old.type != ssoBUTTONITEM)
		return false;
	SubscrWidget::load_old(old);
	btn = old.d1;
	SETFLAG(flags,SUBSCR_BTNITM_TRANSP,old.d2);
	return true;
}
word SW_ButtonItem::getW() const
{
	return 16;
}
word SW_ButtonItem::getH() const
{
	return 16;
}
byte SW_ButtonItem::getType() const
{
	return ssoBUTTONITEM;
}
void SW_ButtonItem::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	if(flags&SUBSCR_BTNITM_TRANSP)
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	
	buttonitem(dest, btn, x, y);
	
	if(flags&SUBSCR_BTNITM_TRANSP)
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

SW_Counter::SW_Counter(subscreen_object const& old) : SW_Counter()
{
	load_old(old);
}
bool SW_Counter::load_old(subscreen_object const& old)
{
	if(old.type != ssoCOUNTER)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	align = old.d2;
	shadtype = old.d3;
	ctrs[0] = old.d7;
	ctrs[1] = old.d8;
	ctrs[2] = old.d9;
	SETFLAG(flags,SUBSCR_COUNTER_SHOW0,old.d6&0b01);
	SETFLAG(flags,SUBSCR_COUNTER_ONLYSEL,old.d6&0b10);
	digits = old.d4;
	infitm = old.d10;
	infchar = old.d5;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Counter::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Counter::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Counter::getW() const
{
	return text_length(get_zc_font(fontid), "0")*4 + shadow_w(shadtype);
}
word SW_Counter::getH() const
{
	return text_height(get_zc_font(fontid)) + shadow_h(shadtype);
}
int16_t SW_Counter::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Counter::getType() const
{
	return ssoCOUNTER;
}
void SW_Counter::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	counter(dest, getX()+xofs,getY()+yofs, tempfont, c_text.get_color(),
		c_shadow.get_color(), c_bg.get_color(),align,shadtype,digits,infchar,
		flags&SUBSCR_COUNTER_SHOW0, ctrs[0], ctrs[1], ctrs[2], infitm,
		flags&SUBSCR_COUNTER_ONLYSEL);
}

SW_Counters::SW_Counters(subscreen_object const& old) : SW_Counters()
{
	load_old(old);
}
bool SW_Counters::load_old(subscreen_object const& old)
{
	if(old.type != ssoCOUNTERS)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	SETFLAG(flags,SUBSCR_COUNTERS_USEX,old.d2);
	shadtype = old.d3;
	digits = old.d4;
	infitm = old.d10;
	infchar = old.d5;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
int16_t SW_Counters::getX() const
{
	return x+shadow_x(shadtype);
}
int16_t SW_Counters::getY() const
{
	return y+shadow_y(shadtype);
}
word SW_Counters::getW() const
{
	return 32 + shadow_w(shadtype);
}
word SW_Counters::getH() const
{
	return 32 + shadow_h(shadtype);
}
byte SW_Counters::getType() const
{
	return ssoCOUNTERS;
}
void SW_Counters::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	defaultcounters(dest, getX()+xofs, getY()+yofs, tempfont, c_text.get_color(),
		c_shadow.get_color(), c_bg.get_color(),flags&SUBSCR_COUNTERS_USEX,shadtype,
		digits,infchar);
}

SW_MMapTitle::SW_MMapTitle(subscreen_object const& old) : SW_MMapTitle()
{
	load_old(old);
}
bool SW_MMapTitle::load_old(subscreen_object const& old)
{
	if(old.type != ssoMINIMAPTITLE)
		return false;
	SubscrWidget::load_old(old);
	fontid = to_real_font(old.d1);
	SETFLAG(flags,SUBSCR_MMAPTIT_REQMAP,old.d4);
	shadtype = old.d3;
	c_text.load_old(old,1);
	c_shadow.load_old(old,2);
	c_bg.load_old(old,3);
	return true;
}
word SW_MMapTitle::getW() const
{
	return 80;
}
word SW_MMapTitle::getH() const
{
	return 16;
}
int16_t SW_MMapTitle::getXOffs() const
{
	switch(align)
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_MMapTitle::getType() const
{
	return ssoMINIMAPTITLE;
}
void SW_MMapTitle::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(fontid);
	if(!(flags&SUBSCR_MMAPTIT_REQMAP) || has_item(itype_map, get_dlevel()))
		minimaptitle(dest, getX()+xofs, getY()+yofs, tempfont, c_text.get_color(),
			c_shadow.get_color(),c_bg.get_color(), align, shadtype);
}

SW_MMap::SW_MMap(subscreen_object const& old) : SW_MMap()
{
	load_old(old);
}
bool SW_MMap::load_old(subscreen_object const& old)
{
	if(old.type != ssoMINIMAP)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_MMAP_SHOWMAP,old.d1);
	SETFLAG(flags,SUBSCR_MMAP_SHOWPLR,old.d2);
	SETFLAG(flags,SUBSCR_MMAP_SHOWCMP,old.d3);
	c_plr.load_old(old,1);
	c_cmp_blink.load_old(old,2);
	c_cmp_off.load_old(old,3);
	return true;
}
word SW_MMap::getW() const
{
	return 80;
}
word SW_MMap::getH() const
{
	return 48;
}
byte SW_MMap::getType() const
{
	return ssoMINIMAP;
}
void SW_MMap::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	bool showplr = (flags&SUBSCR_MMAP_SHOWPLR) && !(TheMaps[(DMaps[get_currdmap()].map*MAPSCRS)+get_homescr()].flags7&fNOHEROMARK);
	bool showcmp = (flags&SUBSCR_MMAP_SHOWCMP) && !(DMaps[get_currdmap()].flags&dmfNOCOMPASS);
	drawdmap(dest, getX()+xofs, getY()+yofs, flags&SUBSCR_MMAP_SHOWMAP, showplr,
		showcmp, c_plr.get_color(), c_cmp_blink.get_color(), c_cmp_off.get_color());
}

SW_LMap::SW_LMap(subscreen_object const& old) : SW_LMap()
{
	load_old(old);
}
bool SW_LMap::load_old(subscreen_object const& old)
{
	if(old.type != ssoLARGEMAP)
		return false;
	SubscrWidget::load_old(old);
	SETFLAG(flags,SUBSCR_LMAP_SHOWMAP,old.d1);
	SETFLAG(flags,SUBSCR_LMAP_SHOWROOM,old.d2);
	SETFLAG(flags,SUBSCR_LMAP_SHOWPLR,old.d3);
	SETFLAG(flags,SUBSCR_LMAP_LARGE,old.d10);
	c_room.load_old(old,1);
	c_plr.load_old(old,2);
	return true;
}
word SW_LMap::getW() const
{
	return 16*((flags&SUBSCR_LMAP_LARGE)?9:7);
}
word SW_LMap::getH() const
{
	return 80;
}
byte SW_LMap::getType() const
{
	return ssoLARGEMAP;
}
void SW_LMap::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	putBmap(dest, getX()+xofs, getY()+yofs, flags&SUBSCR_LMAP_SHOWMAP,
		flags&SUBSCR_LMAP_SHOWROOM, flags&SUBSCR_LMAP_SHOWPLR, c_room.get_color(),
		c_plr.get_color(), flags&SUBSCR_LMAP_LARGE);
}

SW_Clear::SW_Clear(subscreen_object const& old) : SW_Clear()
{
	load_old(old);
}
bool SW_Clear::load_old(subscreen_object const& old)
{
	if(old.type != ssoCLEAR)
		return false;
	SubscrWidget::load_old(old);
	c_bg.load_old(old,1);
	return true;
}
word SW_Clear::getW() const
{
	return 5;
}
word SW_Clear::getH() const
{
	return 5;
}
byte SW_Clear::getType() const
{
	return ssoCLEAR;
}
void SW_Clear::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	clear_to_color(dest,c_bg.get_color());
}

SW_CurrentItem::SW_CurrentItem(subscreen_object const& old) : SW_CurrentItem()
{
	load_old(old);
}
bool SW_CurrentItem::load_old(subscreen_object const& old)
{
	if(old.type != ssoCURRENTITEM)
		return false;
	SubscrWidget::load_old(old);
	iclass = old.d1;
	iid = old.d8;
	pos = old.d3;
	pos_up = old.d4;
	pos_down = old.d5;
	pos_left = old.d6;
	pos_right = old.d7;
	SETFLAG(flags,SUBSCRFLAG_SELECTABLE,pos>=0);
	SETFLAG(flags,SUBSCR_CURITM_INVIS,!(old.d2&0x1));
	SETFLAG(flags,SUBSCR_CURITM_NONEQP,old.d2&0x2);
	return true;
}
word SW_CurrentItem::getW() const
{
	return 16;
}
word SW_CurrentItem::getH() const
{
	return 16;
}
byte SW_CurrentItem::getType() const
{
	return ssoCURRENTITEM;
}
int32_t SW_CurrentItem::getItemVal() const
{
	return iid>0 ? ((iid-1) | 0x8000) : iclass;
}
void SW_CurrentItem::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	#ifdef IS_PLAYER
	if(flags&SUBSCR_CURITM_INVIS)
		return;
	#else
	if((flags&SUBSCR_CURITM_INVIS) && !(zinit.ss_flags&ssflagSHOWINVIS))
		return;
	#endif
	subscreenitem(dest, getX()+xofs,getY()+yofs, getItemVal());
}

SW_TriFrame::SW_TriFrame(subscreen_object const& old) : SW_TriFrame()
{
	load_old(old);
}
bool SW_TriFrame::load_old(subscreen_object const& old)
{
	if(old.type != ssoTRIFRAME)
		return false;
	SubscrWidget::load_old(old);
	frame_tile = old.d1;
	frame_cset = old.d2;
	piece_tile = old.d3;
	piece_cset = old.d4;
	SETFLAG(flags,SUBSCR_TRIFR_SHOWFR,old.d5);
	SETFLAG(flags,SUBSCR_TRIFR_SHOWPC,old.d6);
	SETFLAG(flags,SUBSCR_TRIFR_LGPC,old.d7);
	c_outline.load_old(old,1);
	c_number.load_old(old,2);
	return true;
}
word SW_TriFrame::getW() const
{
	return 16*((flags&SUBSCR_TRIFR_LGPC)?7:6);
}
word SW_TriFrame::getH() const
{
	return 16*((flags&SUBSCR_TRIFR_LGPC)?7:3);
}
byte SW_TriFrame::getType() const
{
	return ssoTRIFRAME;
}
void SW_TriFrame::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	puttriframe(dest, getX()+xofs,getY()+yofs, c_outline.get_color(), c_number.get_color(),
		frame_tile, frame_cset, piece_tile, piece_cset, flags&SUBSCR_TRIFR_SHOWFR,
		flags&SUBSCR_TRIFR_SHOWPC, flags&SUBSCR_TRIFR_LGPC);
}

SW_McGuffin::SW_McGuffin(subscreen_object const& old) : SW_McGuffin()
{
	load_old(old);
}
bool SW_McGuffin::load_old(subscreen_object const& old)
{
	if(old.type != ssoMCGUFFIN)
		return false;
	SubscrWidget::load_old(old);
	tile = old.d1;
	cset = old.d2;
	number = old.d5;
	SETFLAG(flags,SUBSCR_MCGUF_OVERLAY,old.d3);
	SETFLAG(flags,SUBSCR_MCGUF_TRANSP,old.d4);
	cs.load_old(old,1);
	return true;
}
word SW_McGuffin::getW() const
{
	return 16;
}
word SW_McGuffin::getH() const
{
	return 16;
}
byte SW_McGuffin::getType() const
{
	return ssoMCGUFFIN;
}
void SW_McGuffin::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	puttriforce(dest,getX()+xofs,getY()+yofs,tile,cs.get_cset(),w,h,
		cset,flags&SUBSCR_MCGUF_OVERLAY,flags&SUBSCR_MCGUF_TRANSP,number);
}

SW_TileBlock::SW_TileBlock(subscreen_object const& old) : SW_TileBlock()
{
	load_old(old);
}
bool SW_TileBlock::load_old(subscreen_object const& old)
{
	if(old.type != ssoTILEBLOCK)
		return false;
	SubscrWidget::load_old(old);
	tile = old.d1;
	flip = old.d2;
	SETFLAG(flags,SUBSCR_TILEBL_OVERLAY,old.d3);
	SETFLAG(flags,SUBSCR_TILEBL_TRANSP,old.d4);
	cs.load_old(old,1);
	return true;
}
word SW_TileBlock::getW() const
{
	return w * 16;
}
word SW_TileBlock::getH() const
{
	return h * 16;
}
byte SW_TileBlock::getType() const
{
	return ssoTILEBLOCK;
}
void SW_TileBlock::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	draw_block_flip(dest,getX()+xofs,getY()+yofs,tile,cs.get_cset(),
		w,h,flip,flags&SUBSCR_TILEBL_OVERLAY,flags&SUBSCR_TILEBL_TRANSP);
}

SW_MiniTile::SW_MiniTile(subscreen_object const& old) : SW_MiniTile()
{
	load_old(old);
}
bool SW_MiniTile::load_old(subscreen_object const& old)
{
	if(old.type != ssoMINITILE)
		return false;
	SubscrWidget::load_old(old);
	if(old.d1 == -1) tile = -1;
	else tile = old.d1>>2;
	special_tile = old.d2;
	crn = old.d3;
	flip = old.d4;
	SETFLAG(flags,SUBSCR_MINITL_OVERLAY,old.d5);
	SETFLAG(flags,SUBSCR_MINITL_TRANSP,old.d6);
	cs.load_old(old,1);
	return true;
}
word SW_MiniTile::getW() const
{
	return 8;
}
word SW_MiniTile::getH() const
{
	return 8;
}
byte SW_MiniTile::getType() const
{
	return ssoMINITILE;
}
int32_t SW_MiniTile::get_tile() const
{
	if(tile == -1)
	{
		switch(special_tile)
		{
			case ssmstSSVINETILE:
				return wpnsbuf[iwSubscreenVine].tile;
			case ssmstMAGICMETER:
				return wpnsbuf[iwMMeter].tile;
			default:
				return (zc_oldrand()*100000)%32767;
		}
	}
	else return tile;
}
void SW_MiniTile::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	auto t = (get_tile()<<2)+crn;
	auto tx = getX()+xofs, ty = getY()+yofs;
	byte cset = cs.get_cset();
	if(flags&SUBSCR_MINITL_OVERLAY)
	{
		if(flags&SUBSCR_MINITL_TRANSP)
			overtiletranslucent8(dest,t,tx,ty,cset,flip,128);
		else
			overtile8(dest,t,tx,ty,cset,flip);
	}
	else
	{
		if(flags&SUBSCR_MINITL_TRANSP)
			puttiletranslucent8(dest,t,tx,ty,cset,flip,128);
		else
			oldputtile8(dest,t,tx,ty,cset,flip);
	}
}


SW_Temp::SW_Temp(subscreen_object const& old) : SW_Temp()
{
	load_old(old);
}
bool SW_Temp::load_old(subscreen_object const& _old)
{
	type = ssoTEMPOLD;
	old = _old;
	if(old.dp1)
	{
		old.dp1 = new char[strlen((char*)_old.dp1)+1];
		strcpy((char*)old.dp1,(char*)_old.dp1);
	}
	return true;
}
int16_t SW_Temp::getX() const
{
	return sso_x(&old);
}
int16_t SW_Temp::getY() const
{
	return sso_y(&old);
}
word SW_Temp::getW() const
{
	return sso_w(&old);
}
word SW_Temp::getH() const
{
	return sso_h(&old);
}
int16_t SW_Temp::getXOffs() const
{
	switch(get_alignment(&old))
	{
		case sstaCENTER:
			return -getW()/2;
		case sstaRIGHT:
			return -getW();
	}
	return 0;
}
byte SW_Temp::getType() const
{
	return ssoTEMPOLD;
}
void SW_Temp::draw(BITMAP* dest, int32_t xofs, int32_t yofs, SubscrPage& page) const
{
	FONT* tempfont = get_zc_font(old.d1);
	switch(getType())
	{
		case ssoTEXTBOX:
		{
			draw_textbox(dest, x, y, old.w, old.h, tempfont, (char *)old.dp1, old.d4!=0, old.d5, old.d2, old.d3, subscreen_color(old.colortype1, old.color1), subscreen_color(old.colortype2, old.color2), subscreen_color(old.colortype3, old.color3));
		}
		break;
		
		case ssoSELECTEDITEMNAME:
		{
			int32_t itemid=Bweapon(page.cursor_pos);
			
			// If it's a combined bow and arrow, the item ID will have 0xF000 added.
			if(itemid>=0xF000)
				itemid-=0xF000;
			
			// 0 can mean either the item with index 0 is selected or there's no
			// valid item to select, so be sure Hero has whatever it would be.
			if(!game->get_item(itemid))
				break;
				
			itemdata const& itm = itemsbuf[itemid];
			char itemname[256]="";
			strncpy(itemname, itm.get_name().c_str(), 255);
			
			draw_textbox(dest, x, y, old.w, old.h, tempfont, itemname, old.d4!=0, old.d5, old.d2, old.d3, subscreen_color(old.colortype1, old.color1), subscreen_color(old.colortype2, old.color2), subscreen_color(old.colortype3, old.color3));
		}
		break;
		
		case ssoSELECTOR1:
		case ssoSELECTOR2:
		{
			int32_t p=-1;
			
			SubscrWidget* selitm = nullptr;
			for(size_t j=0; j < page.contents.size(); ++j)
			{
				SubscrWidget& w = page.contents[j];
				if(w.getType()==ssoCURRENTITEM)
				{
					if(w.pos==page.cursor_pos)
					{
						p=j;
						selitm = &w;
						break;
					}
				}
			}
			
			bool big_sel=old.d5 != 0;
			item *tempsel=(old.type==ssoSELECTOR1)?sel_a:sel_b;
			int32_t temptile=tempsel->tile;
			tempsel->drawstyle=0;
			
			if(old.d4)
				tempsel->drawstyle=1;
			int32_t itemtype = selitm ? selitm->getItemVal() : -1;
			itemdata const& tmpitm = itemsbuf[get_subscreenitem_id(itemtype, true)];
			bool oldsel = get_qr(qr_SUBSCR_OLD_SELECTOR);
			if(!oldsel) big_sel = false;
			int32_t sw = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : (tempsel->extend > 2 ? tempsel->hit_width : 16),
				sh = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : (tempsel->extend > 2 ? tempsel->hit_height : 16),
				dw = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_WIDTH) ? tmpitm.hxsz : 16),
				dh = oldsel ? (tempsel->extend > 2 ? tempsel->txsz*16 : 16) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_HEIGHT) ? tmpitm.hysz : 16);
			int32_t sxofs = oldsel ? 0 : (tempsel->extend > 2 ? tempsel->hxofs : 0),
				syofs = oldsel ? 0 : (tempsel->extend > 2 ? tempsel->hyofs : 0),
				dxofs = oldsel ? (tempsel->extend > 2 ? (int)tempsel->xofs : 0) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_X_OFFSET) ? tmpitm.hxofs : 0) + (tempsel->extend > 2 ? (int)tempsel->xofs : 0),
				dyofs = oldsel ? (tempsel->extend > 2 ? (int)tempsel->yofs : 0) : ((tmpitm.overrideFLAGS & itemdataOVERRIDE_HIT_Y_OFFSET) ? tmpitm.hyofs : 0) + (tempsel->extend > 2 ? (int)tempsel->yofs : 0);
			BITMAP* tmpbmp = create_bitmap_ex(8,sw,sh);
			for(int32_t j=0; j<4; ++j)
			{
				clear_bitmap(tmpbmp);
				if(selitm)
				{
					tempsel->x=0;
					tempsel->y=0;
					int32_t tmpx = selitm->x+xofs+(big_sel?(j%2?8:-8):0);
					int32_t tmpy = selitm->y+yofs+(big_sel?(j>1?8:-8):0);
					tempsel->tile+=(zc_max(itemsbuf[tempsel->id].frames,1)*j);
					
					if(temptile)
					{
						tempsel->drawzcboss(tmpbmp);
						tempsel->tile=temptile;
					}
					masked_stretch_blit(tmpbmp, dest, vbound(sxofs, 0, sw), vbound(syofs, 0, sh), sw-vbound(sxofs, 0, sw), sh-vbound(syofs, 0, sh), tmpx+dxofs, tmpy+dyofs, dw, dh);
					
					if(!big_sel)
						break;
				}
			}
			destroy_bitmap(tmpbmp);
		}
		break;
		
		case ssoMAGICGAUGE:
		{
			magicgauge(dest,x,y, old.d1, old.d2, old.colortype1, ((old.d10&1)?1:0), old.d3, old.color1, ((old.d10&2)?1:0),
					   old.d4, old.colortype2, ((old.d10&4)?1:0), old.d5, old.color2, ((old.d10&8)?1:0), old.d6, old.d7, old.d8, ((old.d10&16)?1:0),
					   old.d9);
		}
		break;
		
		case ssoLIFEGAUGE:
		{
			lifegauge(dest,x,y, old.d1, old.d2, old.colortype1, ((old.d10&1)?1:0), old.d3, old.color1, ((old.d10&2)?1:0),
					  old.d4, old.colortype2, ((old.d10&4)?1:0), old.d5, old.color2, ((old.d10&8)?1:0), old.d6, old.d7, old.d8, ((old.d10&16)?1:0));
		}
		break;
	}
}



bool new_widget_type(int ty)
{
	switch(ty)
	{
		//These have been upgraded
		case sso2X2FRAME:
		case ssoTEXT:
		case ssoLINE:
		case ssoRECT:
		case ssoBSTIME:
		case ssoTIME:
		case ssoSSTIME:
		case ssoMAGICMETER:
		case ssoLIFEMETER:
		case ssoBUTTONITEM:
		case ssoCOUNTER:
		case ssoCOUNTERS:
		case ssoMINIMAPTITLE:
		case ssoMINIMAP:
		case ssoLARGEMAP:
		case ssoCLEAR:
		case ssoCURRENTITEM:
		case ssoTRIFRAME:
		case ssoMCGUFFIN:
		case ssoTILEBLOCK:
		case ssoMINITILE:
			return true;
		//These ones are just empty
		case ssoITEM:
		case ssoICON:
		case ssoNULL:
		case ssoNONE:
			return true;
	}
	return false;
}
SubscrWidget SubscrWidget::fromOld(subscreen_object const& old)
{
	switch(old.type)
	{
		case sso2X2FRAME:
			return SW_2x2Frame(old);
		case ssoTEXT:
			return SW_Text(old);
		case ssoLINE:
			return SW_Line(old);
		case ssoRECT:
			return SW_Rect(old);
		case ssoBSTIME:
		case ssoTIME:
		case ssoSSTIME:
			return SW_Time(old);
		case ssoMAGICMETER:
			return SW_MagicMeter(old);
		case ssoLIFEMETER:
			return SW_LifeMeter(old);
		case ssoBUTTONITEM:
			return SW_ButtonItem(old);
		case ssoCOUNTER:
			return SW_Counter(old);
		case ssoCOUNTERS:
			return SW_Counters(old);
		case ssoMINIMAPTITLE:
			return SW_MMapTitle(old);
		case ssoMINIMAP:
			return SW_MMap(old);
		case ssoLARGEMAP:
			return SW_LMap(old);
		case ssoCLEAR:
			return SW_Clear(old);
		case ssoCURRENTITEM:
			return SW_CurrentItem(old);
		case ssoTRIFRAME:
			return SW_TriFrame(old);
		case ssoMCGUFFIN:
			return SW_McGuffin(old);
		case ssoTILEBLOCK:
			return SW_TileBlock(old);
		case ssoMINITILE:
			return SW_MiniTile(old);
		case ssoSELECTOR1:
		case ssoSELECTOR2:
		case ssoMAGICGAUGE:
		case ssoLIFEGAUGE:
		case ssoTEXTBOX:
		case ssoCURRENTITEMTILE:
		case ssoSELECTEDITEMTILE:
		case ssoCURRENTITEMTEXT:
		case ssoCURRENTITEMNAME:
		case ssoSELECTEDITEMNAME:
		case ssoCURRENTITEMCLASSTEXT:
		case ssoCURRENTITEMCLASSNAME:
		case ssoSELECTEDITEMCLASSNAME:
			return SW_Temp(old); //!TODO SUBSCR
		case ssoITEM:
		{
			SubscrWidget ret(old);
			ret.w = 16;
			ret.h = 16;
			return ret;
		}
		case ssoICON:
		{
			SubscrWidget ret(old);
			ret.w = 8;
			ret.h = 8;
			return ret;
		}
		case ssoNULL:
		case ssoNONE:
			break; //Nothingness
	}
	return SubscrWidget(old);
}

void SubscrPage::draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime)
{
	for(SubscrWidget& widg : contents)
	{
		if(widg.visible(pos,showtime))
			widg.draw(dest,xofs,yofs,*this);
	}
}
void ZCSubscreen::draw(BITMAP* dest, int32_t xofs, int32_t yofs, byte pos, bool showtime)
{
	if(pages.empty()) return;
	size_t page = curpage;
	if(page >= pages.size()) page = 0;
	//!TODO SUBSCR handle animations between multiple pages?
	pages[page].draw(dest,xofs,yofs,pos,showtime);
}
