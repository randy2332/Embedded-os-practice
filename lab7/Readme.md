# Lab7



A guessing game composed of Game.c and Guess.c.

Gamer:

- Obtain the addressing space of shared memory,
- The Guess program uses the **SIGUSR1** signal to notify the Game program to make numerical judgments (too big, small, correct).
- **SIGUSR1** handler function, compares the guess variable in shared memory with the guessed number, and writes the result back to the result variable.

Guess:

- Obtain the addressing space of shared memory
- Use **half-guess (two-point search)** to make guesses.
- The Game program uses the **SIGUSR1** signal to notify the Game program to receive the judgment result.
- Read the result of the result variable every second, calculate the number to be guessed in this round and write it into the guess variable, and finally use **SIGUSR1** to notify the Game program for processing
