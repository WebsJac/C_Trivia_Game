# C_Trivia_Game
*Math and science trivia game created using C language interrupts

*The game features a 10 second timer that counts down using the 10 LEDs

*The hex displays show the users score and increment by 100 for a correct answer and decrement by 25 for an incorrect answer or if they run out of time (assuming score >= 25)

*JTAG_UART IO is used to both pose a question to the user and retrieve their answer

*Questions are generated using a random number generator that uses a modulation technique (since cpulator doesn't support the use of libraries)

Notes to play: go to https://cpulator.01xz.net/?sys=nios-de0&loadasm=share/sZ9cyq0.s&lang=c
1. Uncheck "Memory Suspicious Use of Bypass Cache" in settings (on bottom-left side)
2. Uncheck "Instruction fetch: modified OP code" in settings (on bottom-left side)
3. Click 'Compile and Load' in upper left corner
4. Click the 'Memory' tab and go to address 200. Change this address to '06c02034' instead of '06c10034'

Adjustments/Improvements for future: 
1. Disable pushbutton interrupts while previous is still happening -- check if this is working.....
2. Disable saving chars on stack. Only allow one so they don't lose points for accidental clicks
3. Rand allows for repeat questions, we would want someway of preventing it from being a previously chosen question 

