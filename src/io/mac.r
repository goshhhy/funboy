#include "Processes.r"
#include "Menus.r"

resource 'SIZE' (-1) {
	reserved,
	ignoreSuspendResumeEvents,
	reserved,
	cannotBackground,
	needsActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	notHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	1024 * 1024 * 8,
	1024 * 1024 * 1
};


#define mApple 128
resource 'MENU' (mApple, preload) {
    mApple, textMenuProc;
    0b11111111111111111111111111111101, enabled;
    apple, {
        "About Funboy...",
            noicon,
            nokey,
            nomark,
            plain;
        "-",
            noicon, nokey, nomark, plain
    }

};

#define mFile 129
resource 'MENU' (mFile, preload) {
    mFile, textMenuProc;
    0b00000000000000000000000000001011, enabled;
    "File", {
        "Open", noicon, "O", nomark, plain;
        "Close", noicon, "W", nomark, plain;
        "-", noicon, nokey, nomark, plain;
        "Quit", noicon, "Q", nomark, plain;
    }
};

#define mEmulation 130
resource 'MENU' (mEmulation, preload) {
    mEmulation,
    textMenuProc,
    0b00000000000000000000000000001011,
    enabled,
    "Emulation", {
        "Pause", noicon, "P", nomark, plain;
        "Reset", noicon, "R", nomark, plain;
        "-", noicon, nokey, nomark, plain;
        "Turbo", noicon, "T", nomark, plain;
    }
};

resource 'MBAR' (128) {
    {mApple, mFile, mEmulation };
};