$modelname "wizard.mdl"
$cd ".\"
$cdtexture "."
$scale 1.0
$origin 0 0 -21 -90
$eyeposition 0 -16 48

//$cliptotextures

//reference mesh(es)
$bodygroup body
{
studio "reference"
}

$attachment 0 "Base" 32.000000 0.000000 16.000000

$hbox 1 "Head" -5.31 -6.98 -9.55  7.97 6.98 4.95
$hbox 2 "Gut" -12.77 -12.18 -10.50  7.05 12.51 17.30
$hbox 3 "Tail" -0.88 -5.78 -5.59  10.37 8.43 7.72
$hbox 3 "Tail1" -0.45 -4.01 -3.88  7.41 7.47 5.22
$hbox 3 "Tail2" -0.47 -3.22 -3.20  7.34 6.25 3.50
$hbox 3 "Tail3" -0.12 -2.40 -2.86  6.97 5.44 1.96
$hbox 3 "Tail4" 0.00 -1.68 -1.72  6.86 4.13 1.76
$hbox 3 "Tail5" -0.08 -1.02 -1.59  11.92 2.80 0.88
$hbox 5 "RShoulder" -2.90 -2.52 -2.12  2.09 2.71 2.86
$hbox 5 "RForeArm" -2.13 -3.26 -4.99  4.57 4.54 4.61
$hbox 5 "RArm" -2.06 -2.31 -2.25  5.43 3.42 3.13
$hbox 4 "LForeArm" -1.92 -4.08 -4.55  4.70 3.98 4.68
$hbox 4 "LShoulder" -2.64 -2.85 -1.94  2.34 2.38 2.63
$hbox 4 "LArm" -2.33 -2.93 -2.61  6.65 2.83 3.66

$sequence "hover" "hover" fps 10 loop ACT_IDLE 1
$sequence "hover" "hover" fps 10 loop ACT_HOVER 1
$sequence "walk" "hover" fps 10 loop ACT_WALK 1 { event 3 1 }
$sequence "run" "hover" fps 10 loop ACT_RUN 1 { event 3 1 }
$sequence "fly" "fly" fps 10 loop ACT_RUN 1 { event 3 1 }
$sequence "magatt" "magatt" fps 10 ACT_MELEE_ATTACK1 1 { event 1 2 } { event 2 8 } 
$sequence "pain1" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1