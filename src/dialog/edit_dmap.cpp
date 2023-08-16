#include "edit_dmap.h"
#include <gui/builder.h>
#include "jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include <fmt/format.h>
#include <base/qrs.h>

static size_t editdmap_tab = 0;
void call_editdmap_dialog(int32_t slot)
{
	EditDMapDialog(slot).show();
}
void call_editdmap_dialog(size_t forceTab, int32_t slot)
{
	editdmap_tab = forceTab;
	call_editdmap_dialog(slot);
}

EditDMapDialog::EditDMapDialog(int32_t slot) :
	thedmap(&DMaps[slot]), local_dmap(DMaps[slot]), dmapslot(slot),
	list_maps(GUI::ListData::numbers(false, 1, map_count)),
	list_types(GUI::ZCListData::dmaptypes()),
	list_lpals(GUI::ZCListData::lpals()),
	list_strings(GUI::ZCListData::strings()),
	list_activesub(GUI::ZCListData::activesubscreens()),
	list_passivesub(GUI::ZCListData::passivesubscreens())
{}

// trims spaces at the end of a string
char* strip_whitespace(char* title, int bufsz)
{
	for (int q = bufsz-1; q >0; --q)
	{
		if (title[q] == ' ')
			title[q] = 0;
		else
			break;
	}
	return title;
}

bool sm_dmap(int dmaptype)
{
	switch (dmaptype & dmfTYPE)
	{
		case dmOVERW:
			return false;
		default:
			return true;
	}
}

void EditDMapDialog::refreshDMapStrings()
{
	//string_switch->switchTo(get_qr(qr_OLD_DMAP_INTRO_STRINGS) ? 0 : 1);
}

