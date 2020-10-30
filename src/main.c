
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <math.h>
#include <stdint.h>
#include "midi.h"

// The DAC rate
#define RATE 30000
// The wavetable size
#define N 100
short int sine[N];
short int sawtooth[N];
short int square[N];
short int hybrid[N];
short int hybrid2[N];
// The number of simultaneous voices to support.
#define VOICES 15

// N powers of the the 12th-root of 2.
#define STEP1 1.05946309436
#define STEP2 (STEP1*STEP1)
#define STEP3 (STEP2*STEP1)
#define STEP4 (STEP3*STEP1)
#define STEP5 (STEP4*STEP1)
#define STEP6 (STEP5*STEP1)
#define STEP7 (STEP6*STEP1)
#define STEP8 (STEP7*STEP1)
#define STEP9 (STEP8*STEP1)

// Macros for computing the fixed point representation of
// the step size used for traversing a wavetable of size N
// at a rate of RATE to produce tones of various doublings
// and halvings of 440 Hz.  (The "A" above middle "C".)
#define A14    ((13.75   * N/RATE) * (1<<16)) /* A0 */
#define A27    ((27.5    * N/RATE) * (1<<16)) /* A1 */
#define A55    ((55.0    * N/RATE) * (1<<16)) /* A2 */
#define A110   ((110.0   * N/RATE) * (1<<16)) /* A3 */
#define A220   ((220.0   * N/RATE) * (1<<16)) /* A4 */
#define A440   ((440.0   * N/RATE) * (1<<16)) /* A5 */
#define A880   ((880.0   * N/RATE) * (1<<16)) /* A6 */
#define A1760  ((1760.0  * N/RATE) * (1<<16)) /* A7 */
#define A3520  ((3520.0  * N/RATE) * (1<<16)) /* A8 */
#define A7040  ((7040.0  * N/RATE) * (1<<16)) /* A9 */
#define A14080 ((14080.0 * N/RATE) * (1<<16)) /* A10 */


