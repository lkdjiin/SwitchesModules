# NEX Switches Modules for VCV Rack

## Matrix Mixer 4

A matrix mixer with 4 inputs and 4 outputs. You can mute rows and/or columns via
some different algorithms.

*Un mélangeur matriciel avec 4 entrées et 4 sorties. Vous pouvez rendre muettes
les rangées et/ou les colonnes grâce à différents algorithmes.*

![]()

**INPUTS**: The 4 inputs are on the top of the module.

**OUTPUTS**: The 4 outputs are on the right of the module.

**ROWS**: Think of each row as a 4x1 mixer. That is 4 inputs to 1 output.

**COLUMNS**: Think of each column as a 1x4 multiplexer. That is 1 input to 4 outputs with different levels.

**MUTE**: Pushing the button to the left of a row will mute/unmute that row.
Same thing for the columns, pushing the button at the bottom of a column will
mute/unmute that column. You can also use Control Voltage with each mute button.
How individual potentiometers will be enabled/disabled depends on the actual
algorithm.

**MUTE ALGORITHM**: There is 2 algorithms. The first one is «Force», and the
second one is «Flip-flop».  With «Force», the default one, the whole row (or
column) is enabled or disabled, regardless of the state of the individual
potentiometers.  That is, if a row (or column) is muted, all the 4
potentiometers of that row (or column) are disabled.  And reciprocally, if a
row (or column) is unmuted, all the 4 potentiometers of that row (or column)
are enabled.  The «Flip-flop» algorithm works differently: when you mute (or
unmute) a row (or a column), each potentiometer's state is inversed in that row
(or column). That is, each enabled potentiometer of the row is disabled, and
each disabled potentiometer is enabled. I know this can be a confusing reading ;)
You can change the algorithm with the button on the bottom right.
*NOTE: Despite the UI, there is only 2 algorithms and no Control Voltage to change the algo for now.*

**AMPLITUDE ALGORITHM**: Right click the module to choose between ducking, hard clipping or no processing at all.
- Ducking - the more voices in a row, the less amplitude per voice.
- Hard clipping - 10 Volt peak to peak. This one could introduce a lot of distortion. Sometimes cool, sometimes harsh and not musical.
- No processing - Inputs are simply summed together.


## 2x2 Mute

It's like a crossfader for two groups of two channels. You can play group 1, or you can play group 2.
But you can't play both groups at the same time.

*C'est comme un fondu enchainé entre deux groupes de deux canaux. Vous jouez le groupe 1, ou vous jouez le groupe 2.
Mais vous ne pouvez pas jouer les deux groupes en même temps.*

![](https://github.com/lkdjiin/SwitchesModules/blob/master/images/2x2mute.png)

**IN**: On the picture above, the 2 inputs of group 1 are yellow and the 2 inputs of group 2 are red.

**OUT**: On the picture above, the 2 outputs of group 1 are green and the 2 outputs of group 2 are blue. First intput goes to first output, second input goes to second output and so on.

**Green LED Button**: Change the group to play. When the LED is on, group 1
 is playingplays and group 2 is muted. Vice versa, when the LED is off, group 1 is muted and group 2 is playing.

**CV**: Wait for a trigger then act like the green LED button.

**Knob**: Adjust the base time of the fade.

**Switch**: Adjust the scale for the fade's time, x1 up to 1 second, x10 up to 10 seconds and x100 up to 100 seconds.

**Exponential/Linear**: right click the module to choose between exponential and linear functions.

## Double Mute

With this module you can mute/unmute two monophonic inputs with a single button. You can
also choose the time of the fade-in and fade-out. Fade-in and fade-out are
independent and adjustable from 0 to 100 seconds.

*Avec ce module vous pouvez muter/démuter deux entrées (monophoniques) avec un seul bouton. Vous
pouvez aussi choisir le temps du fondu en ouverture et en fermeture. Ouverture
et fermeture sont indépendants et réglables de 0 à 100 secondes.*

![](https://github.com/lkdjiin/SwitchesModules/blob/master/images/doublemute.png)

**IN**: One or two input sources.

**OUT**: The outputs (top input goes to top output and bottom input goes to bottom output).

**Green LED Button**: Hit it to fade in/out.

**CV**: Wait for a trigger to fade in/out.

**Knobs**: Adjust the base time of the fade.

**Switches**: Adjust the scale for the fade's time, x1 up to 1 second, x10 up to 10 seconds and x100 up to 100 seconds.

**Exponential/Linear**: right click the module to choose between exponential and linear functions.
