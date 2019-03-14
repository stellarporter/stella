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
// Copyright (c) 1995-2019 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "SimpleResampler.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SimpleResampler::SimpleResampler(
  Resampler::Format formatFrom,
  Resampler::Format formatTo,
  Resampler::NextFragmentCallback nextFragmentCallback,
  StaggeredLogger::Logger logger)
  : Resampler(formatFrom, formatTo, nextFragmentCallback, logger),
    myCurrentFragment(nullptr),
    myTimeIndex(0),
    myFragmentIndex(0),
    myIsUnderrun(true)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SimpleResampler::fillFragment(float* fragment, uInt32 length)
{
  if (myIsUnderrun) {
    Int16* nextFragment = myNextFragmentCallback();

    if (nextFragment) {
      myCurrentFragment = nextFragment;
      myFragmentIndex = 0;
      myIsUnderrun = false;
    }
  }

  if (!myCurrentFragment) {
    memset(fragment, 0, sizeof(float) * length);
    return;
  }

  const uInt32 outputSamples = myFormatTo.stereo ? (length >> 1) : length;

  // For the following math, remember that myTimeIndex = time * myFormatFrom.sampleRate * myFormatTo.sampleRate
  for (uInt32 i = 0; i < outputSamples; ++i) {
    if (myFormatFrom.stereo) {
      float sampleL = static_cast<float>(myCurrentFragment[2*myFragmentIndex]) / static_cast<float>(0x7fff);
      float sampleR = static_cast<float>(myCurrentFragment[2*myFragmentIndex + 1]) / static_cast<float>(0x7fff);

      if (myFormatTo.stereo) {
        fragment[2*i] = sampleL;
        fragment[2*i + 1] = sampleR;
      }
      else
        fragment[i] = (sampleL + sampleR) / 2.f;
    } else {
      float sample = static_cast<float>(myCurrentFragment[myFragmentIndex] / static_cast<float>(0x7fff));

      if (myFormatTo.stereo)
        fragment[2*i] = fragment[2*i + 1] = sample;
      else
        fragment[i] = sample;
    }

    // time += 1 / myFormatTo.sampleRate
    myTimeIndex += myFormatFrom.sampleRate;

    // time >= 1 / myFormatFrom.sampleRate
    if (myTimeIndex >= myFormatTo.sampleRate) {
      // myFragmentIndex += time * myFormatFrom.sampleRate
      myFragmentIndex += myTimeIndex / myFormatTo.sampleRate;
      myTimeIndex %= myFormatTo.sampleRate;
    }

    if (myFragmentIndex >= myFormatFrom.fragmentSize) {
      myFragmentIndex %= myFormatFrom.fragmentSize;

      Int16* nextFragment = myNextFragmentCallback();
      if (nextFragment)
        myCurrentFragment = nextFragment;
      else {
#ifdef __LIB_RETRO__
        if((i+1) < outputSamples)
#endif
        myUnderrunLogger.log();
        myIsUnderrun = true;
      }
    }
  }
}
