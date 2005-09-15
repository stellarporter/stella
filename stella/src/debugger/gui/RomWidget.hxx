//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2005 by Bradford W. Mott and the Stella team
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: RomWidget.hxx,v 1.4 2005-09-15 19:43:36 stephena Exp $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef ROM_WIDGET_HXX
#define ROM_WIDGET_HXX

class GuiObject;
class RomListWidget;
class StringList;

#include <map>

#include "Array.hxx"
#include "Widget.hxx"
#include "Command.hxx"

typedef map<int,int> AddrToLine;


class RomWidget : public Widget, public CommandSender
{
  public:
    RomWidget(GuiObject* boss, const GUI::Font& font, int x, int y);
    virtual ~RomWidget();

    void invalidate() { myListIsDirty = true; }

    void handleCommand(CommandSender* sender, int cmd, int data, int id);
    void loadConfig();

  private:
    void initialUpdate();
    void incrementalUpdate(int line, int rows);

    void setBreak(int data);
    void setPC(int data);
    void patchROM(int data, const string& bytes);
    void saveROM();

  private:
    RomListWidget* myRomList;

    /** List of addresses indexed by line number */
    IntArray myAddrList;

    /** List of line numbers indexed by address */
    AddrToLine myLineList;

    /** Indicates whether a given line is valid or not;
        Invalid lines need to be disassembled again */
    BoolArray myLineValid;

    bool myListIsDirty;
    bool mySourceAvailable;
    int  myCurrentBank;
};

#endif
