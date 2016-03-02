# joystick-control
Map certain actions over joystick input and control your computer with it.

## How to use it?
You need 2 certain files: the joystick device file (the most
common ones are `/dev/input/jsX`, where `X` is the number of joystick, starting
from 0) and the script file. Use them as the first and the second argument to joystick-control.

## What are the dependecies of it?
These are linux-headers and libxdo-dev. To install them:

## Does this program run in background?
Yes, it does. To run it, use:
```bash
joystick-control /path/to/joystick/file /path/to/mapping/file [/optional/path/to/configuration/file]
```
To finish:
```bash
finish-joystick-control
```

```bash
apt-get install linux-headers-$(uname -r)
apt-get install libxdo-dev
```

Find the appropriate names of packages for your distribution.
## Installing the program.
```bash
make
sudo make install
```

## What can I actually script and how?
First, you need to be aware of how joysticks work. Every button has its code number.
Unfortunately, joysticks vary and the same buttons on both of some joysticks may have
different code numbers. That's why the program needs to be "calibrated". If you
don't provide the configuration file as the third argument to the program, it will
automatically turn up the configuration wizard. It'll ask you to press all of
buttons on the chosen joystick first.
#### WARNING: Directional buttons are treated as axes, which is a standard.
The order in which you will press the buttons is crucial. This way, the program
will easily avoid the aforementioned problem with differences in input codes
in different pads. Memorize the order in which you'll be pressing the buttons
and use the same order in the script (more on scripting later). When it comes to "calibrating" the axes,
remember that every analog and set of directional buttons consist of a pair of axes - the horizontal
and the vertical one. You should move them PERFECTLY right and PERFECTLY up (so as to know where up
and right are, sometimes, the axes can be reversed) in the order you wish.

---
For instance: a standard PS3 pad has 4 action buttons, 6 L/R buttons, Start, Select and
PlayStation home button. It also has 2 analogs and a set of directional buttons. It is
compatible with joystick-control. In order to configure it, press the buttons in the order
you want and then, move the analogs carefully. Press the right arrow and an up arrow
afterwards. Since the program detects previously used buttons and axes, as well as the
number of them, the configuration should move smoothly.
#### WARNING: The program tends to hang up if you provide input too quickly.
When the configuration is over, you can save it in a file to be used later as a third argument
in the program so that you can avoid configuring it again.


---
Let's move on to the possibilities of joystick-input. It supports the following commands:
* `mousemove` - designed only for axes. It takes either "horizontal" or "vertical" input as its
first argument. Since you are forced to assign one command to one axis/button, you should
use "horizontal" with a horizontal axis and "vertical" with a vertical axis so that you can
fully move the mouse pointer in all directions. The second argument is sensitivity. It is a number
between 1 and 9 inclusive.
* `mouseclick` - it emits a mouse click. It can be one of the following: left, middle, right, wheelup
and wheeldown. The second and third arguments are optional. They specify the coordinates where the
click should happen.
* `mouseteleport` - literally teleports the mouse to the specified coordinates, using the coordinates
from the first and the second argument. The third argument is the screen you want the mouse pointer
to appear on. It's optional. The default screen is the 0th one. Use the number of the different
connected screen to alter it.
* `keypress` - this command enables the user to use any kinds of keyboard shortcuts, using any
buttons on the keyboard. To get the list of key symbols, see the definitions in `X11/keysymdef.h`
header file in your include folder (after installing `libxdo-dev`), just exclude the `XK_` prefix
from any key symbol you'll use, you can use pretty any symbol that's listed there. It is possible to
press an unlimited number of buttons at once - you need to separate them with the plus sign, such as
`Control+Alt_L+T`. Do not use spaces. When it comes to the axes, I didn't want their potential
usage, as in: moving the cursor or whatever else around, to be "wasted" (since you can assign
a keypress command to a whole axis, it's impossible to implement 2 different actions for, for
example, 2 opposed buttons on the directional pad, because left and right represent the same axis,
same for up and down) - that's why I made special `/HORIZONTAL` and `/VERTICAL` arguments. When
either the analog is maximally tilted or a directional button is pressed, it'll have the same
effect as pressing the arrows on the keyboard.
* `keystroke` - a simple command for pasting the pieces of text you define as its argument. Since
it can't be prolonged to more than one line, you should use `/NEWLINE` instead of the real newline
character or pressing Enter (trying to avoid possible conflicts). For a horizontal tab, use `/HTAB`.
* `command` - it allows to perform a system command. It takes everything after as an argument (of
course, everything should be in the same line) and you can use `/NEWLINE` and `/HTAB` there, too.

---
A sample mapping file:
```
a0 mousemove horizontal 3
a1 mousemove vertical 3
b0 mouseclick left
b1 keypress Return
b2 keystroke test/NEWLINE/HTABindented test
b3 mouseclick right 1 1
b4 mouseteleport 10 10
b5 mouseteleport 20 20 1
b6 command notify-send "whoo, it works"
```
First, you need to specify the kind of input: `a` for an axis and `b` for a button. Use the number
of the axis/button, taking into account the order you pressed/moved it in, counting from zero - as I mentioned
earlier. If you pressed, let's say, a circle first, it's going to be `b0`, if you want to
click a right directional button first, the directional horizontal axis will be labeled as `a0`, it doesn't matter
what code it is, for joystick-input will take care of it.

---
The configuration file created with the wizard will contain the joystick's name. Thanks to that,
it's possible to recognize whether a certain configuration file was created for some certain
joystick.

## Questions and suggestions.
Alwats feel free to open an issue, fork my project or write to me in case you find any bugs or
have any idea to improve or implement joystick-control, no feedback shall remain unanswered.