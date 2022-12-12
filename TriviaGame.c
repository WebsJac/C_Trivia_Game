//Adaptation of Template Code from ELEC 371 Course
//Trivia game using features common to FGPA board

//Notes to play: go to https://cpulator.01xz.net/?sys=nios-de0 and open this file
//1. Uncheck "Memory Suspicious Use of Bypass Cache" in settings (on left side)
//2. Uncheck "Instruction fetch: modified OP code" in settings (on left side)
//3. Click 'Compile and Load' in upper left corner
//4. Click the 'Memory' tab and go to address 200. Change this address to '06c02034' instead of '06c10034'

//Adjustments/Improvements for future: 
//1. Disable pushbutton interrupts while previous is still happening -- check if this is working.....
//2. Disable saving chars on stack. Only allow one so they don't lose points for accidental clicks
//3. Rand allows for repeat questions, we would want someway of preventing it from being a previously chosen question 

#ifndef _NIOS2_CONTROL_H_
#define _NIOS2_CONTROL_H_


#define NIOS2_WRITE_STATUS(value)  (__builtin_wrctl (0, value))

#define NIOS2_READ_IENABLE()	   (__builtin_rdctl (3))

#define NIOS2_WRITE_IENABLE(value) (__builtin_wrctl (3, value))

#define NIOS2_READ_IPENDING()	   (__builtin_rdctl (4))


#endif /* _NIOS2_CONTROL_H_ */

/*-----------------------------------------------------------------*/

#ifndef _TIMER_H_
#define _TIMER_H_


/* define pointer macros for accessing the timer interface registers */

#define TIMER_STATUS	((volatile unsigned int *) 0x10002000)

#define TIMER_CONTROL	((volatile unsigned int *) 0x10002004)

#define TIMER_START_LO	((volatile unsigned int *) 0x10002008)

#define TIMER_START_HI	((volatile unsigned int *) 0x1000200C)

#define LEDS ((volatile unsigned int *) 0x10000010)

#define TIMER_TO_BIT 0x1


#endif /* _TIMER_H_ */


#ifndef _LEDS_H_
#define _LEDS_H_


/* define pointer macro for accessing the LED interface data register */

#define LEDS	((volatile unsigned int *) 0x10000010)

#endif /* _LEDS_H_ */

#define HEX_DISPLAY ((volatile unsigned int*) 0x10000020)

#define JTAG_UART_DATA ((volatile unsigned int*) 0x10001000)
#define JTAG_UART_STATUS ((volatile unsigned int*) 0x10001004)


#define BUTTON ((volatile unsigned int*) 0x10000050)
#define BUTTON_MASK ((volatile unsigned int*) 0x10000058) //should interrupt be sent to processor
#define BUTTON_EDGE ((volatile unsigned int*) 0x1000005C) //write back here to clear an interrupt

void printChar (unsigned int ch);
void printString (char *s);
void printScore (int score);
void countdown(void);
int myRand(void);
int myRandinRange(void);
void askMathQuestion(void);
void askScienceQuestion(void);
void checkAnswer(void);
/*-----------------------------------------------------------------*/
/*             start of application-specific code                  */
/*-----------------------------------------------------------------*/

/* place additional #define macros here */


/* define global program variables here */
unsigned int hex_table[]={0x3F, 0x06, 0x5B, 0x4F,0x66, 0x6D, 0x7D, 0x07,0x7F, 0x6F}; //corresponding hex display to print 0-9 in hex
int score =0; //global to keep track of score, initialize to zero to start
char correctAnswer;

void interrupt_handler(void)
{
	unsigned int ipending;
    ipending = NIOS2_READ_IPENDING();

    if ((ipending & 0x1) == 0x1){ //timer interrupt
        *TIMER_STATUS = 0;
            if (*LEDS == 0x1){ //far right LED is on - stop timer and reset to leftmost LED
                *TIMER_CONTROL = 0x8; //stop timer
                *LEDS = 0x200;
                *BUTTON_MASK = 0x7; //reenable button interrupts
                NIOS2_WRITE_IENABLE(0x3); //reenable pushbutton interrupts in processor
                checkAnswer();
        }
        else{
             *LEDS = *LEDS >> 1; //bit shift by 1 to the right
        } 
    }


    if ((ipending & 0x2) == 0x2){ //button interrupt
        unsigned int pressed = *BUTTON_EDGE; //find out which button was pressed
        *BUTTON_EDGE = pressed; //clears the interrupt
        if (pressed == 0x1){
            score = 0;
            printScore(score);
        }

        if (pressed == 0x2){ //button 1 means add 1 for example
            NIOS2_WRITE_IENABLE(0x1); //disable pushbutton interrupts
            askMathQuestion();
            countdown();
        }
        if (pressed == 0x4){ //button 2 is pressed means add 10 for example
            NIOS2_WRITE_IENABLE(0x1); //disable pushbutton interrupts
            askScienceQuestion();
            countdown();
        }
    }
}


