#pragma once

#include "pitches.h"

#include <Arduino.h>

int blaze_notes = [
	NOTE_C4, NOTE_G4, NOTE_E4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_G4,
	NOTE_C4, NOTE_G4, NOTE_E4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_G4,
	NOTE_E4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_G4,
	NOTE_E4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_C4, NOTE_D4, NOTE_C4,
	NOTE_D4, NOTE_E4, NOTE_F4, NOTE_E4, NOTE_D4,
	NOTE_C4
];

int blaze_note_lengths = [
	4, 2, 8, 8, 4, 8, 8, 4,
	4, 2, 8, 8, 4, 8, 8, 4,
	8, 8, 4, 8, 8, 4,
	8, 8, 4, 8, 8, 4, 4, 2,
	8, 8, 4, 8, 4, 4
]

	// https://dragaosemchama.com/en/2019/02/songs-for-arduino/
	// https://www.onlinepianist.com/virtual-piano
	// https://www.instructables.com/Arduino-Tone-Music/
	// https://trinket.io/music
	/*

	C4 G4 ~ G4. E8 G8 A4 G8 E8 G4.
	C4 G4 ~ G4. E8 G8 A4 G8 E8 G8 ~ G8
	E8 G8 A4 G8 E8 G4
	E8 G8 A4 G8 E8 C4 D4 C2.
	D8 E8 F4 E8 D4 C2.
	*/