/*
 * This application creates a random mix of RGB color on the Booster LED
 * It lets the user guess what colors are using the top and bottom buttons of the booster board.
 * Then, the application tells the user if he/she was right or wrong.
 *
 * The general operation is as follows:
 * 1) an opening screen is shown for a few seconds
 * 2) an instructions page is shown. It stays on until the bottom button is pushed.
 * 3) A random RGB mix is created (this can be even dark - meaning none of R, or G, or B)
 * 4) The user can press the top button to traverse a menu by moving an arrow.
 * 5) When the user presses the bottom button on a certain color, a * appears in front of that
 *    color, which means the user is guessing the color mix has that specific color in it.
 * 6) When the user presses the bottom button on "End", the application goes to a result page.
 * 7) In the result page, the application shows if he/she was right or wrong. This is screen is
 *    shown for a few seconds. Then the application goes back to step 2.
 *
 *
 *
 */

#include <LED_HAL.h>
#include <Buttons_HAL.h>
#include <Timer_HAL.h>
#include <Display_HAL.h>
#include <ADC_HAL.h>

#define OPENING_WAIT 1000 // 1 second or 1000 ms
#define ENDTEST_WAIT 2000 // 2 second or 2000 ms

// The top and bottom options locations on 2nd and 5th row are defined as macros here.
#define TOP_OPTION_POS 1
#define BOTTOM_OPTION_POS 4

// The RGB colors
typedef enum {RED, GREEN, BLUE} color_t;

// The last option on the menu that ends the test.
// Since this option is after Red, Green, Blue, it is the 4th choice and its index is 3
#define END 3

// This structure helps us track the actual color mix and the guessed color mix
typedef struct {
    bool hasRed;
    bool hasGreen;
    bool hasBlue;
} colorMix_t;


void DrawOpeningScreen()
{
    LCDClearDisplay(MY_BLACK);
    PrintString("COLOR TEST", 2, 2);
    PrintString("by", 3, 3);
    PrintString("LN", 4, 2);
}

void DrawInstructionsScreen()
{
    LCDClearDisplay(MY_BLACK);
    PrintString("Guess RGB mix.", 1, 1);
    PrintString("During test:", 2, 1);
    PrintString("BTM: move arrow", 3, 1);
    PrintString("TOP: select", 4, 1);
    PrintString("BTM to start", 7, 1);
}

void DrawTestScreen()
{
    LCDClearDisplay(MY_BLACK);
    PrintString("Red", 1, 3);
    PrintString("Green", 2, 3);
    PrintString("Blue", 3, 3);
    PrintString("End test", 4, 3);

    PrintString("BTM: move arrow", 6, 1);
    PrintString("TOP: select", 7, 1);

    LCDDrawChar(1, 1, '>');
}

// This screen displays different things based on the result of the test
void DrawEndTestScreen(bool correct)
{
    LCDClearDisplay(MY_BLACK);
    if (correct)
    {
        PrintString("Right!", 2, 3);
    } else
    {
        PrintString("Wrong!", 2, 3);
    }
}


// This function compares the actual and the guessed color mix.
// It returns true if they are the same and false otherwise.
bool match(colorMix_t* guessColor, colorMix_t* actualColor)
{
    if ((guessColor->hasBlue == actualColor->hasBlue) &&
        (guessColor->hasGreen == actualColor->hasGreen) &&
        (guessColor->hasRed == actualColor->hasRed))
        return true;
    return false;
}

// This is the function used for guessing the colors
// Its inputs are the arrow position pointer and the guessed color pointer
// The function uses the content of these pointers and modifies it for the caller
// The returned value of the function is false if the test is not finished yet. Otherwise, it is true.
// Notice that this function has no internal states (no static variable). Everything depends on the inputs given to it.
// This means this function is not implementing an FSM and simply is a function that helps the testFSM.
// We have cut it out from the testFSM to make the code more readable and easier to debug
bool guess(unsigned int * arrowPosPointer, colorMix_t* guessColor)
{
    // In order to make writing the code easier, I dump the content of the location pointer is pointing to into a local variable.
    unsigned int arrowPos = *arrowPosPointer;

    // the local variable used to name the choice
    unsigned int choice;

    // output
    bool finished = false;

    // If the bottom button is pushed, it moves the arrow down on the display.
    // If the arrow is on the lowest option, it wraps around to the top

    if (Booster_Bottom_Button_Pushed())
    {
        // Clearing the old arrow
        LCDDrawChar(arrowPos, 1, ' ');

        // moving the arrow down
        arrowPos++;

        // wrap around if needed
        if (arrowPos > BOTTOM_OPTION_POS)
            arrowPos = TOP_OPTION_POS;

        // draw the new arrow
        LCDDrawChar(arrowPos, 1, '>');
    }

    // pressing the top button makes the selection by putting a star on the right side of the color
    // If this button is pressed in front of the "end" option, the test ends
    if (Booster_Top_Button_Pushed())
    {
        // Draw the *
        LCDDrawChar(arrowPos, 9, '*');

        // The choice is the index of the choice made
        choice = arrowPos - TOP_OPTION_POS;
        switch (choice)
        {
        case RED:
            guessColor->hasRed = true;
            break;
        case GREEN:
            guessColor->hasGreen = true;
            break;
        case BLUE:
            guessColor->hasBlue = true;
            break;
        default:
            finished = true;
        }
    }

    // We need to copy the updated arrowPos to the memory location pointed by arrowPosPointer so that testFSM can see it
    *arrowPosPointer = arrowPos;
    return finished;

}