void Init (void)
{
    *TIMER_START_LO = 0xF080;
    *TIMER_START_HI = 0x02FA;
    *TIMER_CONTROL = 0x1; //Enables interrupt, but we dont want to have them until a question has been posed

    *LEDS = 0x200; //leftmost LED turns on

    *HEX_DISPLAY = 0x3F3F3F3F; //pattern to make it start at 0000

    *BUTTON_MASK = 0x7; //enable interrupts in both buttons - 1 and 2

    NIOS2_WRITE_IENABLE(0x3);
    NIOS2_WRITE_STATUS (0x1);

}

void printScore (int score){
    int thousands = score/1000; //use truncation to our advantage
    int hundreds = (score%1000)/100;
    int tens = (score%100)/10;
    int ones = score%10;
    *HEX_DISPLAY = ((hex_table[thousands] << 24)|(hex_table[hundreds] << 16)|(hex_table[tens] << 8)|(hex_table[ones])); //to produce the hex we want onto screen
  
}

void printChar (unsigned int ch){
	unsigned int st;
	do{
		st = *JTAG_UART_STATUS;
		st = st & 0xFFFF0000;
	}while (st == 0);
	*JTAG_UART_DATA = ch;
}

void printString (char *s){
	while (1){
		if (*s == '\0') break;
		else {
			printChar (*s);
			s++;
		}
	}
}
unsigned int getChar (){
    unsigned int info;
    unsigned int st;
    unsigned int result;
    info = *JTAG_UART_DATA;
    st = info & 0x8000;
    if (st == 0){ //no input by user in time, ran out of time
        result = -1;
    }
    else{
        result = (info & 0xFF);
    }
    return result;
}

void countdown(){
    *BUTTON_MASK = 0x0;
    *TIMER_CONTROL = 0x7; //cause a timer interrupt
    //askMathQuestion();
}

int myRand (){          //to generate a random number without stdlib.h
    static int next = 8642;
    next = ((next * next)/100)%10000;
    return next;
}

int myRandinRange(){
    return myRand()%10; //10 questions per subject, put whatever number of questions we have
}

void askMathQuestion(){
    int randomN = myRandinRange();
    switch (randomN){
        case 0:
        printString("What's 6^2 + 8 / 4?\nA: 36\nB: 11\nC: 38\nD: 44\n");
        correctAnswer = 'c';        
        break;
        case 1:
        printString("What's square of 17?\nA: 256\nB: 301\nC: 196\nD: 289\n");
        correctAnswer = 'd';
        break;
        case 2:
        printString("How many prime numbers under 20?\nA: 5\nB: 6\nC: 7\nD: 8\n");
        correctAnswer = 'd';
        break;
        case 3:
        printString("What's sum of all integers between 0 and 10\nA: 55\nB: 45\nC: 50\nD: 60\n");
        correctAnswer = 'a';
        break;
        case 4:
        printString("What's 0.4 of 0.8 of 120?\nA: 20\nB: 24\nC: 18\nD: 28\n");
        correctAnswer ='b';
        break;   
        case 5:
        printString("What's highest common factor of 30 and 132?\nA: 8\nB: 6\nC: 4\nD: 2\n");
        correctAnswer = 'b';
        break;
        case 6:
        printString("What decimal number is binary 100100?\nA: 32\nB: 40\nC: 36\nD: 20\n");
        correctAnswer = 'c';
        break;
        case 7:
        printString("How many edges does a cube have?\nA: 12\nB: 8\nC: 10\nD: 14\n");
        correctAnswer = 'a';
        break;
        case 8:
        printString("What number is twice the sum of its digits?\nA: 20\nB: 18\nC: 29\nD: 16\n");
        correctAnswer = 'b';
        break;
        case 9:
        printString("What is 5 factorial?\nA: 15\nB: 50\nC: 120\nD: 24\n");
        correctAnswer ='c';
        break;
    }
}
void askScienceQuestion(){
    int randomN = myRandinRange(); //could create a new function if we had more/less than 5Qs
    switch(randomN){
        case 0:
        printString("How many planets are in our solar system?\nA: 7\nB: 8\nC: 9\nD: 0\n");
        correctAnswer = 'b';
        break;
        case 1:
        printString("What is botany the study of?\nA: Plants\nB: Animals\nC: Planets\n");
        correctAnswer = 'a';
        break;
        case 2:
        printString("Where is gravity strongest?\nA: Moon\nB: Mercury\nC: ISS\nD: Earth\n");
        correctAnswer ='d';
        break;
        case 3:
        printString("What do most enzymes end in?\nA: ase\nB: is\nC: ouse\nD: ane\n");
        correctAnswer = 'a';
        break;
        case 4:
        printString("What is 100km/h in m/s?\nA: 30\nB: 40\nC: 27.78\nD: 25.33\n");
        correctAnswer = 'c';
        break;
        case 5:
        printString("What's chemical composition of water?\nA: H2O\nB: HO2\nC: H3O\nD: C6H12O6\n");
        correctAnswer = 'a';
        break;
        case 6:
        printString("How many bones are in the human body?\nA: 167\nB: 999\nC: 206\nD: 32\n");
        correctAnswer = 'b';
        break;
        case 7:
        printString("What does L in a circuit mean?\nA: Resistor\nB: Capacitor\nC: Source\nD: Inductor\n");
        correctAnswer = 'd';
        break;
        case 8:
        printString("What is the most abundant element in Earth's atmosphere?\nA: Oxygen\nB: Nitrogen\nC: Phosphorus\nD: Helium\n");
        correctAnswer = 'b';
        break;
        case 9:
        printString("How many valence electrons do common Noble Gases have?\nA: 6\nB: 8\nC: 0\nD: 10\n");
        correctAnswer = 'b';
        break;
    }
}
  
