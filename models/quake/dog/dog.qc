$modelname "dog.mdl"
$cd ".\"
$cdtexture "."
$scale 1.0
$origin 0 0 5 -90

//$cliptotextures

//reference mesh(es)
$bodygroup body
{
studio "reference"
}

$hbox 2 "Spine" -10.06 -7.38 -5.16  4.31 7.48 1.43
$hbox 2 "Spine1" -4.50 -6.14 -7.43  6.22 7.48 5.04
$hbox 3 "Spine2" -5.00 -7.29 -8.22  5.30 4.66 3.35
$hbox 3 "Spine3" -1.90 -5.56 -5.18  5.05 5.87 6.14
$hbox 3 "Tail" -2.07 -2.42 -2.25  2.05 2.74 3.78
$hbox 1 "Head" -1.12 -4.59 -2.53  7.72 5.45 6.95
$hbox 1 "Head1" -4.68 -4.09 -3.59  2.48 4.01 4.20
$hbox 5 "RShoulder" -3.26 -1.78 -3.23  10.29 1.51 4.80
$hbox 5 "RHand" -0.65 -2.47 -3.54  7.05 1.85 3.85
$hbox 5 "RHand1" -0.66 -1.63 -1.63  10.02 1.35 2.38
$hbox 5 "RHand2" -1.56 -2.32 -3.11  2.45 1.72 3.03
$hbox 4 "LShoulder" -2.99 -1.38 -4.69  10.48 1.40 3.85
$hbox 4 "LHand" -0.69 -2.42 -3.07  7.01 2.90 3.99
$hbox 4 "LHand1" -0.73 -1.99 -1.96  9.49 1.23 2.39
$hbox 4 "LHand2" -1.17 -2.27 -3.60  1.45 2.05 3.07
$hbox 7 "RThigh" -6.90 -2.53 -4.81  13.15 3.10 6.24
$hbox 7 "RLeg" -2.95 -1.80 -3.73  8.78 1.80 3.10
$hbox 7 "RLeg1" -1.36 -2.21 -3.18  1.33 1.84 2.36
$hbox 6 "LThigh" -7.32 -3.35 -5.62  12.88 2.20 5.55
$hbox 6 "LLeg" -3.21 -1.89 -3.50  9.19 1.43 2.73
$hbox 6 "LLeg1" -1.48 -1.70 -2.93  2.21 1.80 2.36
$hbox 1 "LEar" -1.73 -1.22 -1.43  3.30 2.32 0.98
$hbox 1 "REar" -1.47 -2.38 -1.21  3.15 2.32 1.41

$sequence "stand" "stand" loop fps 10 ACT_IDLE 1
$sequence "walk" "walk" LX loop fps 10 ACT_WALK 1 { event 1 1 }
$sequence "run" "run" LX loop fps 10 ACT_RUN 1 { event 1 1 }
$sequence "leap" "leap" LX fps 10 ACT_LEAP 1 { event 2 2 }
$sequence "attack" "attack" fps 10 ACT_MELEE_ATTACK1 1 { event 3 4 }
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1
