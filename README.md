# CS120B---Tetris-Project-for-Arduino
Final project for my embedded systems course. Details the workings of a handheld Tetris console with working inputs, outputs, and logic.

The system features an arduino microcontroller, a breadboard with resistors and wiring, a joystick and an ultrasonic sensor for inputs, and a 4 8x8 matrix LED board with 2 matrices removed. Park Miller random number generation is used to randomize the order of blocks that spanws during the game. The top 2 matrices were removed because having more than 16 lines for a game of Tetris is very overkill for what this project needs to show. The ultrasonic sensor was included to be a unique hardware inclusion for the project that wasn't used in any of the course's previous lab assignments. 

To play the game, you press in on the joystick to transition from the start screen to the game. Random shape objects, or blocks, fall from the top matrix to the bottom matrix at a row per second. Pushing the joystick left and right moves the block one LED over each time, and pulling down on the joystick causes the block to move down rapidly. Moving your hand over the ultrasonic sensor sends a signal to rotate the block clockwise. Blocks 'solidify' at the bottom row and lock into place, while a new block spawns at the top matrix. The rest follows usual Tetris rules - fill all blocks in a row to delete that row and extend game time before losing, and having blocks solidify at the very top of the matrix means a 'game over'.

State machines are used in the script in order to sequence each event in the game. There are two main states that handle block movement, one of which causes blocks to fall and the other stores solidified blocks. Both each have a matrix table of arrays assigned to them, having either 1s or 0s in each element to say if a position is occupied by a block. For the falling state, each second or so the block is changed on the matrix array to move one row down while having its old position deleted. The other state sees that when a block hits the bottom row in the falling state, it position gets saved to a different matrix - this keeps track of what object is resting where, and is used for clearing out rows when the user gets a 'Tetris' or when they reach the game over state. When the block solidifies in the top row and is stored in the matrix, the script goes to a game over state, and allows the user to restart by pressing in on the joystick. Other states handle rotation based on user inputs, as well as the start screen (which just shows the name of the game).

## Link to Video
https://youtu.be/keBC4AZi-6A?si=rpgabwMz1QY-AeZG