// A table of steps for each of 128 notes.
// step[60] is the step size for middle C.
// step[69] is the step size for 440 Hz.
const int step[] = {
        A14 / STEP9,    // C                         C-1
        A14 / STEP8,    // C# / Db
        A14 / STEP7,    // D
        A14 / STEP6,    // D# / Eb
        A14 / STEP5,    // E
        A14 / STEP4,    // F
        A14 / STEP3,    // F# / Gb
        A14 / STEP2,    // G
        A14 / STEP1,    // G# / Ab
        A14,            // A27                       A0
        A14 * STEP1,    // A# / Bb
        A14 * STEP2,    // B
        A14 * STEP3,    // C                         C0
        A14 * STEP4,    // C# / Db
        A14 * STEP5,    // D
        A27 * STEP6,    // D# / Eb
        A27 / STEP5,    // E
        A27 / STEP4,    // F
        A27 / STEP3,    // F# / Gb
        A27 / STEP2,    // G
        A27 / STEP1,    // G# / Ab
        A27,            // A27                       A1
        A27 * STEP1,    // A# / Bb
        A27 * STEP2,    // B
        A27 * STEP3,    // C                         C1
        A27 * STEP4,    // C# / Db
        A27 * STEP5,    // D
        A27 * STEP6,    // D# / Eb
        A55 / STEP5,    // E
        A55 / STEP4,    // F
        A55 / STEP3,    // F# / Gb
        A55 / STEP2,    // G
        A55 / STEP1,    // G# / Ab
        A55,            // A55                       A2
        A55 * STEP1,    // A# / Bb
        A55 * STEP2,    // B
        A55 * STEP3,    // C                         C2
        A55 * STEP4,    // C# / Db
        A55 * STEP5,    // D
        A55 * STEP6,    // D# / Eb
        A110 / STEP5,   // E
        A110 / STEP4,   // F
        A110 / STEP3,   // F# / Gb
        A110 / STEP2,   // G
        A110 / STEP1,   // G# / Ab
        A110,           // A110                     A3
        A110 * STEP1,   // A# / Bb
        A110 * STEP2,   // B
        A110 * STEP3,   // C                        C3
        A110 * STEP4,   // C# / Db
        A110 * STEP5,   // D
        A110 * STEP6,   // D# / Eb
        A220 / STEP5,   // E
        A220 / STEP4,   // F
        A220 / STEP3,   // F# / Gb
        A220 / STEP2,   // G
        A220 / STEP1,   // G# / Ab
        A220,           // A220                     A4
        A220 * STEP1,   // A# / Bb
        A220 * STEP2,   // B
        A220 * STEP3,   // C (middle C)             C4 (element #60)
        A220 * STEP4,   // C# / Db
        A220 * STEP5,   // D
        A220 * STEP6,   // D# / Eb
        A440 / STEP5,   // E
        A440 / STEP4,   // F
        A440 / STEP3,   // F# / Gb
        A440 / STEP2,   // G
        A440 / STEP1,   // G# / Ab
        A440,           // A440                     A5
        A440 * STEP1,   // A# / Bb
        A440 * STEP2,   // B
        A440 * STEP3,   // C                        C5
        A440 * STEP4,   // C# / Db
        A440 * STEP5,   // D
        A440 * STEP6,   // D# / Eb
        A880 / STEP5,   // E
        A880 / STEP4,   // F
        A880 / STEP3,   // F# / Gb
        A880 / STEP2,   // G
        A880 / STEP1,   // G# / Ab
        A880,           // A880                     A6
        A880 * STEP1,   // A# / Bb
        A880 * STEP2,   // B
        A880 * STEP3,   // C                        C6
        A880 * STEP4,   // C# / Db
        A880 * STEP5,   // D
        A880 * STEP6,   // D# / Eb
        A1760 / STEP5,  // E
        A1760 / STEP4,  // F
        A1760 / STEP3,  // F# / Gb
        A1760 / STEP2,  // G
        A1760 / STEP1,  // G# / Ab
        A1760,          // A1760                   A7
        A1760 * STEP1,  // A# / Bb
        A1760 * STEP2,  // B
        A1760 * STEP3,  // C                       C7
        A1760 * STEP4,  // C# / Db
        A1760 * STEP5,  // D
        A1760 * STEP6,  // D# / Eb
        A3520 / STEP5,  // E
        A3520 / STEP4,  // F
        A3520 / STEP3,  // F# / Gb
        A3520 / STEP2,  // G
        A3520 / STEP1,  // G# / Ab
        A3520,          // A3520                   A8
        A3520 * STEP1,  // A# / Bb
        A3520 * STEP2,  // B
        A3520 * STEP3,  // C                       C8
        A3520 * STEP4,  // C# / Db
        A3520 * STEP5,  // D
        A3520 * STEP6,  // D# / Eb
        A7040 / STEP5,  // E
        A7040 / STEP4,  // F
        A7040 / STEP3,  // F# / Gb
        A7040 / STEP2,  // G
        A7040 / STEP1,  // G# / Ab
        A7040,          // A7040                   A9
        A7040 * STEP1,  // A# / Bb
        A7040 * STEP2,  // B
        A7040 * STEP3,  // C                       C9
        A7040 * STEP4,  // C# / Db
        A7040 * STEP5,  // D
        A7040 * STEP6,  // D# / Eb
        A14080 / STEP5, // E
        A14080 / STEP4, // F
        A14080 / STEP3, // F# / Gb
        A14080 / STEP2, // G
};

// An array of "voices".  Each voice can be used to play a different note.
// Each voice can be associated with a channel (explained later).
// Each voice has a step size and an offset into the wave table.
struct {
    uint8_t in_use;
    uint8_t note;
    uint8_t chan;
    uint8_t volume;
    int     step;
    int     offset;
    int last_sample;
    int16_t *wavetable;
} voice[VOICES];

// Initialize the DAC so that it can output analog samples
// on PA4.  Configure it to either be software-triggered
// or triggered by TIM6.
void init_DAC(void) {

    // TODO: you fill this in.
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;
    GPIOA -> MODER |= GPIO_MODER_MODER4;
    DAC -> CR &= ~DAC_CR_TSEL1;
    DAC -> CR &= ~DAC_CR_EN1; // disable DAC channel
    // DAC -> CR &= ~DAC_CR_BOFF1; // turn on buffer
    DAC -> CR |= DAC_CR_TEN1; // enable trigger
    DAC -> CR &= ~DAC_CR_TSEL1; // select software trigger
    DAC -> CR |= DAC_CR_EN1; // re-enable DAC channel
}

