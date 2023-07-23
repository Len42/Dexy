#pragma once

// Dexy - Main header file

#include "CompileDefs.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <map>
#include <numeric>
#include <string> // only for ShowDecl.h
#include <string_view>
#include <utility>
#include <variant>
#include <stdio.h>

// C library definitions that aren't in header files
extern "C" {
int _write(int handle, char *buffer, int length);
#define STDIO_HANDLE_STDIN  0
#define STDIO_HANDLE_STDOUT 1
#define STDIO_HANDLE_STDERR 2
}

using namespace std::literals;

#include "PicoSdk.h"

#include "Debug.h"
#include "ShowDecl.h"
#include "Version.h"

#include "Defs.h"
#include "Utils.h"
#include "Error.h"
#include "Lockout.h"
#include "Flash.h"
#include "Serialize.h"

#include "Patches.h"
#include "PatchData.h"
#include "PatchChanges.h"

#include "CritSec.h"
#include "Defer.h"
#include "Gpio.h"
#include "DataTable.h"
#include "WaveTable.h"
#include "SineWave.h"
#include "AdcInput.h"
#include "Envelope.h"
#include "Operator.h"
#include "SpiDac.h"
#include "SynthAlgos.h"
#include "Synth.h"
#include "Tasks.h"
#include "TestTasks.h"
#include "Watchdog.h"
#include "SerialIO.h"
#include "Display.h"
#include "Encoder.h"
#include "UI.h"
#include "IrqDispatch.h"
#include "Core0.h"
#include "Core1.h"
