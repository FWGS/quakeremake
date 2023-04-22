$modelname "zombie.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -4 -90

$bodygroup body 
{ 
studio "reference_fixed" 
}

$attachment 0 "LHand" 0.000000 0.000000 0.000000
$attachment 1 "RHand" 0.000000 0.000000 0.000000

$hbox 3 "Pelvis" -2.42 -4.15 -3.75  2.12 4.39 0.19
$hbox 3 "Spine1" -0.10 -1.46 -1.80  3.45 1.48 1.72
$hbox 2 "Spine2" -0.77 -4.62 -1.69  8.84 4.52 1.58
$hbox 2 "Spine3" -4.52 -5.93 -2.97  1.92 5.74 3.61
$hbox 1 "Neck" -1.20 -1.53 -1.48  3.20 1.51 1.35
$hbox 1 "Head" -3.03 -2.34 -2.46  2.31 2.43 2.70
$hbox 4 "LShoulder" -1.68 -1.90 -1.61  7.68 1.25 2.42
$hbox 4 "LArm" -0.65 -1.47 -1.40  8.16 1.31 1.42
$hbox 4 "LHand" -0.95 -1.09 -1.19  2.98 0.90 1.12
$hbox 5 "RShoulder" -1.43 -1.07 -2.51  8.18 1.84 2.21
$hbox 5 "RArm" -0.83 -1.12 -1.29  9.28 1.05 1.26
$hbox 5 "RHand" -1.21 -1.10 -1.16  2.67 1.27 1.09
$hbox 6 "LThigh" -0.64 -1.77 -2.55  12.96 2.10 2.59
$hbox 6 "LKnee" -1.18 -1.91 -1.68  10.56 1.81 1.77
$hbox 6 "LFoot" -1.48 -2.16 -1.47  5.39 1.80 1.59
$hbox 7 "RThigh" -0.80 -2.00 -2.78  12.99 1.66 2.44
$hbox 7 "RKnee" -1.23 -1.86 -1.71  10.35 1.89 1.93
$hbox 7 "RFoot" -1.52 -1.62 -1.58  5.36 1.94 1.47

$sequence "stand" "stand" loop fps 10 ACT_IDLE 1
$sequence "walk" "walk" LX loop fps 10 ACT_WALK 1 { event 2 18 }
$sequence "run" "run" LX loop fps 10 ACT_RUN 1 { event 3 17 }
$sequence "atta" "atta" fps 10 ACT_MELEE_ATTACK1 1 { event 4 12 }
$sequence "attb" "attb" fps 10 ACT_MELEE_ATTACK1 1 { event 5 13 }
$sequence "attc" "attc" fps 10 ACT_MELEE_ATTACK1 1 { event 4 11 }
$sequence "paina" "paina" fps 10 ACT_SMALL_FLINCH 1
$sequence "painb" "painb" fps 10 ACT_SMALL_FLINCH 1 { event 6 8 }
$sequence "painc" "painc" fps 10 ACT_SMALL_FLINCH 1
$sequence "paind" "paind" fps 10 ACT_SMALL_FLINCH 1
$sequence "paine" "paine" fps 10 ACT_BIG_FLINCH 1 { event 6 10 }  { event 7 11 }
$sequence "cruc" "cruc_" loop fps 10 ACT_SLEEP 1 { event 1 2 }
