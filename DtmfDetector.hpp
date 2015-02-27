/** Author:       Plyashkevich Viatcheslav <plyashkevich@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * All rights reserved.
 */


#ifndef DTMF_DETECTOR
#define DTMF_DETECTOR

#include "types_cpp.hpp"
//Uncomment WIDEBAND_SUPPORT to enable it. Works only for DTMF 1 and 0 detections
//#define WIDEBAND_SUPPORT

typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Int32     INT32;
typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Uint32    UINT32;
typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Int16     INT16;
typedef Types<sizeof(long int), sizeof(int), sizeof(short int), sizeof(char)>::Uint16    UINT16;


// DTMF detector object

// N.B. Not sure why this interface is necessary, as the only class to 
// implement it is the DtmfDetector.
class DtmfDetectorInterface
{
public:
    // The maximum number of detected tones
    static const UINT32 NUMBER_OF_BUTTONS = 65;
    // A fixed-size array to store the detected tones.  The array is
    // NUL-terminated.
    char dialButtons[NUMBER_OF_BUTTONS];
    // A constant pointer to expose dialButtons for read-only access
	#ifndef WIDEBAND_SUPPORT
    char *const pDialButtons;
    #endif
    // The number of tones detected (stored in dialButtons)
    mutable INT16 indexForDialButtons;
public:
    INT32 getIndexDialButtons() // The number of detected push buttons, max number = 64
    const
    {
        return indexForDialButtons;
    }
   	#ifndef WIDEBAND_SUPPORT
    char *getDialButtonsArray() // Address of array, where store detected push buttons
    const
    {
        return pDialButtons;
    }
    #endif
    void zerosIndexDialButton() // Zeros of a detected button array
    const
    {
        indexForDialButtons = 0;
    }
	#ifdef WIDEBAND_SUPPORT
	DtmfDetectorInterface():indexForDialButtons(0)
	#else
    DtmfDetectorInterface():indexForDialButtons(0), pDialButtons(dialButtons)
    #endif
    {
        dialButtons[0] = 0;
    }
};

class DtmfDetector : public DtmfDetectorInterface
{
protected:
    // These coefficients include the 8 DTMF frequencies plus 10 harmonics.
    static const unsigned COEFF_NUMBER=18;
    // A fixed-size array to hold the coefficients
    static const INT16 CONSTANTS[COEFF_NUMBER];
    // This array keeps the entire buffer PLUS a single batch.
    INT16 *pArraySamples;
    // The magnitude of each coefficient in the current frame.  Populated
    // by goertzel_filter
    INT32 T[COEFF_NUMBER];
    // An array of size SAMPLES.  Used as input to the Goertzel function.
    INT16  *internalArray;
    // The size of the entire buffer used for processing samples.  
    // Specified at construction time.
#ifndef WIDEBAND_SUPPORT
    const INT32 frameSize; //Size of a frame is measured in INT16(word)
#endif
    // The number of samples to utilize in a single call to Goertzel.
    // This is referred to as a frame.
    #ifdef WIDEBAND_SUPPORT
    static const UINT32 SAMPLES;
    UINT32 frameCount;
    #else
    static const INT32 SAMPLES;
    // This gets used for a variety of purposes.  Most notably, it indicates
    // the start of the circular buffer at the start of ::dtmfDetecting.
    INT32 frameCount;
    #endif
    // The tone detected by the previous call to DTMF_detection.
    char prevDialButton;
    // This flag is used to aggregate adjacent tones and spaces, i.e.
    //
    // 111111   222222 -> 12 (where a space represents silence)
    //
    // 1 means that during the previous iteration, we entered a tone 
    // (gone from silence to non-silence).
    // 0 means otherwise.
    //
    // While this is a member variable, it can by all means be a local
    // variable in dtmfDetecting.
    //
    // N.B. seems to not work.  In practice, you get this:
    //
    // 111111   222222 -> 1111122222
    char permissionFlag;
#ifdef WIDEBAND_SUPPORT
	char pattern_tone;
	UINT32 sample_count_flag;
	UINT32 sample_count;
	UINT32 pattern_count;
	char detected_tone;
#endif
    // Used for quickly determining silence within a batch.
    static INT32 powerThreshold;
    //
    // dialTonesToOhersTone is the higher ratio.
    // dialTonesToOhersDialTones is the lower ratio.
    //
    // It seems like the aim of this implementation is to be more tolerant
    // towards strong "dial tones" than "tones".  The latter include
    // harmonics.
    //
    static INT32 dialTonesToOhersTones;
    static INT32 dialTonesToOhersDialTones;

    // This protected function determines the tone present in a single frame.
    char DTMF_detection(INT16 short_array_samples[]);
public:
#ifdef WIDEBAND_SUPPORT
    const UINT32 frameSize; //Size of a frame is measured in INT16(word)
#endif
    // frameSize_ - input frame size
    DtmfDetector(INT32 frameSize_);
    ~DtmfDetector();
#ifdef WIDEBAND_SUPPORT
    char dtmfDetecting(INT16 inputFrame[], UINT32* p_pValid); // The DTMF detection.
#else
    void dtmfDetecting(INT16 inputFrame[]); // The DTMF detection.
#endif
    // The size of a inputFrame must be equal of a frameSize_, who
    // was set in constructor.
};

#endif
