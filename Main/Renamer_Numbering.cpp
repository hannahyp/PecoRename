/*
 * Copyrights (c):
 *     2000 - 2008 , Werner Freytag.
 *     2009, Haiku
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *              Werner Freytag <freytag@gmx.de>
 */

#include <strstream.h>

#include <Alert.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Beep.h>

#include "constants.h"
#include "functions.h"

#include "FileListItem.h"
#include "LiveTextControl.h"
#include "Renamer_Numbering.h"

Renamer_Numbering::Renamer_Numbering() : Renamer() {

	char *Modi[] = { "1, 2, 3, ...", "01, 02, 03, ...", "001, 002, 003, ...", "0001, 0002, 0003, ..."};

	fName 		= REN_NUMBERING;

	BRect	frame = Bounds().InsetByCopy(4.0, 4.0);

	BPopUpMenu	*myMenu = new BPopUpMenu(STR_PLEASE_SELECT);
	for ( uint32 i = 0; i < sizeof(Modi) / sizeof(char *); i++ )
		myMenu->AddItem(new BMenuItem(Modi[i], new BMessage(MSG_RENAME_SETTINGS)));

	myMenu->ItemAt(0)->SetMarked(true);

	fFormat = new BMenuField( BRect(frame.left, frame.top - 2, frame.left + be_plain_font->StringWidth(REN_SET_FORMAT) + 130, frame.top + be_plain_font->Size() + 8), NULL, REN_SET_FORMAT, myMenu, new BMessage(MSG_RENAME_SETTINGS));
	fFormat->SetDivider( be_plain_font->StringWidth(REN_SET_FORMAT) + 5);
	AddChild( fFormat );

	frame.left += be_plain_font->StringWidth(REN_SET_FORMAT) + 140;

	fStartWith = new LiveTextControl( BRect(frame.left, frame.top, frame.left + be_plain_font->StringWidth(REN_SET_STARTWITH) + 50, 0), NULL, REN_SET_STARTWITH, "0", new BMessage(MSG_RENAME_SETTINGS));
	fStartWith->SetDivider( be_plain_font->StringWidth(REN_SET_STARTWITH) + 5);
	fStartWith->SetAlignment(B_ALIGN_LEFT, B_ALIGN_CENTER);
	((BTextView *)fStartWith->ChildAt(0))->SetMaxBytes(4);
	AddChild(fStartWith);

	frame = Bounds().InsetByCopy(4.0, 4.0);
	frame.top += be_plain_font->Size() + 14;
	
	fTextBefore = new LiveTextControl( BRect(frame.left, frame.top, frame.left + be_plain_font->StringWidth(REN_SET_TEXTBEFORE) + 130, 0), NULL, REN_SET_TEXTBEFORE, NULL, new BMessage(MSG_RENAME_SETTINGS));
	fTextBefore->SetDivider( be_plain_font->StringWidth(REN_SET_TEXTBEFORE) + 5);
	AddChild(fTextBefore);

	frame.left += be_plain_font->StringWidth(REN_SET_TEXTBEFORE) + 140;

	fTextBehind = new LiveTextControl( BRect(frame.left, frame.top, frame.left + be_plain_font->StringWidth(REN_SET_TEXTBEHIND) + 130, 0), NULL, REN_SET_TEXTBEHIND, NULL, new BMessage(MSG_RENAME_SETTINGS));
	fTextBehind->SetDivider( be_plain_font->StringWidth(REN_SET_TEXTBEHIND) + 5);
	AddChild(fTextBehind);
};

void Renamer_Numbering::RenameList(BList *FileList) {

	Renamer :: RenameList(FileList);

	int			Startzahl;
	strstream	iStream, oStream;
	
	if (strlen(fStartWith->Text())==0) {
		Startzahl = 0;
		oStream << Startzahl; oStream.put(0);
		fStartWith->SetText(oStream.str());
	} else {
		iStream << fStartWith->Text(); iStream >> Startzahl;
		if ((Startzahl<0)) {
			Startzahl = 0;
			oStream << Startzahl; oStream.put(0);
//			fStartWith->SetText(oStream.str());
		}
	}
	
	int MinAnzStellen = fFormat->Menu()->IndexOf(fFormat->Menu()->FindMarked()) + 1;

	BString TextBefore = fTextBefore->Text();
	BString TextBehind = fTextBehind->Text();
	
	FileListItem	*ListItem;
	BString			Nummer;
		
	for (int64 i = 0; i < fNumberOfItems; i++ ) {
		
		ListItem = (FileListItem *)FileList->ItemAt(i);

		strstream 	oStream;
		BString		NummerString;
		
		oStream << i + Startzahl; oStream.put(0);
		NummerString = oStream.str();

		if (NummerString.Length() < MinAnzStellen)
			NummerString.Prepend(BString("000").Truncate(MinAnzStellen - NummerString.Length()));
		
		BString		ResultString = TextBefore;
		ResultString.Append(NummerString);
		ResultString.Append(TextBehind);
		ListItem->SetNewName( ResultString );
	}
};

void Renamer_Numbering::DetachedFromWindow() {
	BMessage	msg;

	msg.AddString("start_with", fStartWith->Text());
	BMenu *menu = fFormat->Menu();
	msg.AddInt8("positions", menu->IndexOf(menu->FindMarked()));
	msg.AddString("text_before", fTextBefore->Text());
	msg.AddString("text_behind", fTextBehind->Text());
	
	UpdatePreferences("ren_numbering", msg);
}

void Renamer_Numbering::AttachedToWindow() {
	BMessage	msg;
	ReadPreferences("ren_numbering", msg);
	
	BString string;
	int8 number;

	if (msg.FindString("start_with", &string)==B_OK)
		fStartWith->SetText(string.String());

	if (msg.FindInt8("positions", &number)==B_OK) {
		BMenu *menu = fFormat->Menu();
		for (int i=0; i<4; ++i) menu->ItemAt(i)->SetMarked(i==number);
	}
	if (msg.FindString("text_before", &string)==B_OK)
		fTextBefore->SetText(string.String());
	if (msg.FindString("text_behind", &string)==B_OK)
		fTextBehind->SetText(string.String());

}
