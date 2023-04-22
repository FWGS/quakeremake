$modelname "boss.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 -110 -90

$flags 512

$bodygroup body 
{ 
studio "reference_fixed" 
} 

$hbox 2 "Spine1" -30.43 -68.28 -50.25  77.93 58.89 54.67
$hbox 2 "Spine2" -43.64 -71.07 -14.83  53.47 58.57 23.77
$hbox 2 "Neck" -7.22 -19.64 -26.71  48.83 27.64 27.71
$hbox 1 "Head" -41.16 -55.77 -61.19  42.85 55.22 51.51
$hbox 4 "LShoulder" -38.37 -31.67 -24.81  72.36 30.36 21.05
$hbox 4 "LArm" -14.78 -38.11 -25.12  56.48 30.15 19.76
$hbox 4 "LArm1" -7.46 -17.30 -15.79  43.12 18.40 17.78
$hbox 4 "LHand" -14.66 -7.44 -13.67  17.03 9.98 21.87
$hbox 4 "LHand11" -0.76 -5.59 -5.93  55.56 5.04 5.04
$hbox 4 "LHand21" -7.61 -9.53 -9.74  45.77 8.17 8.26
$hbox 5 "RShoulder" -27.05 -29.53 -19.04  70.34 33.51 38.06
$hbox 5 "RArm" -18.56 -27.97 -23.53  68.37 34.48 27.23
$hbox 5 "RArm1" -9.25 -14.75 -13.64  55.14 14.43 15.01
$hbox 5 "RHand" -6.17 -6.88 -9.82  7.20 11.74 20.07
$hbox 5 "RHand21" -8.91 -9.87 -10.69  50.56 12.27 8.90
$hbox 5 "RHand11" -13.72 -9.11 -9.24  64.45 9.11 15.93
$hbox 3 "Pelvis" -5.78 -63.62 -60.84  102.29 62.41 23.17
$hbox 3 "joint23" -23.39 -22.12 -17.39  13.41 40.49 20.89
$hbox 3 "joint24" -26.23 -41.51 -18.20  12.45 18.96 20.12
$hbox 3 "joint25" -12.59 -34.48 -13.22  18.57 36.51 34.30
$hbox 3 "joint26" -15.56 -36.05 -18.07  26.90 37.13 40.66
$hbox 3 "joint27" -29.18 -32.52 -23.70  16.78 36.67 34.36
$hbox 3 "joint28" -28.85 -37.37 -22.33  19.57 31.41 30.58
$hbox 2 "Chest" -73.20 -53.01 -11.75  30.48 51.14 25.32
$hbox 2 "LSide" -0.90 -5.72 -17.16  0.74 4.79 16.33
$hbox 2 "RSide" -5.60 -5.20 -16.33  4.71 2.72 17.02

// 4 attachment(s)
$attachment 0 "RHand" 16.000000 0.000000 0.000000
$attachment 1 "LHand" 16.000000 0.000000 0.000000

$sequence "shocka" "shocka" loop fps 10 ACT_SMALL_FLINCH 1
$sequence "shockb" "shockb" loop fps 10 ACT_SMALL_FLINCH 1
$sequence "shockc" "shockc" loop fps 10 ACT_BIG_FLINCH 1
$sequence "attack" "attack" fps 10 ACT_MELEE_ATTACK1 1 { event 3 5 } { event 4 17 } 
$sequence "rise" "rise" fps 10 ACT_USE 1 { event 1 1 } { event 2 2 } 
$sequence "death" "death" fps 10 ACT_DIEVIOLENT 1 { event 5 0 } { event 6 8 }
$sequence "walk" "walk" LX loop fps 10 ACT_WALK 1