bool testFSM(bool newTest, bool* resultPointer)
{
    // local variables that need to keep their value from call to call
    static colorMix_t actualColor, guessColor;
    static color_t colorIndex;
    static enum  {setup, lightup, testing} testState;
    static unsigned int arrowPos;

    // regular local variables
    unsigned int vx, vy;
    bool randomBit;

    // default output
    bool finished = false;

    // since the below variables are static, when we start a new test, they start with the value they had at the end of the last test.
    // This is not desirable. Therefore, we have to tell testFSM that a new test has started and all the static variables should be reset
    if (newTest)
    {
        testState = setup;
        colorIndex = RED;
        arrowPos = TOP_OPTION_POS;

        // all LEDs should be turned off at the beginning of a new test
        TurnOFF_Booster_Blue_LED();
        TurnOFF_Booster_Green_LED();
        TurnOFF_Booster_Red_LED();

        // reset the guessed colors
        guessColor.hasRed = false;
        guessColor.hasGreen = false;
        guessColor.hasBlue = false;
    }

    switch (testState)
    {
    // In this state we create the color mix.
    // We do this in three rounds. This means the testFSM is called three times before we leave this state.
    // In each round, we create one random bit and assign it to one of the Red, green, blue options.
    // If we did all of this in one round, we would have stated in this function too long and could be blocking other stimuli of the application
    case setup:
       getSampleJoyStick(&vx, &vy);
       randomBit = (vx%2) ^ (vy%2);
        switch (colorIndex)
        {
        case RED:
            actualColor.hasRed = randomBit;
            break;

        case GREEN:
            actualColor.hasGreen = randomBit;
            break;

        case BLUE:
            actualColor.hasBlue = randomBit;
            break;

        default:
            testState = lightup;
        }
        colorIndex++;
        break;

    // In this state, we light up the LEDs based on the random bits we picked in the previous state
    // The three commented out lines were in the code for debugging purposes.
    // If you keep guessing wrong, perhaps uncomment them and see what colors are present in each mixture.
    case lightup:
        if (actualColor.hasRed)
        {
            TurnON_Booster_Red_LED();
//          PrintString("R", 7, 1);
        }
        if (actualColor.hasGreen)
        {
            TurnON_Booster_Green_LED();
//          PrintString("G", 7, 2);
        }
        if (actualColor.hasBlue)
        {
            TurnON_Booster_Blue_LED();
 //         PrintString("B", 7, 3);
        }
        testState = testing;
        break;

    // This is the main state of this FSM, we call the menu fsm from here, which updates the arrow position and the guessed color structure
    case testing:
        finished = guess(&arrowPos, &guessColor);

    }

    // If the test is finished, we need to compare the actual and guessed mixture.
    // The result of this comparison goes in the memory location pointed by resultPointer
    if (finished)
        *resultPointer = match(&guessColor, &actualColor);

    return finished;
}


void ScreensFSM()
{
    // These are local variables for this function that need to keep their value from previous call
    static enum states {INCEPTION, OPENING, INSTRUCTIONS, TEST, TESTEND} state = INCEPTION;
    static OneShotSWTimer_t OST;
    static bool newTest;

    // Set the default outputs
    bool drawOpeningScreen = false;
    bool drawInstructionsScreen = false;
    bool drawTestScreen = false;
    bool drawEndScreen = false;
    bool startSWTimer = false;

    // Inputs of the FSM
    bool testFinished;
    bool result;
    bool swTimerExpired;
    bool bottomPushed;

    switch (state)
    {
    case INCEPTION:
        // State transition
        state = OPENING;

        // The output(s) that are affected in this transition
        drawOpeningScreen = true;
        startSWTimer = true;

        break;

    case OPENING:
        // Some states depend on some specific inputs that we should get (create)
        // In this case we need to call the below function to get that input
        swTimerExpired = OneShotSWTimerExpired(&OST);
        if (swTimerExpired)
        {
            // State transition
            state = INSTRUCTIONS;

            // The output(s) that are affected in this transition
            drawInstructionsScreen = true;
        }
        break;

    case INSTRUCTIONS:
        // This state depends on the state of the bottom button. So, we get it by calling the below function
        bottomPushed = Booster_Bottom_Button_Pushed();
        if (bottomPushed)
        {
            state = TEST;
            newTest = true;

            drawTestScreen = true;
        }
        break;

    case TEST:
        // This state needs two inputs, whether the test is over and wheter the result was right or wrong.
        // One of the is the output of the function (returned value), the other is passed by reference (result) to be filled by the callee
        testFinished = testFSM(newTest, &result);
        if (testFinished)
        {
            // State transition
            state = TESTEND;

            // The output(s) that are affected in this transition
            drawEndScreen = true;
            startSWTimer = true;
         }
        newTest = false;
        break;

    case TESTEND:
        swTimerExpired = OneShotSWTimerExpired(&OST);
        if (swTimerExpired)
        {
            // State transition
            state = INSTRUCTIONS;

            // The output(s) that are affected in this transition
            drawInstructionsScreen = true;
        }
        break;
    } // End of switch-case

    // Implement actions based on the outputs of the FSM
    if (startSWTimer)
    {
        InitOneShotSWTimer(&OST, TIMER32_1_BASE, OPENING_WAIT);
        StartOneShotSWTimer(&OST);
    }

    if (drawOpeningScreen)
       DrawOpeningScreen();

    if (drawInstructionsScreen)
        DrawInstructionsScreen();

    if (drawTestScreen)
       DrawTestScreen();

    // This screen does different things based on the result of the test, so we pass the result to it
    if (drawEndScreen)
       DrawEndTestScreen(result);

}

int main(void) {

    WDT_A_hold(WDT_A_BASE);

    BSP_Clock_InitFastest();
    InitGraphics();
    InitHWTimers();
    InitButtons();
    InitLEDs();
    initADC();
    initJoyStick();
    startADC();

    while (1)
    {
        ScreensFSM();
    }

}
