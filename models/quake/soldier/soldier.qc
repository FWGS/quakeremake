$modelname "soldier.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -5 -90 

$bodygroup body 
{ 
studio "reference_fixed" 
}

$attachment 0 "Gun1" 5 0 0

$hbox 3 "Spine1" -4.53 -6.44 -6.61  5.22 6.39 7.82
$hbox 2 "Spine2" -4.69 -7.64 -4.24  6.88 7.39 5.42
$hbox 2 "Spine3" -2.23 -7.96 0.00  0.00 8.14 6.58
$hbox 1 "Head" -4.02 -4.03 -3.82  3.42 3.77 5.53
$hbox 6 "LThigh" -0.76 -5.62 -1.95  11.74 3.91 4.21
$hbox 6 "LKnee" -0.27 -3.20 -4.21  3.21 3.19 3.15
$hbox 6 "LKnee1" -0.25 -4.16 -3.40  8.86 4.05 3.51
$hbox 6 "LFoot" -0.90 -1.93 -2.43  3.26 1.97 2.49
$hbox 6 "LFoot1" -2.46 -3.53 -3.48  8.90 3.44 0.79
$hbox 7 "RThigh" -0.66 -2.06 -5.59  11.54 2.05 5.21
$hbox 7 "RKnee" -0.41 -3.10 -4.19  3.06 3.00 3.15
$hbox 7 "RKnee1" -0.18 -3.53 -4.21  9.19 3.71 4.21
$hbox 7 "RFoot" -1.17 -1.91 -2.25  3.27 1.96 2.56
$hbox 7 "RFoot1" -2.32 -3.66 -3.54  8.87 3.58 0.90
$hbox 4 "LShoulder" -4.35 -4.97 -3.04  8.73 2.73 2.88
$hbox 4 "LArm" -1.95 -3.27 -3.33  7.84 2.58 3.42
$hbox 4 "LHand" -0.06 -1.35 -1.66  4.50 1.72 1.83
$hbox 4 "LHand1" -2.00 -2.26 -2.60  1.89 2.22 2.37
$hbox 5 "RShoulder" -4.24 -4.18 -3.81  9.45 2.84 4.00
$hbox 5 "RArm" -1.80 -2.27 -3.42  9.16 2.99 3.64
$hbox 5 "RHand" -4.86 -1.52 -1.60  2.63 1.66 1.94
$hbox 5 "RHand1" -1.97 -1.95 -2.72  2.04 2.58 2.73
$hbox 0 "Gun" -3.06 -3.26 -2.75  24.47 3.63 2.33
$hbox 0 "Gun1" -1.43 -3.61 -3.31  1.17 3.40 3.57

$sequence "stand" "stand" loop fps 10 ACT_IDLE 1
$sequence "walk" "prowl_" LX loop fps 10 ACT_WALK 1 { event 4 1 }
$sequence "run" "run" LX loop fps 10 ACT_RUN 1 { event 4 1 }
$sequence "shoot" "shoot" fps 10 ACT_RANGE_ATTACK1 1 { event 1 8 } { event 3 4 } { event 5001 4 "11" }  
$sequence "load" "load" fps 10 ACT_RELOAD 1
$sequence "pain" "pain" fps 10 ACT_BIG_FLINCH 1
$sequence "painb" "painb" fps 10 ACT_BIG_FLINCH 1
$sequence "painc" "painc" fps 10 ACT_BIG_FLINCH 1
$sequence "death" "death" fps 10 ACT_DIESIMPLE 1 { event 2 4 }

//$sequence "deathc" "deathc" fps 10 

 
 
