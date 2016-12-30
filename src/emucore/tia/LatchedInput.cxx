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
// Copyright (c) 1995-2017 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "LatchedInput.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LatchedInput::LatchedInput()
{
  reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LatchedInput::reset()
{
  myModeLatched = false;
  myLatchedValue = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void LatchedInput::vblank(uInt8 value)
{
  if (value & 0x40)
    myModeLatched = true;
  else {
    myModeLatched = false;
    myLatchedValue = 0x80;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 LatchedInput::inpt(bool pinState)
{
  uInt8 value = pinState ? 0 : 0x80;

  if (myModeLatched) {
    myLatchedValue &= value;
    value = myLatchedValue;
  }

  return value;
}