// Initialize Timer 6 so that it calls TIM6_DAC_IRQHandler
// at exactly RATE times per second.  You'll need to select
// a PSC value and then do some math on the system clock rate
// to determine the value to set for ARR.
void init_TIM6(void) {

    // TODO: you fill this in.
    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6 -> PSC = 48 - 1;
    TIM6 -> ARR = 1000000 / RATE - 1;
    TIM6 -> DIER |= TIM_DIER_UDE;
    TIM6 -> CR1 |= TIM_CR1_CEN;
    TIM6 -> CR2 &= ~TIM_CR2_MMS;
    TIM6 -> CR2 |= TIM_CR2_MMS_1;
    //(NVIC -> ISER)[0] = (1 << TIM6_DAC_IRQn);
    //NVIC_SetPriority(TIM6_DAC_IRQn,0);
}

// sine wave
void init_sine(void) {
    int x;
    for(x=0; x<N; x++) {
        sine[x] = 32767 * sin(2 * M_PI * x / N);
    }
}

// sawtooth wave
void init_sawtooth(void) {
    int x;
    for(x=0; x<N; x++)
        sawtooth[x] = 32767.0 * (x - N/2) / (1.0*N);
}

// square wave
void init_square(void) {
    int x;
    for(x=0; x<N; x++)
        if (x < N/2)
            square[x] = -32767;
        else
            square[x] = 32767;
}

// 1/2 amplitude sine wave added to 1/2 amplitude sawtooth wave
void init_hybrid(void) {
    int x;
    for(x=0; x<N; x++)
        hybrid[x] = 16383 * sin(2 * M_PI * x / N) + 16383.0 * (x - N/2) / (1.0*N);
}

// 3/4 amplitude sine wave added to 1/4 amplitude sawtooth wave
void init_hybrid2(void) {
    int x;
    for(x=0; x<N; x++)
        hybrid2[x] = 3*8191 * sin(2 * M_PI * x / N) + 8191.0 * (x - N/2) / (1.0*N);
}

// Find the voice current playing a note, and turn it off.
void note_off(int time, int chan, int key, int velo)
{
    int n;
    for(n=0; n<sizeof voice / sizeof voice[0]; n++) {
        if (voice[n].in_use && voice[n].note == key) {
            voice[n].in_use = 0; // disable it first...
            voice[n].chan = 0;   // ...then clear its values
            voice[n].note = key;
            voice[n].step = step[key];
            return;
        }
    }
}

int16_t *channel_wavetable[20]; // It's zero, by default.

void program_change(int time, int chan, int prog)
{
    channel_wavetable[chan] = hybrid; // Use this by default.
    if (prog >= 71 && prog <= 79) // flutes, recorders, etc.
        channel_wavetable[chan] = sine;
    if (prog == 29) // overdriven guitar
        channel_wavetable[chan] = square;
    if (prog >= 52 && prog <= 54) // choir voices
        channel_wavetable[chan] = sine;
}

// Find an unused voice, and use it to play a note.
void note_on(int time, int chan, int key, int velo)
{
    if (velo == 0) {
        note_off(time, chan, key, velo);
        return;
    }
    int n;
    for(n=0; n<sizeof voice / sizeof voice[0]; n++) {
        if (channel_wavetable[chan] == 0)
            voice[n].wavetable = hybrid;
        else
            voice[n].wavetable = channel_wavetable[chan];
        if (voice[n].in_use == 0) {
            voice[n].note = key;
            voice[n].step = step[key];
            voice[n].offset = 0;
            voice[n].chan = chan;
            voice[n].in_use = 1;
            voice[n].volume = velo;
            return;
        }
    }
}

void set_tempo(int time, int value, const MIDI_Header *hdr)
{
    // This assumes that the TIM2 prescaler divides by 48.
    // It sets the timer to produce an interrupt every N
    // microseconds, where N is the new tempo (value) divided by
    // the number of divisions per beat specified in the MIDI header.
    TIM2->ARR = value/hdr->divisions - 1;
}


