# ScienceFair2016
## How to view
If you're on Windows, you'll want to check out the two executables in the root directory.
If not, all the source for the executables is in the SFVariableGraph directory at the root.
Additionally, screenshots can be found in the Screenshots directory at the root.

## What this is
Coded for an ISEF project back in 2016, SFVariableGraph (henceforth referred to as SFVG) graphs four functions related to the project. On the right, there are sliders to adjust variables present in the functions, which change the graph in real-time.
* The four functions are two cubics and two quadratics, and we are primarily interested in the intersections between any cubic and any quadratic.
* "Trace"ing a variable draws in blue the intersections as the variable changes between the ranges of the slider.
* Slider ranges are adjustable.
* Thin green and red lines denote the physical limitations of the system; only intersections in this range are meaningful.
* "Scale", "Vertical Shift", and "Horizontal Shift" center the view on the box bounded by the thin green/red lines.
* "Toggle Arrows" shows real-time arrows which show the gradient at every point - or how the system behaves toward intersections (equilibrium).
* "Toggle Bifurcation" draw the bifurcation diagram, with respect to any one variable, which can be adjusted by a button beneath.
* Upon mouse idle for a set time, random sliders will move around to demonstrate different systems and their equilibria.