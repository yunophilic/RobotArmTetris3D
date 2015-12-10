# Robot Arm Tetris 3D
This is an interactive 3D game of Tetris using a robot arm (controlled by typical "WASD" keyboard controls) to drop the tetris block at the desired location. When a row is completely filled, it is removed and the tiles above it will be moved one row down. This game is coded in C++ with OpenGL libraries.

Demonstration: https://www.youtube.com/watch?v=SGEVGslZtdc

# System Requirements
- Linux OS
- GLEW (http://glew.sourceforge.net/index.html)
- freeglut (`sudo apt-get install freeglut3-dev`)

# How to play
```
$ make
$ ./RobotArmTetris3D
```

# Controls
- CTRL+Left_key and CTRL+Right_key to rotate camera
- 'W', 'A', 'S', and 'D' to control the robot arm
- 'W' and 'S' to adjust the angle of the upper arm
- 'A' and 'D' to adjust the angle of the lower arm
- Up key to rotate the tetris block
- Space to drop the tetris block (only able to drop if tetris block is not greyed out)
- 'Q' to quit
- 'R' to restart

# Additional Features
- Score: +10 points for a full row, -50 points if time is up (minimum score is zero).
- The tetris block in the robot hand is replaced by a new one if time is up.
- Each individual cell of a new tile piece will have different colors.