void init_TIM2(int n) {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2 -> PSC = 48 - 1;
    TIM2 -> ARR = n - 1;
    TIM2 -> CR1 |= TIM_CR1_ARPE;
    TIM2 -> DIER |= TIM_DIER_UIE;
    TIM2 -> CR1 |= TIM_CR1_CEN;
    (NVIC -> ISER)[0] = (1 << TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn,3);
}

int time = 0;
int n = 0;
void TIM2_IRQHandler(void)
{
    // TODO: Remember to acknowledge the interrupt here!
    TIM2 -> SR &= ~TIM_SR_UIF;
    midi_play();
}
void pitch_wheel(int time, int chan, int val) {
    float multiplier = pow(1.05946309436, val - 0x2000);
    for(int x=0; x < sizeof(voice)/sizeof(voice[0]); x++) {
        if(voice[x].in_use && voice[x].chan == chan) {
            voice[x].step *= multiplier;
        }
    }
}

uint16_t queue[200]; // the circular buffer

void DMA1_Channel2_3_IRQHandler(void)
{
    uint16_t *arr;
    if (DMA1->ISR & DMA_ISR_HTIF3) {
        // At halfway mark.  Refill the first half of the buffer.
        arr = &queue[0]; // point to beginning of buffer
        // Need to clear both the half-transfer and global flags.
        DMA1->IFCR = DMA_IFCR_CHTIF3|DMA_IFCR_CGIF3;
    } else {
        // Total complete.  Refill the second half of the buffer.
        arr = &queue[100]; // point to second half of buffer
        // Need to clear both the transfer-complete and global flags.
        DMA1->IFCR = DMA_IFCR_CTCIF3|DMA_IFCR_CGIF3;
    }

    int i;
    for(i = 0; i < 100; i++) {


        // TODO: Copy the sample generation from TIM6_DAC_IRQHandler()
        int x;
        int sample = 0;
        for(x=0; x < sizeof(voice)/sizeof(voice[0]); x++) {
            if (voice[x].in_use) {
                voice[x].offset += voice[x].step;
                if (voice[x].offset >= N<<16)
                    voice[x].offset -= N<<16;
                voice[x].last_sample = voice[x].wavetable[voice[x].offset>>16];
                sample += (voice[x].last_sample * voice[x].volume) >> 4;
            } else if (voice[x].volume != 0) {
                sample += (voice[x].last_sample * voice[x].volume) >> 4;
                voice[x].volume --;
            }
        }
        sample = sample / 1024 + 2048;
        if (sample > 4095) sample = 4095;
        else if (sample < 0) sample = 0;
        //DAC->DHR12R1 = sample;

        arr[i] = sample;
    }
}

void init_DMA() {
    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3 -> CCR &= ~DMA_CCR_EN;
    DMA1_Channel3 -> CMAR = (uint32_t) queue;
    DMA1_Channel3 -> CPAR = (uint32_t) &(DAC -> DHR12L1);
    DMA1_Channel3 -> CNDTR = sizeof queue / sizeof queue[0];
    DMA1_Channel3 -> CCR |= DMA_CCR_DIR;
    DMA1_Channel3 -> CCR |= DMA_CCR_MINC;
    DMA1_Channel3 -> CCR &= ~DMA_CCR_PSIZE;
    DMA1_Channel3 -> CCR &= ~DMA_CCR_MSIZE;
    DMA1_Channel3 -> CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel3 -> CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel3 -> CCR |= DMA_CCR_CIRC;
    DMA1_Channel3 -> CCR |= DMA_CCR_TCIE;
    DMA1_Channel3 -> CCR |= DMA_CCR_HTIE;
    DMA1_Channel3 -> CCR |= DMA_CCR_EN;
    (NVIC -> ISER)[0] = (1 << DMA1_Channel2_3_IRQn);
    NVIC_SetPriority(DMA1_Channel2_3_IRQn,0);

}


extern uint8_t midifile[];
int main(void)
{
    init_hybrid();
    init_DAC();
    init_DMA();
    init_TIM6();
    MIDI_Player *mp = midi_init(midifile);
    // The default rate for a MIDI file is 2 beats per second
    // with 48 ticks per beat.  That's 500000/48 microseconds.
    init_TIM2(10417);
    for(;;)
        asm("wfi");
}

