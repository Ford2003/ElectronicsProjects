# Maze Game - LCD1602

This version of the maze game uses an **LCD1602** display. I used it display your current score (score being the current number of times you have completed the maze).

![MazeGameLCD](https://github.com/user-attachments/assets/9ccfecaa-c365-4903-a4a6-c8a75873a29a)
*Note:* This TinkerCad version uses the Arduino UNO R3 since R4 isn't available, but the connections remain the same.

Here's a completed breakdown of this project:
1. Load the saved high score from the Arduino EEPROM.
2. Display the saved high score and the current score (0 initially) on the LCD.
3. Utilise a **randomised BFS** method to generate the maze of a given size.
4. Continually blink the players position LED every 0.5s.
5. Check if any of the buttons are emitting a HIGH signal (i.e. they are being pressed) and that the debounce time has passed.
6. If a button is pressed check if the player can move in that respective direction, i.e. check there's no walls in the way.
7. If the player has moved into the exit wall of the maze then increment the score and randomly choose a set phrase and display it as a scrolling text on the LED matrix as a 'Well Done!' message.
8. Display the new score on the LCD and if a new high score was reached display that too and write the new score into the Arudino EEPROM.
9. Regenerate the maze and reset the player position and repeat.

Below is what the circuit looks like in real life
![MazeGameIRL](https://github.com/user-attachments/assets/8722c629-d86d-4cae-8623-51a0e4291b15)
