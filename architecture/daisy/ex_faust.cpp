/************************************************************************
 IMPORTANT NOTE : this file contains two clearly delimited sections :
 the ARCHITECTURE section (in two parts) and the USER section. Each section
 is governed by its own copyright and license. Please check individually
 each section for license and copyright information.
 *************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
 FAUST Architecture File
 Copyright (C) 2020 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This Architecture section is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3 of
 the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; If not, see <http://www.gnu.org/licenses/>.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 
 ************************************************************************
 ************************************************************************/

#include "daisysp.h"
#include "daisy_seed.h"

#include "faust/gui/meta.h"
#include "faust/gui/UI.h"
#include "faust/gui/DaisyControlUI.h"
#include "faust/dsp/dsp.h"

#ifdef MIDICTRL
#include "faust/midi/daisy-midi.h"
#include "faust/gui/MidiUI.h"
#endif

using namespace daisysp;
using namespace daisy;
using namespace std;

/******************************************************************************
 *******************************************************************************
 
 VECTOR INTRINSICS
 
 *******************************************************************************
 *******************************************************************************/

<<includeIntrinsic>>

/********************END ARCHITECTURE SECTION (part 1/2)****************/

/**************************BEGIN USER SECTION **************************/

<<includeclass>>

/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/

#ifdef POLY
#include "faust/dsp/poly-dsp.h"
mydsp_poly* dsp_poly = nullptr;
#endif

DaisySeed hw;
DaisyControlUI* control_UI;
dsp* DSP = nullptr;

#ifdef MIDICTRL
list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;
#endif

#define MY_BUFFER_SIZE 32

static void AudioCallback(float** in, float** out, size_t count)
{
    // Update controllers
    control_UI->update();
    
    // DSP processing
    DSP->compute(count, in, out);
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    hw.Configure();
    hw.Init();
    
    // allocate DSP
#ifdef POLY
    int nvoices = 0;
    bool midi_sync = false;
    mydsp* dsp = new mydsp();
    MidiMeta::analyse(dsp, midi_sync, nvoices);
    dsp_poly = new mydsp_poly(dsp, nvoices, true, true);
    DSP = dsp_poly;
#else
    DSP = new mydsp();
#endif
    
    // set buffer-size
    hw.SetAudioBlockSize(MY_BUFFER_SIZE);
    
    // init Faust DSP
    DSP->init(hw.AudioSampleRate());
    
    // setup controllers
    control_UI = new DaisyControlUI(&hw, hw.AudioSampleRate()/MY_BUFFER_SIZE);
    DSP->buildUserInterface(control_UI);
    
    // start ADC
    hw.adc.Start();
    
    // define and start callback
    hw.StartAudio(AudioCallback);
    
#ifdef MIDICTRL
    daisy_midi midi_handler;
#ifdef POLY
    midi_handler.addMidiIn(dsp_poly);
#endif
    MidiUI midi_interface(&midi_handler);
    DSP->buildUserInterface(&midi_interface);
    midi_handler.startMidi();
#endif
    
    // MIDI handling loop
    while(1) {
#ifdef MIDICTRL
        midi_handler.processMidi();
#endif
    }
}

/********************END ARCHITECTURE SECTION (part 2/2)****************/