void checkAnswer(){
    char userAnswer = getChar();
    if (userAnswer == -1){
        printString("Ran out of time\n\n");
        if (score > 24){
            score = score - 25;
        }
        printScore(score);
    }
    else{
    if ((userAnswer >= 'A')&&(userAnswer <= 'Z')) {
        userAnswer = (userAnswer - 'A' + 'a'); //to correct if they input a capital
    }
    if (userAnswer == correctAnswer){ //user answered correctly
        printString ("Correct! Click a button to play again\n\n");
        score = score + 100;
        printScore(score);
    }
    else{
        printString("Incorrect. Correct Answer is ");
        printChar(correctAnswer);
        printString("\n\n");
        if (score > 24){ 
            score = score - 25;
        }
        printScore(score);
    }
    }
}


int main(void){

    Init();

    printString("Welcome to Jack's Trivia Game!\n\n");
    printString("To answer, type correct letter on your keyboard and wait for timer to countdown\n\n");
    printString("Click BUTTON0 to RESET score\n");
    printString("Click BUTTON1 for a MATH question\n");
    printString("Click BUTTON2 for a SCIENCE question\n\n");

    
	return 0;	/* never reached, but main() must return a value */
}


/*-----------------------------------------------------------------*/
/*              end of application-specific code                   */
/*-----------------------------------------------------------------*/



/*-----------------------------------------------------------------*/


/* 
   exception_handler.c

   This file is a portion of the original code supplied by Altera.

   It has been adapted by N. Manjikian for use in ELEC 371 laboratory work.

   Various unnecessary or extraneous elements have been excluded. For
   example, declarations in C for external functions called from asm()
   instructions are not required because any reference to external names
   in asm() instructions is embedded directly in the output written to
   the assembly-language .s file without any other checks by the C compiler.

   There is one particularly important change: on _reset_, the jump must be
   to the >> _start << location in order to properly initialize the stack
   pointer and to perform other crucial initialization tasks that ensure
   proper C semantics for variable initialization are enforced. The Altera
   version of the code jumped to main(), which will _not_ perform these
   crucial initialization tasks correctly.

   Finally, a reference to control register 'ctl4' in the asm() sequence
   has been replaced with the more meaningful alias 'ipending' for clarity.

   Other than the changes described above, the file contents have also been
   reformatted to fit in 80 columns of text, and comments have been edited.
*/


/* The assembly language code below handles processor reset */
void the_reset (void) __attribute__ ((section (".reset")));

/*****************************************************************************
 * Reset code. By giving the code a section attribute with the name ".reset" *
 * we allow the linker program to locate this code at the proper reset vector*
 * address. This code jumps to _startup_ code for C program, _not_ main().   *
 *****************************************************************************/

void the_reset (void)
{
  asm (".set noat");         /* the .set commands are included to prevent */
  asm (".set nobreak");      /* warning messages from the assembler */
  asm ("movia r2, _start");  /* jump to the C language _startup_ code */
  asm ("jmp r2");            /* (_not_ main, as in the original Altera file) */
}

/* The assembly language code below handles exception processing. This
 * code should not be modified; instead, the C language code in the normal
 * function interrupt_handler() [which is called from the code below]
 * can be modified as needed for a given application.
 */

