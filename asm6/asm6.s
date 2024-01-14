# @author: Minh Duong
# COURSE: CSC 252
# @description: This "toy" program is a guessing game. It will generate a random number,
# then have the user guess. After the guess, the game will tell them if they are too high
# or too low. As an extra feature, the game will limit the number of guesses to 10 each game,
# and the game will have a "Easter Egg" that if the number is the author's favorite number,
# the game will print a special string.

.data
str_welcome:	            .asciiz	"Hello there! Welcome to Minh's special guessing game!"
str_instruction_attempts:	.asciiz	"You have a total of 10 attempts for each game."
str_instruction_scores:	  .asciiz	"Your final score will be the number of guesses left after each game. If you are correct, of course!"
str_prompt:	              .asciiz	"Guess a number between 0 and 100..."
str_low:	                .asciiz	"Your guess is too low!"
str_high:	                .asciiz	"Your guess is too high!"
str_correct:	            .asciiz	"Nice! Your guess is correct! The number is "
str_bye:	                .asciiz	"\nGood bye! I will miss you :("
str_invalid:	            .asciiz	"Invalid input: "
str_new_game_prompt:	    .asciiz	"\nWould you like to start a new game? (Enter a character to continue, or N/n to quit)"
str_you_have:	            .asciiz	"You have "
str_attempts_left:	      .asciiz	" attempts left."
str_out_of_attempt:	      .asciiz	"You had run out of attempts. Game Over :("
str_scores:	              .asciiz	"Your total score is: "
str_special:              .asciiz "Wow, you have guessed my favorite number! Lucky you!"
newline:                  .asciiz	"\n"

.text
.globl	main
main:	
	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_welcome
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	
	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_instruction_attempts
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_instruction_scores
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall

	addi $s4, $zero, 0	# s4 = total score

START_GAME:
	addi $a1, $zero, 100	  # upper bound for randomizing
  addi $v0, $zero, 42	    # generate a random number
  syscall
  
  addi $s0, $a0, 0	      # s0 = correct number
    	
  addi $s3, $zero, 10	    # s3 = number of attempts left each game

LOOP:		
	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_prompt	
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	# read_int(guess) from user and store to s1
	addi	$v0, $zero, 5		
	syscall	
	addi	$s1, $v0, 0

	addi $t1, $zero, 0		  # t1 = 0
	addi $t2, $zero, 100		# t2 = 100
	
	slt $t0, $s1, $t1		# if guess < 0, go to WRONG_INPUT
	bne $t0, $zero, WRONG_INPUT
	
	slt $t0, $t2, $s1		# if guess > 100, go to WRONG_INPUT
	bne $t0, $zero, WRONG_INPUT
	
	addi $s3, $s3, -1   # Decrement attempts
	# if number of attempts reaches 0, go to ALL_ATTEMPT_USED
	beq $s3, $zero, ALL_ATTEMPT_USED
	
	slt $t0, $s1, $s0		  # if guess < num, go to LOWER
	bne $t0, $zero, LOWER
	
	slt $t0, $s0, $s1		  # if guess > num, go to HIGHER
	bne $t0, $zero, HIGHER

	beq	$s1, $s0, CORRECT	# if guess == num, go to CORRECT
	j	LOOP


ALL_ATTEMPT_USED:
	add $s4, $s4, $s3
	
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_out_of_attempt
	syscall

	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
		
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_scores
	syscall
	
	# print int()
	addi	$v0, $zero,1		
	addi	$a0, $s4, 0
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall

	j PLAY_AGAIN

WRONG_INPUT:
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_invalid
	syscall

	# print int()
	addi	$v0, $zero, 1	
	addi	$a0, $s1, 0
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	j LOOP

LOWER:
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_low
	syscall

	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_you_have	
	syscall
	
	# print int()
	addi	$v0, $zero, 1	
	addi	$a0, $s3, 0
	syscall
	
	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_attempts_left	
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	j LOOP
	
HIGHER:
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_high
	syscall

	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall

	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_you_have	
	syscall
	
	# print int()
	addi	$v0, $zero, 1	
	addi	$a0, $s3, 0
	syscall
	
	# print str()
	addi	$v0, $zero, 4		
	la	$a0, str_attempts_left	
	syscall

	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	j LOOP
		
CORRECT:	
	add $s4, $s4, $s3
	
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_correct
	syscall

	# print correct number
	addi	$v0, $zero, 1	
	addi	$a0, $s0, 0
	syscall

	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall

  beq $s0, 23, CORRECT_LUCKY
  j AFTER_CORRECT

CORRECT_LUCKY:
  # print str()
  addi	$v0, $zero,4		
	la	$a0, str_special
	syscall

  # print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall

  j AFTER_CORRECT

AFTER_CORRECT:
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_scores
	syscall
	
	# print int()
	addi	$v0, $zero,1		
	addi	$a0, $s4, 0
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline

PLAY_AGAIN:	
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_new_game_prompt
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	
	syscall
	
	
	# read_char() from user and store to s2
	addi	$v0, $zero, 12
	syscall	
	addi	$s2, $v0, 0
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall

	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	syscall
	
	addi $t0, $zero, 'n'
	beq $s2, $t0, END
	
	addi $t0, $zero, 'N'
	beq $s2, $t0, END
	
	j START_GAME
	
END:
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_scores
	syscall
	
	# print int()
	addi	$v0, $zero,1		
	add	$a0, $zero, $s4
	syscall
	
	# print '\n'
	addi	$v0, $zero, 4		
	la	$a0, newline
	
	# print str()
	addi	$v0, $zero,4		
	la	$a0, str_bye
	syscall
	
	addi	$v0, $zero, 10		
	syscall