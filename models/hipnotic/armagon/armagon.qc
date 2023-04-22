$modelname "armagon.mdl"

$cd ".\armagon"
$cdtexture ".\armagon"
$scale 1.0
$origin 0 0 0 180
$flags 512

$body "legs" "armalegs"

$texrendermode flash_skin.bmp fullbright

$bodygroup body
{
studio "armabody"
blank
}

$hbox 10 "cog" -25.52 -15.71 -19.67  25.52 11.45 8.53
$hbox 10 "hip_R" -8.01 -15.52 -10.60  6.22 29.05 8.32
$hbox 10 "hip_L" -7.24 -15.07 -10.60  7.36 29.05 8.62
$hbox 10 "knee_R" -7.32 0.00 -9.76  9.10 54.63 15.43
$hbox 10 "knee_L" -8.09 0.00 -9.76  8.69 54.63 15.43
$hbox 10 "foot_R" -20.09 -27.26 -13.24  20.77 25.98 0.00
$hbox 10 "foot_L" -20.13 -27.26 -13.24  21.10 25.44 0.00
$hbox 0 "spine1" -14.82 -0.26 -17.00  16.62 15.18 16.86
$hbox 0 "spine2" -22.46 -3.30 -22.27  22.15 16.54 15.16
$hbox 0 "spine3" -22.46 -1.13 -21.36  19.39 23.66 13.80
$hbox 0 "neck" -10.18 0.00 -9.80  9.68 10.74 10.20
$hbox 0 "clav_R" -16.75 0.00 -3.16  13.97 20.02 6.80
$hbox 0 "clav_L" -13.89 0.00 -3.27  10.14 20.02 6.80
$hbox 0 "head" -9.49 -0.28 -15.29  9.09 13.20 12.02
$hbox 0 "shoulder_R" -10.52 0.00 -7.77  9.02 28.20 10.36
$hbox 0 "shoulder_aux_R" -9.62 -5.78 -9.85  10.23 9.89 11.64
$hbox 0 "shoulder_L" -8.98 0.00 -7.47  10.60 28.20 10.66
$hbox 0 "shoulder_aux_L" -10.32 -5.11 -9.60  9.64 10.39 11.89
$hbox 0 "elbow_R" -7.84 -0.67 -9.22  5.96 53.16 8.60
$hbox 0 "elbow_L" -5.70 -0.55 -9.75  8.18 53.26 8.07
$hbox 0 "flash_R" -4.52 0.00 -5.07  2.67 45.02 3.21
$hbox 0 "flash_L" -3.14 -2.30 -4.11  4.03 42.09 3.34

$attachment 0 "muzzle_R" 4 4 4
$attachment 1 "muzzle_L" -4 4 4

$controller 0 "spine1" ZR -45.000000 45.000000

$sequence "stand" "stand" fps 5 loop ACT_IDLE 1
$sequence "walk" "walk" fps 10 loop ACT_WALK 1 { event 2 1 } { event 3 3 } { event 2 8 } { event 3 10 }
$sequence "run" "walk" fps 10 loop ACT_RUN 1 { event 2 1 } { event 3 3 } { event 2 8 } { event 3 10 } { event 4 11 }
$sequence "lw0fir" "w0fir" fps 10 ACT_MELEE_ATTACK1 1 { event 3 2 }  { event 2 4 } { event 3 8 } { event 2 10 } { event 5 10 } { event 10 12 } 
$sequence "lg0fir" "w0fir" fps 10 ACT_MELEE_ATTACK2 1 { event 3 2 }  { event 2 4 } { event 3 8 } { event 2 10 } { event 6 10 } { event 10 12 } 
$sequence "lsfire" "sfire" fps 10 ACT_RANGE_ATTACK1 1 { event 2 6 } { event 5 8 } { event 7 10 }
$sequence "lgfire" "sfire" fps 10 ACT_RANGE_ATTACK2 1 { event 2 6 } { event 6 8 } { event 7 10 }
$sequence "lw1fir" "w1fir" fps 10 ACT_TURN_LEFT 1 { event 3 1 } { event 2 4 } { event 3 7 } { event 2 12 } { event 8 9 } { event 9 10 } { event 10 14 }
$sequence "lw2fir" "w2fir" fps 10 ACT_TURN_RIGHT 1 { event 3 11 }  { event 2 2 } { event 3 4 } { event 2 8 } { event 8 3 } { event 9 4 } { event 10 12 }
$sequence "tranl" "tran" fps 10 ACT_ARM 1 { event 1 1 }
$sequence "ldie" "die" fps 8 ACT_DIESIMPLE 1 { event 11 2 } { event 12 12 }