std::shared_ptr<GUI::Widget> EditDMapDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	char titlebuf[256];
	sprintf(titlebuf, "Edit DMap (%d)", dmapslot);
	window = Window(
		title = titlebuf,
		onClose = message::CANCEL,
		Column(
			Row(hAlign = 0.0,
				Label(text = "Name:", hAlign = 0.0, colSpan = 2),
				TextField(
					fitParent = true,
					type = GUI::TextField::type::TEXT,
					maxLength = 20,
					text = local_dmap.name,
					onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
					{
						std::string str;
						str.assign(text);
						strncpy(local_dmap.name, str.c_str(), 20);
						local_dmap.name[20] = 0;
					}
				)
			),
			TabPanel(
				ptr = &editdmap_tab,
				TabRef(name = "Mechanics", Column(
					Rows<6>(
						Label(text = "Map:"),
						DropDownList(data = list_maps,
							fitParent = true,
							selectedValue = local_dmap.map + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.map = val - 1;
								dmap_mmap->setCurMap(val);
							}),
						Label(text = "Type:"),
						DropDownList(data = list_types,
							fitParent = true,
							selectedValue = local_dmap.type & dmfTYPE,
							onSelectFunc = [&](int32_t val)
							{
								bool wassmol = sm_dmap(local_dmap.type);
								local_dmap.type = (local_dmap.type & ~dmfTYPE) | (val & dmfTYPE);
								bool smol = sm_dmap(local_dmap.type);
								if (smol != wassmol)
								{
									dmap_mmap->setSmallDMap(smol);
									dmap_grid->setSmallDMap(smol);
									dmap_slider->setDisabled(!smol);
									if (smol)
									{
										local_dmap.xoff = dmap_slider->getOffset();
										dmap_mmap->setOffset(local_dmap.xoff);
									}
									else
									{
										local_dmap.xoff = 0;
										dmap_mmap->setOffset(0);
									}
								}
							}),
						Label(text = "Level:"),
						TextField(
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = 0, high = 511, val = local_dmap.level,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								local_dmap.level = val;
							})
					),
					Rows<2>(
						dmap_mmap = DMapMinimap(
							rightMargin = 6_px,
							curMap = local_dmap.map + 1,
							smallDMap = sm_dmap(local_dmap.type),
							offset = local_dmap.xoff
						),
						dmap_grid = DMapMapGrid(
							leftMargin = 6_px,
							mapGridPtr = &local_dmap.grid[0],
							continueScreen = local_dmap.cont, compassScreen = local_dmap.compass,
							smallDMap = sm_dmap(local_dmap.type),
							onUpdate = [&](byte* byteptr, byte compassScreen, byte continueScreen)
							{
								if (compassScreen != local_dmap.compass)
								{
									local_dmap.compass = compassScreen;
									compass_field->setVal(compassScreen);
								}
								if (continueScreen != local_dmap.cont)
								{
									local_dmap.cont = continueScreen;
									continue_field->setVal(continueScreen);
								}
							})
					),
					Rows<4>(
						dmap_slider = Slider(
							colSpan = 2, fitParent = true, rightMargin = 64_px,
							disabled = !sm_dmap(local_dmap.type),
							offset = local_dmap.xoff,
							minOffset = -7, maxOffset = 15,
							onValChangedFunc = [&](int32_t offset)
							{
								local_dmap.xoff = offset;
								dmap_mmap->setOffset(offset);
							}),
						Label(text = "Compass: 0x", hAlign = 1.0, rightPadding = 0_px),
						compass_field = TextField(
							hAlign = 0.0,
							fitParent = true, minwidth = 2_em,
							type = GUI::TextField::type::INT_HEX,
							low = 0x00, high = 0x7F, val = local_dmap.compass,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								dmap_grid->setCompassScreen(val);
								local_dmap.compass = val;
							}),
						Checkbox(checked = local_dmap.type & dmfCONTINUE,
							colSpan = 2, text = "Continue here",
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_dmap.type, dmfCONTINUE, state);
							}),
						Label(text = "Continue: 0x", hAlign = 1.0, rightPadding = 0_px),
						continue_field = TextField(
							hAlign = 0.0,
							fitParent = true, minwidth = 2_em,
							type = GUI::TextField::type::INT_HEX,
							low = 0, high = 127, val = local_dmap.cont,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								dmap_grid->setContinueScreen(val);
								local_dmap.cont = val;
							}),
						DummyWidget(colSpan = 2),
						Label(text = "Mirror DMap:", hAlign = 1.0),
						TextField(
							hAlign = 0.0,
							fitParent = true, minwidth = 3_em,
							type = GUI::TextField::type::INT_DECIMAL,
							low = -1, high = 511, val = local_dmap.mirrorDMap,
							onValChangedFunc = [&](GUI::TextField::type, std::string_view, int32_t val)
							{
								local_dmap.mirrorDMap = val;
							})
					)
				)),
				TabRef(name = "Appearance", Column(
					Rows<2>(
						Label(text = "Level Palette:"),
						DropDownList(data = list_lpals,
							fitParent = true,
							selectedValue = local_dmap.color + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.color = val - 1;
							})
					),
					string_switch = Switcher(
						Rows<2>(
							Label(text = "DMap Title"),
							Label(text = "DMap Intro"),
							TextField(
								width = 86_px, height = 22_px,
								maxLength = 20, type = GUI::TextField::type::TEXT_LEGACY,
								text = local_dmap.title,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									zprint2("Title: %s\n", text);
									std::string str;
									str.assign(text);
									strncpy(local_dmap.title, str.c_str(), 20);
									local_dmap.title[20] = 0;
								}),
							TextField(
								width = 198_px, height = 30_px,
								maxLength = 72, type = GUI::TextField::type::TEXT_LEGACY,
								text = local_dmap.intro,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									std::string str;
									str.assign(text);
									strncpy(local_dmap.intro, str.c_str(), 72);
									local_dmap.intro[72] = 0;
								})
						),
						Rows<2>(
							Label(text = "DMap Title:"),
							TextField(
								fitParent = true, 
								type = GUI::TextField::type::TEXT,
								text = local_dmap.title,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									//TODO
								}),
							Label(text = "DMap Intro:"),
							DropDownList(data = list_strings,
								fitParent = true,
								width = 256_px,
								selectedValue = 1,
								onSelectFunc = [&](int32_t val)
								{
									//TODO
								})
						)
					),
					Rows<2>(
						framed = true, frameText = "Subscreens",
						Label(text = "Active:"),
						DropDownList(data = list_activesub,
							fitParent = true,
							selectedValue = local_dmap.active_subscreen + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.active_subscreen = val - 1;
							}),
						Label(text = "Passive:"),
						DropDownList(data = list_passivesub,
							fitParent = true,
							selectedValue = local_dmap.passive_subscreen + 1,
							onSelectFunc = [&](int32_t val)
							{
								local_dmap.passive_subscreen = val - 1;
							})
					)
				)),
				TabRef(name = "Music", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Maps", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Flags", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Disable", Column(
					Row(
						Label(text = "Placeholder")
					)
				)),
				TabRef(name = "Scripts", Column(
					Row(
						Label(text = "Placeholder")
					)
				))
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	refreshDMapStrings();
	return window;
}

bool EditDMapDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
	case message::REFR_INFO:
		break;
	case message::OK:
		*thedmap = local_dmap;
		saved = false;
		[[fallthrough]];
	case message::CANCEL:
	default:
		return true;
	}
	return false;
}