void the_exception (void) __attribute__ ((section (".exceptions")));

/*****************************************************************************
 * Exceptions code. By giving the code a section attribute with the name     *
 * ".exceptions" we allow the linker program to locate this code at the      *
 * proper exceptions vector address. This code calls the interrupt handler   *
 * and later returns from the exception to the main program.                 *
 *****************************************************************************/

void the_exception (void)
{
  asm (".set noat");         /* the .set commands are included to prevent */
  asm (".set nobreak");      /* warning messages from the assembler */
  asm ("subi sp, sp, 128");
  asm ("stw  et, 96(sp)");
  asm ("rdctl et, ipending"); /* changed 'ctl4' to 'ipending' for clarity */
  asm ("beq  et, r0, SKIP_EA_DEC");   /* Not a hardware interrupt, */
  asm ("subi ea, ea, 4");             /* so decrement ea by one instruction */ 
  asm ("SKIP_EA_DEC:");
  asm ("stw	r1,  4(sp)"); /* Save all registers */
  asm ("stw	r2,  8(sp)");
  asm ("stw	r3,  12(sp)");
  asm ("stw	r4,  16(sp)");
  asm ("stw	r5,  20(sp)");
  asm ("stw	r6,  24(sp)");
  asm ("stw	r7,  28(sp)");
  asm ("stw	r8,  32(sp)");
  asm ("stw	r9,  36(sp)");
  asm ("stw	r10, 40(sp)");
  asm ("stw	r11, 44(sp)");
  asm ("stw	r12, 48(sp)");
  asm ("stw	r13, 52(sp)");
  asm ("stw	r14, 56(sp)");
  asm ("stw	r15, 60(sp)");
  asm ("stw	r16, 64(sp)");
  asm ("stw	r17, 68(sp)");
  asm ("stw	r18, 72(sp)");
  asm ("stw	r19, 76(sp)");
  asm ("stw	r20, 80(sp)");
  asm ("stw	r21, 84(sp)");
  asm ("stw	r22, 88(sp)");
  asm ("stw	r23, 92(sp)");
  asm ("stw	r25, 100(sp)"); /* r25 = bt (r24 = et, saved above) */
  asm ("stw	r26, 104(sp)"); /* r26 = gp */
  /* skip saving r27 because it is sp, and there is no point in saving sp */
  asm ("stw	r28, 112(sp)"); /* r28 = fp */
  asm ("stw	r29, 116(sp)"); /* r29 = ea */
  asm ("stw	r30, 120(sp)"); /* r30 = ba */
  asm ("stw	r31, 124(sp)"); /* r31 = ra */
  asm ("addi	fp,  sp, 128"); /* frame pointer adjustment */

  asm ("call	interrupt_handler"); /* call normal function */

  asm ("ldw	r1,  4(sp)"); /* Restore all registers */
  asm ("ldw	r2,  8(sp)");
  asm ("ldw	r3,  12(sp)");
  asm ("ldw	r4,  16(sp)");
  asm ("ldw	r5,  20(sp)");
  asm ("ldw	r6,  24(sp)");
  asm ("ldw	r7,  28(sp)");
  asm ("ldw	r8,  32(sp)");
  asm ("ldw	r9,  36(sp)");
  asm ("ldw	r10, 40(sp)");
  asm ("ldw	r11, 44(sp)");
  asm ("ldw	r12, 48(sp)");
  asm ("ldw	r13, 52(sp)");
  asm ("ldw	r14, 56(sp)");
  asm ("ldw	r15, 60(sp)");
  asm ("ldw	r16, 64(sp)");
  asm ("ldw	r17, 68(sp)");
  asm ("ldw	r18, 72(sp)");
  asm ("ldw	r19, 76(sp)");
  asm ("ldw	r20, 80(sp)");
  asm ("ldw	r21, 84(sp)");
  asm ("ldw	r22, 88(sp)");
  asm ("ldw	r23, 92(sp)");
  asm ("ldw	r24, 96(sp)");
  asm ("ldw	r25, 100(sp)");
  asm ("ldw	r26, 104(sp)");
  /* skip r27 because it is sp, and we did not save this on the stack */
  asm ("ldw	r28, 112(sp)");
  asm ("ldw	r29, 116(sp)");
  asm ("ldw	r30, 120(sp)");
  asm ("ldw	r31, 124(sp)");

  asm ("addi	sp,  sp, 128");

  asm ("eret"); /* return from exception */

  /* Note that the C compiler will still generate the 'standard'
     end-of-normal-function code with a normal return-from-subroutine
     instruction. But with the above eret instruction embedded
     in the final output from the compiler, that end-of-function code
     will never be executed.
   */ 
}
